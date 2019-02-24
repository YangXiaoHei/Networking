#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

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

int connfd = -1;

void sig_urg(int signo)
{
    int nread = 0;
    char buf[100];
    if ((nread = recv(connfd, buf, sizeof(buf), MSG_OOB)) < 0) {
        perror("recv error!");
        exit(1);
    }
    buf[nread] = 0;
    printf("recv %zd bytes OOB data, %s\n", nread, buf);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <recvbuff kb>\n", argv[0]);
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
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    ssize_t rcvbuf = atoi(argv[2]) << 10;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }
    
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    } 
    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    if (fuck(SIGURG, sig_urg) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    
    if ((connfd = accept(fd, NULL, NULL)) < 0) {
        perror("accept error!");
        exit(1);
    }
    fcntl(connfd, F_SETOWN, getpid());
    
    for (;;) {
        sleep(1);
    }
}
