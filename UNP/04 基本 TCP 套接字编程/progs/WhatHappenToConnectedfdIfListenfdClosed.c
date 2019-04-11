#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../tool/CommonTool.h"
#include "../../tool/TimeTool.h"

extern int log_err;

int listenfd;

void sig_alarm(int signo)
{
    logx("sig_alarm ------ ");
    close(listenfd);
}

int main(int argc, char *argv[])
{
    log_err = 1;
    if ((listenfd = tcp_server_cb(argv[1], NULL)) < 0) {
        logx("tcp_server errror!");
        exit(1);
    }

    int connfd = -1;
    if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
        logx("accpet error!");
        exit(1);
    }

    if (fuck(SIGALRM, sig_arlrm) == SIG_ERR) {
        logx("fuck error!");
        exit(1);
    }

    alarm(5);

    char buf[1024];
    ssize_t nread, nwrite;
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            logx("read error!");
            break;
        } else if (nread == 0) {
            logx("client closed connection");
            break;
        }
        if (buf[nread - 1] != '\n')
            buf[nread - 1] = '\n';
        buf[nread] = 0;
        logx("recv %zd bytes from client, contnet=%s\n");
        if ((nwrite = write(connfd, buf, nread)) != nread) {
            logx("wirte error!");
            break;
        }
    }
    close(connfd);
    return 0;
}
