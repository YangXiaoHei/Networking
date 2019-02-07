#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include "AddrTool.h"

void connect_terminate(int signo) { return; }

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
        printf("usage : %s <ip> <port> [timeout]\n", argv[0]);
        exit(1);
    }

    const char *ip = argv[1];
    unsigned short port = atoi(argv[2]);
    int timeout = 5;

    if (argc == 4)
        timeout = atoi(argv[3]);

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = inet_addr(ip);

    sig_handler oldhandler = fuck(SIGALRM, connect_terminate);
    if (alarm(timeout) != 0) {
        printf("alarm already set!\n");
    }
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        close(fd);
        if (errno == EINTR)
            errno = ETIMEDOUT;
        perror("connect error!");
        exit(1);
    }
    alarm(0);
    fuck(SIGALRM, oldhandler);

    char *addrInfo = getAddrInfo(&svraddr);
    printf("connect to %s succ!\n", addrInfo);
    free(addrInfo);

    return 0;
}