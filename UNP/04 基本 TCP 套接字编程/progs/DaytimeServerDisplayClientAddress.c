#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

ssize_t writen(int fd, void *vptr, size_t n) 
{
    ssize_t nleft = n;
    char *ptr = vptr;
    ssize_t nwrite = 0;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        } 
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : ./%s <bind_port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in cliaddr, svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    char ipbuf[1024];
    time_t ticks = 0;
    char rspbuf[1024];
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }
        printf("connection from %s:%d\n", 
            inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf)), 
            ntohs(cliaddr.sin_port));

        snprintf(rspbuf, sizeof(rspbuf), "%s\n", ctime(&ticks));
        if (writen(connfd, rspbuf, strlen(rspbuf)) < 0) {
            perror("write error!");
            exit(1);
        }
        close(connfd);
    }

    return 0;
}