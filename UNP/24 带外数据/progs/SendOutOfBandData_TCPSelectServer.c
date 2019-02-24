#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include "AddrTool.h"

int main(int argc, char *argv[])
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
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }
    
    int connfd = -1;
    fd_set rset, xset;
    fd_set rallset, xallset;

    FD_ZERO(&rset);
    FD_ZERO(&xset);
    FD_ZERO(&rallset);
    FD_ZERO(&xallset);

    FD_SET(fd, &rallset);
    int maxfd = fd;
    int nready = 1;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    ssize_t nread, nwrite;
    for (;;) {
        rset = rallset;
        xset = xallset;
        if ((nready = select(maxfd + 1, &rset, NULL, &xset, NULL)) < 0) {
            perror("select error!");
            continue;
        }
        /*
         * 监听套接字可读
         */
        if (FD_ISSET(fd, &rset)) {
            if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept error!");
                continue;
            }
            if (connfd > maxfd)
                maxfd = connfd;
            FD_SET(connfd, &rallset);
            FD_SET(connfd, &xallset);
            char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
            printf("tcp connection from %s\n", srcInfo);
            free(srcInfo);
        }
        /*
         * 客户端套接字普通数据可读
         */
        if (FD_ISSET(connfd, &rset)) {
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                continue;
            } else if (nread == 0) {
                printf("client cut connection\n");
                close(connfd);
                FD_CLR(connfd, &rallset);
                FD_CLR(connfd, &xallset);
                continue;
            } else {
                buf[nread] = 0;
                printf("recv %zd bytes, %s\n", nread, buf);
            }
        }
        /*
         * 紧急数据可读 
         */
        if (FD_ISSET(connfd, &xset)) {
            if ((nread = recv(connfd, buf, sizeof(buf), MSG_OOB)) <= 0) {
                perror("recv data error!");
                continue;
            } else {
                buf[nread] = 0;
                printf("recv %zd bytes OOB data, %s\n", nread, buf);
            }
        }
    }
}
