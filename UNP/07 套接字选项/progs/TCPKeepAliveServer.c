#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include "CommonTool.h"
#include "AddrTool.h"

int cb(int fd)
{
    int on = 1;
    int interval = 60;
    int idle = 5;
    int count = 3;

    if (setsockopt(fd, IPPROTO_TCP, SO_KEEPALIVE, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        return -1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0) {
        perror("setsockopt error!");
        return -2;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
        perror("setsockopt error!");
        return -3;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0) {
        perror("setsockopt error!");
        return -4;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], cb)) < 0) {
        perror("tcp_server_cb error!");
        exit(1);
    }

    int connfd = -1;
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno != EINTR)
                perror("accept error!");
            continue;
        }
        char *info = getPeerInfo(connfd);
        printf("tcp connection from %s\n", info);
        free(info);

        for (;;)
            pause();
    }
}
