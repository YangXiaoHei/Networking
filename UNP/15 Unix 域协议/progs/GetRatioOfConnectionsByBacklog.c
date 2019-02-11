#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

struct sockaddr_in svraddr;
int fds[10000];
int sockfds[2];

#define pfd sockfds[0]
#define cfd sockfds[1]

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

void do_child()
{
    close(pfd);
    int listenfd = -1;
    int backlog = -1;
    int ready = 1;
    if (read(cfd, &backlog, sizeof(int)) < 0) {
        perror("read backlog error!");
        exit(1);
    }
    while (backlog >= 0) {
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket fd create fail!");
            exit(1);
        }
        int on = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt error!");
            exit(1);
        }
        if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            perror("bind error!");
            exit(1);
        }
        if (listen(listenfd, backlog) < 0) {
            perror("listen error!");
            exit(1);
        }
        if (write(cfd, &ready, sizeof(int)) < 0) {
            perror("write error!");
            exit(1);
        }
        if (read(cfd, &backlog, sizeof(int)) < 0) {
            perror("read backlog error!");
            exit(1);
        }
        close(listenfd);
    }
    exit(0);
}

void connect_terminate(int signo) { return; }

void do_parent()
{
    close(cfd);
    int backlog = 0;
    int ready = 1;

    if (fuck(SIGALRM, connect_terminate) == SIG_ERR) {
        perror("signal error!");
        exit(1);
    }

    for (; backlog < 14; backlog++) {
        printf("backlog = %d\n", backlog);
        write(pfd, &backlog, sizeof(int));
        read(pfd, &ready, sizeof(int));

        int i = 0;
        int fd[10000];
        bzero(fd, sizeof(fd));
        for (i = 0; i < 1000; i++) {
            if ((fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket fd create fail!");
                exit(1);
            }
            alarm(2);
            if (connect(fd[i], (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
                if (errno != EINTR) {
                    perror("connect error!");
                    exit(1);
                }
                printf("\ttimeout occur, %d connections compeleted\n", i - 1);
                for (int k = 0; k <= i; k++)
                    close(fd[k]);
                break;
            }
            alarm(0);
        }
        if (i == 1000) {
            printf("what a fuck?\n");
            exit(1);
        }
    }
    backlog = -1;
    write(pfd, &backlog, sizeof(int));
    exit(0);
}

int main(int argc, char const *argv[])
{
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(50001);
    svraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfds);

    setbuf(stdout, NULL);

    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        do_child();
    } else {
        do_parent();
    }

    return 0;
}