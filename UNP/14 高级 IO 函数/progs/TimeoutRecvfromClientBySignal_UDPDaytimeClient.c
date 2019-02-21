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
#include <signal.h>
#include "../../tool/AddrTool.h"

void recvfrom_terminate(int signo) { return; }

typedef void(*sig_handler)(int);

sig_handler fuck(int signo, sig_handler newhandler) 
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = newhandler;

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        newact.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        newact.sa_flags |= SA_RESTART;
#endif
    }

    if (sigaction(signo, &newact, &oldact) < 0)
        return SIG_ERR;
    return oldact.sa_handler;
}

int main(int argc, char const *argv[])
{
    if (argc != 3 && argc != 4) {
        printf("usage : %s <ip/hostname> <port/service> [recv_timeout]\n", argv[0]);
        exit(1);
    } 

    const char *hostname = argv[1];
    const char *service = argv[2];
    int recv_timeout = 5;

    if (argc == 4) {
        recv_timeout = atoi(argv[3]);
    }

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
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
        if (connect(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("%s %s udp client start fail!\n", hostname, service);
        exit(1);
    }    

    char c = 1;
    if (sendto(udpfd, &c, 1, 0, NULL, 0) < 0) {
        perror("sendto error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0;
    sig_handler oldhandler = fuck(SIGALRM, recvfrom_terminate);
    alarm(recv_timeout);
    if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
        if (errno == EINTR)
            errno = ETIMEDOUT;
        perror("recvfrom error!");
        exit(1);
    }
    alarm(0);
    fuck(SIGALRM, oldhandler);
    buf[nread] = 0;
    printf("%s\n", buf);

    return 0;
}
