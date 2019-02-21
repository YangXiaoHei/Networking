#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "../../tool/PathTool.h"

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
        printf("collect child %ld\n",(long)pid);
    if (pid < 0 && errno != ECHILD)
        perror("waitpid error!");
}

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    unlink(getAbsolutePath("unix_hanson"));

    struct sockaddr_un svraddr, cliaddr;
    strcpy(svraddr.sun_path, getAbsolutePath("unix_hanson"));
    svraddr.sun_family = AF_LOCAL;
    if (bind(fd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }

    fuck(SIGCHLD, collect_child);

    int connfd = -1;
    char buf[1024];
    socklen_t clilen = sizeof(cliaddr);
    ssize_t nread = 0, nwrite = 0;
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            continue;
        }

        printf("new client connected cliaddr.sun_path=%s\n", cliaddr.sun_path);

        pid_t pid = 0;
        if ((pid = fork()) < 0) {
            perror("fork error!");
            continue;
        } else if (pid == 0) {
            for (;;) {
                if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                    perror("read error!");
                    exit(1);
                } else if (nread == 0) {
                    printf("client close connection\n");
                    close(connfd);
                    break;
                } else {
                    if ((nwrite = write(connfd, buf, nread)) != nread) {
                        perror("write error!");
                        close(connfd);
                        break;
                    }
                }
            }
            exit(0);
        }
        close(connfd);
    }
    return 0;
}
