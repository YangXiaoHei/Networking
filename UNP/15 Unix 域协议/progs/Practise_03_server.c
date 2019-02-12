#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3) {
        printf("usage : %s [ip/hostname] <port/service>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

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
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    int listenfd = -1;
    do {
        if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }
        int on = 1;
        if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
            perror("setsockopt error!");
            goto err;
        }
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("bind error!");
            goto err;
        }
        if (listen(listenfd, 100000) < 0) {
            perror("listen error!");
            goto err;
        }
        break;
    err:
        close(listenfd);
        listenfd = -1;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("server start error!\n");
        exit(1);
    }

    ressave = res;

    int connfd = -1;
    char buf[1024];
    ssize_t nwrite = 0;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        socklen_t len = clilen;
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            continue;
        }

        char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        printf("TCP connection from %s\n", srcInfo);
        free(srcInfo);

        time_t ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
        for (int i = 0; i < strlen(buf); i++)
            if (write(connfd, &buf[i], 1) != 1) {
                perror("write error!");
                close(connfd);
                continue;
            }
        close(connfd);
    }

    return 0;
}