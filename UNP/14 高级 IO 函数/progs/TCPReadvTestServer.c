#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/uio.h>
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
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error1");
        exit(1);
    }

    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    int connfd = -1;

    char buf[27];
    bzero(buf, sizeof(buf));

    int cursor = 0;
    struct iovec iov[100];
    iov[cursor].iov_base = buf;
    iov[cursor].iov_len = 3; /* 3 */
    cursor++;

    iov[cursor].iov_base = buf + 3;
    iov[cursor].iov_len = 8;
    cursor++;

    iov[cursor].iov_base = buf + 3 + 8;
    iov[cursor].iov_len = 5;
    cursor++;

    iov[cursor].iov_base = buf + 3 + 8 + 5;
    iov[cursor].iov_len = 6;
    cursor++;

    iov[cursor].iov_base = buf + 3 + 8 + 5 + 6;
    iov[cursor].iov_len = 4;
    cursor++;

    ssize_t nread = 0;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            continue;
        }

        char *srcInfo = getAddrInfo(&cliaddr);
        char *dstInfo = getSockInfo(connfd);
        printf("TCP connection from %s to %s\n", srcInfo, dstInfo);
        free(srcInfo);
        free(dstInfo);

        while ((nread = readv(connfd, iov, cursor)) > 0) {
            buf[nread] = 0;
            printf("%d bytes --- %s | ",nread, buf);
            for (int i = 0; i < sizeof(buf); i++)
                printf("%2c", buf[i]);
            printf("\n");
        }
        if (nread < 0) {
            printf("readv error! %s\n", strerror(errno));
            exit(1);
        } else {
            printf("client send FIN cut connection\n");
            close(connfd);
        } 
    }

    return 0;
}
