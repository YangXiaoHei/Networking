#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port/service>\n", argv[0]);
        exit(1);
    }

    const char *service = argv[1];

    struct addrinfo hints, *res, *ressave;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    int error = 0;
    if ((error = getaddrinfo(NULL, service, &hints, &res)) != 0) {
        printf("getaddrinfo error for %s : %s\n", service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    int listenfd = -1;
    do {
        if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create error!");
            continue;
        }

        int on = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt error!");
            close(listenfd);
            continue;
        }

        if (bind(listenfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("bind error!");
            close(listenfd);
            continue;
        }

        break;

    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("create a listenable fd fail!\n");
        exit(1);
    }

    if (listen(listenfd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }

    char buf[1024];
    int connfd = -1;
    time_t ticks = time(NULL);
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            perror("accept error!");
            exit(1);
        }

        snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
        if (write(connfd, buf, strlen(buf)) != strlen(buf)) {
            perror("write error!");
            exit(1);
        }
        close(connfd);
    }
    return 0;
}