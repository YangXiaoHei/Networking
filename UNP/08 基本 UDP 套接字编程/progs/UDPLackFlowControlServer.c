#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

static int totalRecvdCount;
static void recvfrom_handler(int signo) 
{
    printf("total recvd packet = %d\n", totalRecvdCount);
    exit(0);
}

typedef void(*signal_handler)(int signo);

signal_handler fuck(int signo, signal_handler handler)
{
    struct sigaction oldact, newact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = handler;
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);

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
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    fuck(SIGINT, recvfrom_handler);

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    char buf[3000];
    for (;;) {
        if (recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL) < 0) {
            perror("recvfrom error!");
            exit(1);
        }
        totalRecvdCount++;
    }

    return 0;
}