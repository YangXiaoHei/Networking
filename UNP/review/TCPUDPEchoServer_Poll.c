#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include "../tool/CommonTool.h"
#include "../tool/AddrTool.h"
#include "../tool/TimeTool.h"

#define MAXCLI 1024

int check_valid(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    return getpeername(fd, (struct sockaddr *)&addr, &len) == 0;
}

int cb(int fd)
{
    int on = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        logx("usage : %s <port/service>", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], cb)) < 0) {
        logx("tcp_server_cb error!");
        exit(1);
    }

    int udpfd = -1;
    if ((udpfd = udp_server_cb(argv[1], cb)) < 0) {
        logx("udp_server_cb error!");
        exit(1);
    }

    struct pollfd client[MAXCLI + 2];
    for (int i = 0; i < MAXCLI + 2; i++)
        client[i].fd = -1;

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;

    client[1].fd = udpfd;
    client[1].events = POLLRDNORM;

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    ssize_t nread, nwrite;
    char buf[8192];
    int connfd = -1;
    int maxi = 1;
    int nfd = 2;
    int nready = 0;
    for (;;) {
        if ((nready = poll(client, maxi + 1, -1)) < 0) {
            if (errno != EINTR)
                logx("poll error!");
            continue;
        }

        /* TCP 监听套接字 */
        if (client[0].revents & POLLRDNORM) {
            if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
                logx("accept error!");
                goto listen_end;
            }
            if (!check_valid(connfd)) {
                logx("tcp connection invalid! connfd = %d", connfd);
                goto listen_end;
            }
            char *src = getPeerInfo(connfd);
            logx("new tcp connection from %s", src);
            int i = 0;
            for (i = 2; i < MAXCLI + 2; i++)
                if (client[i].fd < 0)
                    break;
            if (i == MAXCLI + 2) {
                logx("service client beyond limit! reject connection from %s", src);
                close(connfd);
                free(src);
                goto listen_end;
            }
            client[i].fd = connfd;
            client[i].events = POLLRDNORM;
            if (i > maxi) 
                maxi = i;

            free(src);
        listen_end:
            if (--nready <= 0)
                continue;
        }

        /* UDP 套接字  */
        if (client[1].revents & POLLRDNORM) {
            int fd = client[1].fd;
            socklen_t len = clilen;
            if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                logx("recvfrom error!");
                goto udp_end;
            } 
            char *info = getAddrInfo((struct sockaddr *)&cliaddr);
            logx("recv %zd bytes udp data from %s", nread, info);
            free(info);
            if (sendto(fd, buf, nread, 0, (struct sockaddr *)&cliaddr, len) < 0) {
                logx("sendto error!");
                goto udp_end;
            }
        udp_end:
            if (--nready <= 0)
                continue;
        }

        /* TCP 已连接套接字 */
        for (int i = 2; i <= maxi; i++) {
            int fd = client[i].fd;
            if (fd < 0)
                continue;
            if (client[i].revents & POLLRDNORM) {
                int erroccur = 0;
                if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                    logx("read error!");
                    erroccur = 1;
                    goto read_end;
                } else if (nread == 0) {
                    logx("client closed connection");
                    erroccur = 1;
                    goto read_end;
                }
                char *src = getPeerInfo(fd);
                logx("recv %zd tcp bytes from %s", nread, src);

                if (write(fd, buf, nread) != nread) {
                    logx("write error to %s", src);
                    erroccur = 1;
                    free(src);
                    goto read_end;
                }

                free(src);
            read_end:
                if (erroccur) {
                    client[i].fd = -1;
                    close(fd);
                }
                if (--nready <= 0)
                    break;
            }
        }
    } 
}
