#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "AddrTool.h"
#include <time.h>

int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3) {
        printf("usage : %s [ip/hostname] <port/service>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *service = NULL, *hostname = NULL;
    if (argc == 2) {
        service = argv[1];
    } else {
        hostname = argv[1];
        service = argv[2];
    }

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;

    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }

        if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("bind error!");
            close(fd);
            continue;
        }

        break;

    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("daytime udp server start fail!\n");
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char c = 0;

    for (;;) {
        if (recvfrom(fd, &c, 1, 0,  (struct sockaddr *)&cliaddr, &clilen) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        char *addrInfo = getAddrInfo(&cliaddr);
        printf("udp req from %s\n", addrInfo);
        free(addrInfo);

        time_t ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
        if (sendto(fd, buf, strlen(buf), 0,  (struct sockaddr *)&cliaddr, clilen) < 0) {
            perror("sendto error!");
            exit(1);
        }
    }
    

    return 0;
}