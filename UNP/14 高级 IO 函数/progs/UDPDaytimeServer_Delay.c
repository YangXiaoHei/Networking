#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "../../tool/AddrTool.h"

long curtimeus()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3) {
        printf("usage : %s <port/service> [delay_seconds]\n", argv[0]);
        exit(1);
    } 

    const char *hostname = NULL;
    const char *service = NULL;
    int delaySeconds = 5;

    if (argc == 2) {
        service = argv[1];
    } else {
        service = argv[1];
        delaySeconds = atoi(argv[2]);
    }

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int udpfd = -1;
    do {    
        if ((udpfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket error!");
            continue;
        }
        if (bind(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("bind error!");
            close(udpfd);
            udpfd = -1;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("%s %s udp server start fail!\n", hostname, service);
        exit(1);
    }

    for (;;) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        char c = 0;
        if (recvfrom(udpfd, &c, 1, 0, (struct sockaddr *)&cliaddr, &clilen) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        char *addrInfo = getAddrInfo(&cliaddr);
        printf("udp req from %s\n", addrInfo);
        free(addrInfo);

        time_t ticks = time(NULL);
        char buf[1024];
        snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));

        long beg = curtimeus();
        for (;;) {
            if ((curtimeus() - beg) / 1000000 >= delaySeconds) {
                if (sendto(udpfd, buf, strlen(buf), 0, (struct sockaddr *)&cliaddr, clilen) < 0) {
                    perror("sendto error!");
                    exit(1);
                }
                break;
            }
        }
    }
    
    return 0;
}
