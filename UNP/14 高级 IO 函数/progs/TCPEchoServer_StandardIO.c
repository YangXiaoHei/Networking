#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "../../tool/AddrTool.h"

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

void collect_child(int signo) 
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("collect child %ld\n", (long)pid);
    if (pid < 0 && errno != ECHILD) {
        perror("waipid error!");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 10000) < 0) {
        perror("listen error!");
        exit(1);
    }

    fuck(SIGCHLD, collect_child);

    char buf[1024];
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            continue;
        }

        char *srcInfo = getAddrInfo(&cliaddr);
        char *dstInfo = getSockInfo(connfd);
        printf("TCP connection from %s to %s\n", srcInfo, dstInfo);
        free(srcInfo);
        free(dstInfo);

        pid_t pid = -1;
        if ((pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (pid == 0) {
            FILE *fpin = NULL, *fpout = NULL;
            if ((fpin = fdopen(connfd, "r")) == NULL) {
                perror("fdopen error!");
                exit(1);
            }
            if ((fpout = fdopen(connfd, "w")) == NULL) {
                perror("fdopen error!");
                exit(1);
            }
            while (fgets(buf, sizeof(buf), fpin) != NULL) {
                if (fputs(buf, fpout) == EOF) {
                    printf("fputs error!");
                    exit(1);
                }
            }
            exit(0);
        }
        close(connfd);
    }
    return 0;
}
