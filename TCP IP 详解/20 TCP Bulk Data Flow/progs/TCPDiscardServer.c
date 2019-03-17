#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "../../tool/CommonTool.h"
#include "../../tool/TimeTool.h"
#include "../../tool/AddrTool.h"
extern int log_err;

int cb(int fd)
{
    /*
     * int mss = 1024;
     * if (setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(mss)) < 0) 
     *     return -1;
     * int win = 4096;
     * if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &win, sizeof(win)) < 0)
     *     return -1;
     */
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        logx("usage : %s <port/service>", argv[0]);
        exit(1);
    }
    log_err = 1;
    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], cb)) < 0) {
        logx("tcp_server_cb error!");
        exit(1);
    }

    int connfd = -1;
    char buf[1024];
    ssize_t nwrite, nread;
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            logx("accept error!");
            continue;
        }
        char *info = getPeerInfo(connfd);
        logx("tcp connection from %s", info);

        for (;;) {
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                logx("read error! from %s", info);
                break;
            } else if (nread == 0) {
                logx("client %s closed connection", info);
                break;
            }
            logx("recv %zd bytes data from %s", nread, info);
        }
        close(connfd);
        free(info);
    }
}
