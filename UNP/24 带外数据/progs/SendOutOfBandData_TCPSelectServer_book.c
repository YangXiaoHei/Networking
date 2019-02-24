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
    
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = -1;
    if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
        perror("accept error!");
        exit(1);
    }
    
    fd_set rset, xset;
    FD_ZERO(&rset);
    FD_ZERO(&xset);
    FD_SET(fd, &rset);
    
    char buf[1024];
    ssize_t nread, nwrite;
    for (;;) {
        FD_SET(connfd, &rset);
        FD_SET(connfd, &xset);
        
        if (select(connfd + 1, &rset, NULL, &xset, NULL) < 0) {
            perror("seelct error!");
            continue;
        } 

        if (FD_ISSET(connfd, &xset)) {
            if ((nread = recv(connfd, buf, sizeof(buf), MSG_OOB)) < 0) {
                perror("recv error!");
                continue;
            }
            buf[nread] = 0;
            printf("recv %zd bytes OOB data, %s\n", nread, buf);
        }

        if (FD_ISSET(connfd, &rset)) {
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                continue;
            } else if (nread == 0) {
                printf("client cut connection\n");
                close(connfd);
                exit(0);
            }
            buf[nread] = 0;
            printf("read %zd bytes normal data, %s\n", nread, buf);
        }
    }
}
