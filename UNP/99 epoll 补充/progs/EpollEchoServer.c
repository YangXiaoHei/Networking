#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "AddrTool.h"

int tcp_server(unsigned short port)
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create");
        return -1;
    }
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        return -3;
    }   
 
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        return -2;
    }
    if (listen(fd, 1000) < 0) {
        perror("listen error!");
        return -4;
    }
    return fd;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = tcp_server(atoi(argv[1]))) < 0) {
        perror("tcp_server error!");
        exit(1);
    }
    
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int efd = -1;
    if ((efd = epoll_create(1000000)) < 0) {
        perror("epoll_create fail!");
        exit(1);
    }
    struct epoll_event evlist[1024];
    struct epoll_event listenev;
    listenev.data.fd = listenfd;
    listenev.events = EPOLLIN;
    
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &listenev) < 0) {
        perror("add listenfd epoll_ctl error!");
        exit(1);
    }

    int nready = 0;
    int connfd = -1;
    ssize_t nread, nwrite;
    for (;;) {
        if ((nready = epoll_wait(efd, evlist, 1024, -1)) < 0) {
            perror("epoll_wait error!");
            continue;
        }
        socklen_t len = clilen;
        for (int i = 0; i < nready; i++) {
            if (evlist[i].events & EPOLLIN) {
                if (evlist[i].data.fd == listenfd) {
                    /* new connection */
                   if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
                        perror("accept error!");
                        continue;
                    } 
                    char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
                    printf("tcp connection from %s\n", srcInfo);
                    free(srcInfo);
                    
                    struct epoll_event ev;
                    ev.data.fd = connfd;
                    ev.events = EPOLLIN | EPOLLRDHUP;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
                        perror("add connfd, epoll_ctl error!");
                        exit(1);
                    }
                    continue;
                }
                /* connected fd readable event occur */
                int fd = evlist[i].data.fd;
                char buf[1024];
                if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                    perror("close reset connection");
                    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &evlist[i]) < 0) {
                        perror("del connfd, epoll_ctl error!");
                        exit(1);
                    }
                    close(fd);
                } else if (nread == 0) {
                    printf("client close connection\n");
                    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &evlist[i]) < 0) {
                        perror("del connfd, epoll_ctl error!");
                        exit(1);
                    }
                    close(fd);
                } else {
                    if ((nwrite = write(fd, buf, nread)) != nread) {
                        perror("write error!");
                        exit(1);
                    }
                }               
            }
        }
    }
}
