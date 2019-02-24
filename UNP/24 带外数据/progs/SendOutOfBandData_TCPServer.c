#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

int connfd;

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

void sig_urg(int signo)
{
    int n;
    char buf[1024];
    
    if ((n = recv(connfd, buf, sizeof(buf), MSG_OOB)) < 0) {
        perror("recv error!");
        exit(1);
    }
    buf[n] = 0;
    printf("recv OOB data %zd bytes, %s\n", n, buf);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(0);
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
   
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
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
        perror("fuckt error!");
        exit(1);
    }
    ssize_t nread = 0;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    for (;;) {
        socklen_t len = clilen;
        if ((connfd = accept(fd, (struct sockaddr  *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            continue;
        }
        fcntl(connfd, F_SETOWN, getpid());
        for (;;) {
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                continue;
            } else if (nread == 0) {
                printf("client cut connection\n");
                break;
            }
            buf[nread] = 0;
            printf("read %zd bytes, %s\n", nread, buf);
        }
    }
}
