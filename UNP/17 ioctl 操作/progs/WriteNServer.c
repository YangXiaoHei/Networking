#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    struct linger l;
    l.l_onoff = 1;
    l.l_linger = 0;
    if (setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }

    int connfd = -1;
    char buf[4096];
    ssize_t nread = 0;
    ssize_t totalread = 0;
    for (;;) {
        totalread = 0;
        socklen_t len = clilen;
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            continue;
        }

        char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        printf("TCP connection from %s\n", srcInfo);
        free(srcInfo);

        while ((nread = read(connfd, buf, sizeof(buf))) != 0) {
            if (nread < 0 && errno != ECONNRESET)
                nread = 0;
            totalread += nread;

            if (totalread > (1 << 20)) {
                close(connfd);
                break;
            }
        }
        printf("total read = %ld\n", totalread);
    }
    return 0;
}