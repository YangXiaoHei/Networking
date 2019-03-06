#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "TimeTool.h"
#include <errno.h>

#define MAXN (3 << 20)

static char response[MAXN];

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

void sig_int(int signo)
{
    cpu_time();
}

void sig_child(int signo)
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("collect %d\n", pid);
    if (errno != 0 && errno != ECHILD) {
        perror("waitpid error!");
        exit(1);
    }
}

ssize_t writen(int fd, char *buf, ssize_t n)
{
    char *ptr = buf;
    ssize_t nleft = n;
    ssize_t nwrite, nfinish;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno != EINTR && errno != EAGAIN)
                nwrite = 0;
            break;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n - nleft;
}

void handle_req(int connfd)
{
    char buf[1024];
    ssize_t nwrite, nread;
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            break;
        } else if (nread == 0)
           break; 
        buf[nread] = 0;
        int nbytes = atoi(buf);
        if ((writen(connfd, response, nbytes)) != nbytes) {
            perror("writen error!");
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    } 
    
    if (fuck(SIGINT, sig_int) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    if (fuck(SIGCHLD, sig_child) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 10000) < 0) {
        perror("listen error!");
        exit(1);
    }

    pid_t pid;
    int connfd = -1;
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno != EINTR)
                perror("accept error");
            continue;
        }
        if ((pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (pid == 0) {
            handle_req(connfd);
            exit(0);
        }
        close(connfd);
    }
}
