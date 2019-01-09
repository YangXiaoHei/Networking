#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "ReadWriteTool.h"
#include <sys/wait.h>
#include <signal.h>

typedef void(*_handler)(int);

_handler _signal(int signo, _handler handler) 
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

void collectZombie(int signo)
{
    int status;
    pid_t pid = -1;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

void echoTextSentByClient(int connfd) 
{
    struct sockaddr_in peeraddr, svraddr;
    socklen_t peerlen = sizeof(peeraddr), svrlen = sizeof(svrlen);

    // 获取客户端 ip/port
    if (getpeername(connfd, (struct sockaddr *)&peeraddr, &peerlen) < 0) {
        perror("getpeername error!");
        exit(1);
    }

    // 获取服务器 ip/port
    if (getsockname(connfd, (struct sockaddr *)&svraddr, &svrlen) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    char peerip[128];
    char svrip[128];
    if (inet_ntop(AF_INET, &peeraddr.sin_addr, peerip, sizeof(peerip)) < 0) {
        perror("inet_ntop error!");
        exit(1);
    }
    if (inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip)) < 0) {
        perror("inet_ntop error!");
        exit(1);
    }
    unsigned short peerport = ntohs(peeraddr.sin_port);
    unsigned short svrport = ntohs(svraddr.sin_port);

    printf("TCP connection established, handle by child %d  [%s:%d] <---> [%s:%d]\n", 
        getpid(), svrip, svrport, 
        peerip, peerport);

    char buf[1024];
    ssize_t nread = 0;
    while ((nread = readline(connfd, buf, sizeof(buf))) > 0) {
        if (writen(connfd, buf, nread) < 0) {
            perror("writen error!");
            exit(1);
        }
    }
    if (nread == 0) {
        printf("child %d cut connection [%s:%d]\n", getpid(), peerip, peerport);
    } else {
        perror("readline error!");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 100)) {
        perror("listen error!");
        exit(1);
    }

    if (_signal(SIGCHLD, collectZombie) == SIG_ERR) {
        perror("signal error!");
        exit(1);
    }

    int connfd = -1;
    char cliip[128];
    char svrip[128];
    socklen_t clilen = sizeof(cliaddr);
    while (1) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        pid_t child_pid = -1;
        if ((child_pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (child_pid == 0) {
            echoTextSentByClient(connfd);
            exit(0);
        }
        close(connfd);
    }
    return 0;
}