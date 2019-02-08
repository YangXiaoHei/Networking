#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt erro!");
        exit(1);
    }

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }
    socklen_t clilen = sizeof(cliaddr);
    int connfd = -1;
    if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
        perror("accept error!");
        exit(1);
    }

    char *srcInfo = getAddrInfo(&cliaddr);
    char *dstInfo = getSockInfo(connfd);
    printf("TCP connection from %s to %s\n", srcInfo, dstInfo);
    free(srcInfo);
    free(dstInfo);

    ssize_t nCanReadInRecvBuffer = 0;
    for (;;) {
        if (ioctl(connfd, FIONREAD, &nCanReadInRecvBuffer) < 0) {
            perror("ioctl error!");
            exit(1);
        }
        sleep(1);
        printf("there are [ %zd ] bytes in recv buffer\n", nCanReadInRecvBuffer);
    }


    return 0;
}