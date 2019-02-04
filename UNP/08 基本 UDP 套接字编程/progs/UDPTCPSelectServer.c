#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/select.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    int udpfd = -1;
    if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    if (bind(udpfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    int fds[1024];
    for (int i = 0; i < 1024; i++)
        fds[i] = -1;

    fds[0] = udpfd;
    fds[1] = listenfd;

    fd_set rset, allset;
    FD_SET(fds[0], &allset);
    FD_SET(fds[1], &allset);
    rset = allset;

    int maxfd = udpfd > listenfd ? udpfd : listenfd;
    int maxi = 2;

    char buf[2048];
    int connfd = -1;
    ssize_t nwrite = 0, nread = 0;
    int nready = 0;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            continue;
        }

        /* listenfd */
        if (FD_ISSET(fds[1], &rset)) {
            socklen_t len = clilen;
            if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
                perror("accept error!");
                if (--nready <= 0)
                    continue;
            }

            int i = 2;
            for (;i < 1024; i++) {
                if (fds[i] < 0) {
                    fds[i] = connfd;
                    FD_SET(connfd, &allset);
                    break;
                }
            }

            if (i == 1024) {
                printf("too many tcp connections\n");
                close(connfd);
                if (--nready <= 0)
                    continue;
            }

            char *cliinfo = getAddrInfo(&cliaddr);
            char *localinfo = getAddrInfo(&svraddr);
            printf("TCP connection established %s <---> %s\n", localinfo, cliinfo);
            free(cliinfo);
            free(localinfo);

            if (connfd > maxfd)
                maxfd = connfd;

            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        /* udpfd */
        if (FD_ISSET(fds[0], &rset)) {
            socklen_t len = clilen;
            if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                perror("recvfrom error!");
                exit(1);
            }
            buf[nread] = 0;
            char *cliinfo = getAddrInfo(&cliaddr);
            printf("~~~~~ UDP data from %s ~~~\n", cliinfo);
            free(cliinfo);
            printf("%s", buf);
            if (buf[nread - 1] != '\n')
                printf("\n");
            printf("~~~~~ UDP ~~~\n");

            if ((nwrite = sendto(udpfd, buf, nread, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr))) < 0) {
                perror("sendto error!");
                exit(1);
            }

            if (--nready <= 0)
                continue;
        }

        for (int i = 2; i <= maxi; i++) {
            if (FD_ISSET(fds[i], &rset)) {
                if ((nread = read(fds[i], buf, sizeof(buf))) < 0) {
                    perror("read error!");
                    if (errno == ECONNRESET) {
                        printf("cut TCP client connection\n");
                        close(fds[i]);
                        FD_CLR(fds[i], &allset);
                        fds[i] = -1;
                    }
                } else if (nread == 0) {
                    printf("read EOF\n");
                    printf("cut TCP client connection\n");
                    close(fds[i]);
                    FD_CLR(fds[i], &allset);
                    fds[i] = -1;
                } else {
                    char *cliinfo = getAddrInfo(&cliaddr);
                    buf[nread] = 0;
                    printf("~~~~~ TCP data from %s ~~~\n", cliinfo);
                    printf("%s", buf);
                    if (buf[nread - 1] != '\n')
                        printf("\n");
                    printf("~~~~~ TCP ~~~\n");
                    free(cliinfo);

                    if ((nwrite = write(fds[i], buf, nread)) != nread) {
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