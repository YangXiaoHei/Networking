#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "server_heartbeat_helper.h"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

static int nprobes;
static int maxnprobes;
static int nsec;
static int sockfd;

typedef void(*sig_handler)(int);

static void sig_urg(int);
static void sig_alrm(int);

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

void heartbeat_server(int fd, int sec, int max)
{
    sockfd = fd;
    nsec = sec;
    maxnprobes = max;

    if (fuck(SIGURG, sig_urg) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    fcntl(sockfd, F_SETOWN, getpid());
    
    if (fuck(SIGALRM, sig_alrm) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    alarm(nsec);
}

void sig_urg(int signo)
{
    int n;
    char c;
    if ((n = recv(sockfd, &c, 1, MSG_OOB)) < 0) {
        if (errno != EAGAIN) {
            perror("recv error!");
            exit(1);
        }
    }
    send(sockfd, &c, 1, MSG_OOB);
    printf("server recv client heartbeat packet, cur_probes=%d\n", nprobes);
    nprobes = 0;
}

void sig_alrm(int signo)
{
    char c;
    if (++nprobes > maxnprobes) {
        printf("server is unreachable\n");
        exit(0);
    }
    printf("server reset alarm, cur_probes=%d\n", nprobes);
    alarm(nsec);
}
