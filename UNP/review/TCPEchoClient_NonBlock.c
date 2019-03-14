#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"

extern int log_err;

/*
 *  ------------------------------------
 *  |xxxxxxxxxxx|oooooooooooooo|       |
 *  ------------------------------------
 *              ^              ^       ^            
 *              |              |       |            
 *           out_ack        out_send out_size
 * 
 *  ------------------------------------
 *  |xx|oooooooooooooo|                |
 *  ------------------------------------
 *     ^              ^                ^            
 *     |              |                |            
 *   in_ack         in_send         in_size
 */

char sendbuf[8 << 10];
char recvbuf[8 << 10];
char *out_send = sendbuf;
char *out_size = sendbuf + sizeof(sendbuf);
char *out_ack = sendbuf;
char *in_recv = recvbuf;
char *in_size = recvbuf + sizeof(recvbuf);
char *in_ack = recvbuf;

int stop_send = 0;
int stop_recv = 0;
fd_set rset, wset;

int fd;

/* 调试 */
void sig_int(int signo)
{
    logx("out_send = %d "
          "out_ack = %d "
          "in_recv = %d "
          "in_ack = %d "
          "stop_send=%d "
          "stop_recv=%d "
          "stdin_foucs=%d " 
          "fd_read=%d "
          "fd_write=%d "
          "stdout_foucs=%d ", 
           out_send - sendbuf,
           out_ack - sendbuf,
           in_recv - recvbuf,
           in_ack - recvbuf,
           stop_send,
           stop_recv, 
           FD_ISSET(STDIN_FILENO, &rset),
           FD_ISSET(fd, &rset),
           FD_ISSET(fd, &wset),
           FD_ISSET(STDOUT_FILENO, &wset));
}

void nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int max(int a, int b) { return a > b ? a : b; }

int main(int argc, char *argv[])
{
    log_err = 1;
    if (argc != 3) {
        logx("usage : %s <hostname/ip> <service/port>", argv[0]);
        exit(1);
    }

    if (fuck(SIGINT, sig_int) == SIG_ERR) {
        logx("fuck error!");
        exit(1);
    }

    if ((fd = tcp_connect(argv[1], argv[2])) < 0) {
        logx("tpc_connect error!");
        exit(1);
    }

    nonblock(STDIN_FILENO);
    nonblock(STDOUT_FILENO);
    nonblock(fd);

    FD_ZERO(&rset);
    FD_ZERO(&wset);

    int nready = 0;

    int maxfd = max(max(fd, STDIN_FILENO), STDOUT_FILENO);
    ssize_t nread, nwrite;
    for (;;) {

        if (!stop_send && out_size > out_send)
            FD_SET(STDIN_FILENO, &rset);

        if (out_send > out_ack)
            FD_SET(fd, &wset);

        if (!stop_recv && in_size > in_recv)
            FD_SET(fd, &rset);

        if (in_recv > in_ack)
            FD_SET(STDOUT_FILENO, &wset);

        if ((nready = select(maxfd + 1, &rset, &wset, NULL, NULL)) < 0) {
            if (errno != EINTR)
                logx("select error!");
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if ((nread = read(STDIN_FILENO, out_send, out_size - out_send)) < 0) {
                logx("read error!");
                FD_CLR(STDIN_FILENO, &rset);
            } else if (nread == 0) {
                logx("client send all data finished!");
                FD_CLR(STDIN_FILENO, &rset);
                stop_send = 1;
            } else {
                logx("recv %zd bytes from stdin", nread);
                out_send += nread;
                if (out_send >= out_size) {
                    logx("sendbuf is full, remove stdin readable event");
                    FD_CLR(STDIN_FILENO, &rset);
                }
            }
        }

        if (FD_ISSET(fd, &wset)) {
            if ((nwrite = write(fd, out_ack, out_send - out_ack)) < 0) {
                if (errno != EAGAIN && errno != EINTR)
                    logx("write error!");
                nwrite = 0;
            }
            out_ack += nwrite;
            logx("send %zd bytes to svr", nwrite);
            if (out_ack >= out_send) {
                FD_CLR(fd, &wset);
                out_ack = out_send = sendbuf;
                if (stop_send)
                    shutdown(fd, SHUT_WR);
            }
        }

        if (FD_ISSET(fd, &rset)) {
            if ((nread = read(fd, in_recv, in_size - in_recv)) < 0) {
                logx("read error!");
                FD_CLR(fd, &rset);
            } else if (nread == 0) {
                /* 服务器发送了 FIN 过来 */
                logx("server closed connection");
                FD_CLR(fd, &rset);
                stop_recv = 1;
                if (stop_send) {
                    /*
                     * 如果此时客户端已经发完所有数据
                     * 说明也已经接收完了服务器发送的
                     * 所有回显
                     */
                    if (in_ack >= in_recv) {
                        logx("echo client exit");
                        break;
                    }
                } else {
                    /*
                     * 如果服务器发送 FIN 过来时，客户端还没发完
                     * 数据，说明服务器崩溃了
                     */
                    logx("server terminated abnormally");
                }
            } else {
                in_recv += nread;
                logx("recv %zd bytes from svr", nread);
                if (in_recv >= in_size) {
                    logx("recvbuf is full, remove readable event");
                    FD_CLR(fd, &rset);
                }
            }
        }

        if (FD_ISSET(STDOUT_FILENO, &wset)) {
            if ((nwrite = write(STDOUT_FILENO, in_ack, in_recv - in_ack)) < 0) {
                if (errno != EINTR && errno != EAGAIN)
                    logx("write error!");
                nwrite = 0;
            }
            in_ack += nwrite;
            logx("send %zd bytes to stdout", nwrite);
            if (in_ack >= in_recv) {
                FD_CLR(STDOUT_FILENO, &wset);
                in_ack = in_recv = recvbuf;
                if (stop_recv) {
                    logx("echo client exit");
                    break;
                }
            }
        }
    }
}
