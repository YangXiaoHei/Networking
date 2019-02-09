#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

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

void connect_terminate(int signo) { return; }

int connect_timeout(int fd, struct sockaddr *addr, socklen_t addrlen, int sec)
{
    sig_handler oldhandler = fuck(SIGALRM, connect_terminate);
    int ori = 0;
    if ((ori = alarm(sec)) != 0) {
        printf("alarm already set! ori = %d\n", ori);
    }
    int retcode = 0;
    sleep(2);
    if ((retcode = connect(fd, addr, addrlen)) < 0) {
        close(fd);
        if (errno == EINTR) 
            errno = ETIMEDOUT;
    }
    fuck(SIGALRM, oldhandler);
    int conn_left = alarm(0);
    int conn_use = sec - conn_left;
    printf("conn_left = %d conn_use = %d new_set = %d\n", conn_left, conn_use, ori - conn_use);
    alarm(ori - conn_use);
    return retcode;
}

void ori_set(int signo)
{
    printf("original set\n");
}

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        printf("usage : %s <ip> <port> <timeout>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    fuck(SIGALRM, ori_set);
    alarm(10);

    if (connect_timeout(fd, (struct sockaddr *)&svraddr, sizeof(svraddr), atoi(argv[3])) < 0) {
        perror("connect_timeout error!");
        exit(1);
    }

    printf("connect succ!\n");

    while (1);

    return 0;
}