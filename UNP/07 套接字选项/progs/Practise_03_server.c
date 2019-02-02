#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include "AddrTool.h"
#include <errno.h>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_family = AF_INET;
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }

    struct pollfd clifds[1024];
    for (int i = 0; i < 1024; i++)
        clifds[i].fd = -1;
    
    clifds[0].fd = fd;
    clifds[0].events = POLLRDNORM;

    int connfd = -1;
    int nready = 0;
    int maxi = 0;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    for (;;) {
        if ((nready = poll(clifds, maxi + 1, -1)) < 0) {
            perror("poll error!");
            continue;
        }
        printf("nready = %d clifds[0].revents = %d\n", nready, clifds[0].revents);

        if (clifds[0].revents & POLLRDNORM) {
            if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept error!");
                if (--nready <= 0)
                    continue;
            }

            char *peerInfo = getPeerInfo(connfd);
            char *sockInfo = getSockInfo(connfd);
            printf("TCP connection established %s <---> %s\n", sockInfo, peerInfo);
            free(peerInfo);
            free(sockInfo);

            int i = 0;
            for (; i < 1024; i++) {
                if (clifds[i].fd < 0) {
                    clifds[i].fd = connfd;
                    clifds[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        for (int i = 1; i <= maxi; i++) {
            if (clifds[i].revents & POLLRDNORM) {
                ssize_t nread = -1, nwrite = -1;
                if ((nread = read(clifds[i].fd, buf, sizeof(buf))) < 0) {
                    perror("read error!");
                    if (errno == ECONNRESET) {
                        char *peerInfo = getPeerInfo(clifds[i].fd);
                        printf("cut client connection : %s\n", peerInfo);
                        free(peerInfo);

                        close(clifds[i].fd);
                        clifds[i].fd = -1;
                    }
                } else if (nread == 0) {
                    char *peerInfo = getPeerInfo(clifds[i].fd);
                    printf("cut client connection : %s\n", peerInfo);
                    free(peerInfo);

                    close(clifds[i].fd);
                    clifds[i].fd = -1;
                } else {
                    if ((nwrite = write(clifds[i].fd, buf, nread)) != nread) {
                        perror("write error!");
                    }
                }

                if (--nready <= 0)
                    break;
            }
        }
    }


    return 0;
}