#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"
#include <signal.h>

int fd;
static sig_atomic_t svr_crash;

void  sig_usr1(int signo)
{
    svr_crash = 1;
}

void *read_from_stdin(void *arg)
{
    char buf[8192];
    ssize_t nread, nwrite;
    while ((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        ssize_t ntowrite = nread;
        if (write(fd, buf, ntowrite) != ntowrite) {
            logx("write error! %s", strerror(errno));
            break;
        }
        logx("send %zd bytes to server", ntowrite);
    }
    if (nread == 0) {
        logx("client send all data");
    } else if (!svr_crash) {
        /* 不是收到信号被中断的情况，read error */ 
        logx("read error! %s", strerror(errno));
    } else {
        /* 收到信号时该线程还未退出，说明服务器提前终止 */
        logx("server terminated abnormally");
    }

    shutdown(fd, SHUT_WR);
    logx("read_from_stdin exit");
    return (void *)NULL;
}

/*
 * 如果服务器正常服务的话，那么流程应该是
 * 客户端传完所有数据，关闭连接，随后服务器
 * 再关闭连接，如果出现服务器提前关闭连接
 * 的情况，那么算作服务器非正常终止
 */
void *read_from_sockfd(void *arg)
{
    char buf[8192];
    ssize_t nread, nwrite;
    for (;;) {
        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            logx("read error! %s", strerror(errno));
            break;
        } else if (nread == 0) {
            logx("server closed connection");
            break;
        }
        logx("recv %zd bytes from server", nread);
        if (write(STDOUT_FILENO, buf, nread) != nread) {
            logx("write error! %s", strerror(errno));
            break;
        }
    }
    logx("read_from_sockfd exit");
    /* 向指定的线程发信号, 服务器正常服务的情况下
     * 信号发送的目标线程应该已经退出, 从而不会收到该信号 */
    pthread_kill(*(pthread_t *)arg, SIGUSR1);
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        logx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        logx("tcp_connect_cb error! %s", strerror(errno));
        exit(1);
    }

    if (fuck_int(SIGUSR1, sig_usr1) == SIG_ERR) {
        logx("fuck error ! %s", strerror(errno));
        exit(1);
    }
    int n;
    pthread_t tid1, tid2;
    if ((n = pthread_create(&tid1, NULL, read_from_stdin, NULL)) < 0) {
        logx("pthread_create %s", strerror(n));
        exit(1);
    }
    if ((n = pthread_create(&tid2, NULL, read_from_sockfd, (void *)&tid1)) < 0) {
        logx("pthread_create %s", strerror(n));
        exit(1);
    }

    if (pthread_join(tid1, NULL) < 0) {
        logx("pthread_join %s", strerror(n));
        exit(1);
    }
    if (pthread_join(tid2, NULL) < 0) {
        logx("pthread_join %s", strerror(n));
        exit(1);
    }
}
