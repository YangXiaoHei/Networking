#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 1000) < 0){
        perror("listen error!");
        exit(1);
    }

    int connfd = -1;
    char buf[1024];
    ssize_t nread = 0;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        char *srcInfo = getAddrInfo(&cliaddr);
        char *dstInfo = getSockInfo(connfd);
        printf("TCP connection from %s to %s\n", srcInfo, dstInfo);
        free(srcInfo);
        free(dstInfo);

        while ((nread = read(connfd, buf, sizeof(buf))) > 0) {
            buf[nread] = 0;
            printf("%s\n", buf);
        }

        if (nread < 0) {
            printf("read error! %s, cut connection\n", strerror(errno));
            close(connfd);
        } else {
            printf("client send FIN, cut connection\n");
            close(connfd);
        }
    }

    return 0;
}
