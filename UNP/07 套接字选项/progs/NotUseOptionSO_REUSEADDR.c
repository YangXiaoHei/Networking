#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <bind_ip><port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket create fd error!");
        exit(1);
    } 

    struct sockaddr_in svraddr, cliaddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    /* SO_RCVBUF 必须在 listen 前调用，必须在 connect 前调用 */

    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    ssize_t nread = 0;
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        char *peerInfo = getPeerInfo(connfd);
        char *sockInfo = getSockInfo(connfd);
        printf("%s <---> %s\n", peerInfo, sockInfo);
        free(peerInfo);
        free(sockInfo);

again:
        while ((nread = read(connfd, buf, sizeof(buf))) > 0) {
            buf[nread] = 0;
            if (fputs(buf, stdout) == EOF) {
                if (ferror(stdout)) {
                    printf("fputs error!");
                    close(connfd);
                }
            }
        } 

        if (nread == 0) {
            printf("client close connection\n");
            close(connfd);
        } else if (nread < 0) {
            if (errno == EINTR)
                goto again;
            perror("read error!");
            exit(1);
        }
    }

    return 0;
}