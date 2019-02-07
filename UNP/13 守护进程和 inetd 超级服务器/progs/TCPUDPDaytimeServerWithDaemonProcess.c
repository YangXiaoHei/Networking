#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }

    const char *hostname = NULL;
    const char *service = NULL;

    if (argc == 2) {
        service = argv[1];
    } else {
        hostname = argv[1];
        service = argv[2];
    }

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = 0;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int listenfd = -1, udpfd = -1;
    do {
        if (res->ai_socktype == SOCK_DGRAM) {
            if (udpfd >= 0) {
                printf("same service use duplicated udp protocol\n");
                exit(1);
            }
            if ((udpfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
                perror("udp socket fd create fail!");
                continue;
            }
            int sendbuff = 220 << 10;
            if (setsockopt(udpfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
                perror("udp fd set send buffer error!");
                goto udp_fd_err;
            }
            if (bind(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("udp fd bind error!");
                goto udp_fd_err;
            }
            continue;
        udp_fd_err:
            close(udpfd);
            udpfd = -1;
        } else if (res->ai_socktype == SOCK_STREAM) {
            if (listenfd >= 0) {
                printf("same service use duplicated tcp protocol\n");
                exit(1);
            }
            if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
                perror("tcp socket fd create fail!");
                continue;
            }
            int on = 1;
            if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
                perror("setsockopt error!");
                goto tcp_fd_err;
            }
            if (bind(listenfd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("tcp fd bind error!");
                goto tcp_fd_err;
            }
            if (listen(listenfd, 1000) < 0) {
                perror("tcp fd listen error!");
                goto tcp_fd_err;
            }
            continue;
        tcp_fd_err:
            close(listenfd);
            listenfd = -1;
        } else {
            printf("unsupported socket type! ignored");
            continue;
        }
    } while ((res = res->ai_next) != NULL);

    if (udpfd < 0 && listenfd < 0) {
        printf("service %s not use neither udp nor tcp\n", service);
        exit(1);
    }

    char buf[1024];
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int nready = 0;
    int connfd = -1;
    int maxfd = udpfd > listenfd ? udpfd : listenfd;
    fd_set rset, allset;

    if (listenfd > 0)
        FD_SET(listenfd, &allset);
    if (udpfd > 0)
        FD_SET(udpfd, &allset);
    for (;;) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            sleep(1);
            continue;
        }

        if (listenfd > 0) {
            if (FD_ISSET(listenfd, &rset)) {
                bzero(&cliaddr, sizeof(cliaddr));
                socklen_t len = clilen;
                if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
                    perror("accept error!");
                    if (--nready <= 0)
                        continue;
                }

                char *addrInfo = getAddrInfo(&cliaddr);
                printf("tcp connection from %s new connfd = %d\n", addrInfo, connfd);
                free(addrInfo);

                time_t ticks = time(NULL);
                snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
                if (write(connfd, buf, strlen(buf)) != strlen(buf)) {
                    perror("write error!");
                }
                close(connfd);
                connfd = -1;
                if (--nready <= 0)
                    continue;
            }
        }

        if (udpfd > 0) {
            if (FD_ISSET(udpfd, &rset)) {
                bzero(&cliaddr, sizeof(cliaddr));
                char c = 0;
                socklen_t len = clilen;
                if (recvfrom(udpfd, &c, 1, 0, (struct sockaddr *)&cliaddr, &len) < 0) {
                    perror("recvfrom error!");
                    if (--nready <= 0)
                        continue;
                }
                char *addrInfo = getAddrInfo(&cliaddr);
                printf("udp req from %s\n", addrInfo);
                free(addrInfo);

                time_t ticks = time(NULL);
                snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
                if (sendto(udpfd, buf, strlen(buf), 0, (struct sockaddr *)&cliaddr, len) < 0) {
                    perror("sendto error!");
                }
                if (--nready <= 0)
                    continue;
            }
        }
    }

    return 0;
}