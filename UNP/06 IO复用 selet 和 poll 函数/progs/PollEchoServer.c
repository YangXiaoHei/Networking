#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include "AddrTool.h"

#define OPEN_MAX 1024

int main(int argc, char const *argv[])
{
    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listenfd error!");
        exit(1);
    }

    struct pollfd clients[OPEN_MAX];
    for (int i = 0; i < OPEN_MAX; i++) 
        clients[i].fd = -1;

    clients[0].fd = listenfd;
    clients[0].events = POLLRDNORM;

    int connfd = -1;
    int nready = 0;
    int maxi = 0;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    for (;;) {
        if ((nready = poll(clients, maxi + 1, -1)) < 0) {
            perror("poll error!");
            exit(1);
        }

        /* 监听套接字 */
        if (clients[0].revents & POLLRDNORM) {
            if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept error!");
                exit(1);
            }

            char *peerinfo = getPeerInfo(connfd);
            char *sockinfo = getSockInfo(connfd);
            printf("%s <---> %s established TCP connection\n", sockinfo, peerinfo);
            free(peerinfo);
            free(sockinfo);

            int i = 1;
            for (; i < OPEN_MAX; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = connfd;
                    clients[i].events = POLLRDNORM;
                    break;
                }
            }
            if (i == OPEN_MAX) {
                printf("too many fds\n");
                exit(1);
            }

            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        /* 客户端套接字 */
        for (int i = 1; i <= maxi; i++) {
            if (clients[i].revents & POLLRDNORM) {
                ssize_t nread = -1, nwrite = -1;
                if ((nread = read(clients[i].fd, buf, sizeof(buf))) < 0) {
                    if (errno == ECONNRESET) {

                        char *peerinfo = getPeerInfo(connfd);
                        printf("TCP connection reset by peer %s\n", peerinfo);
                        free(peerinfo);

                        close(clients[i].fd);
                        clients[i].fd = -1;

                    } else {
                        perror("read error!");
                        exit(1);
                    }
                } else if (nread == 0) {
                    char *peerinfo = getPeerInfo(connfd);
                    printf("client cut connection [fd=%d][peerinf=%s]\n", clients[i].fd, peerinfo);
                    free(peerinfo);
                    close(clients[i].fd);
                    clients[i].fd = -1;
                } else {
                    if ((nwrite = write(clients[i].fd, buf, nread)) != nread) {
                        perror("nwrite error!");
                        exit(1);
                    }
                }

                if (--nready <= 0) 
                    break;
            }
        }
    }

    return 0;
}