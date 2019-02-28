#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "ReadWriteTool_safe.h"
#include "AddrTool.h"
#include <pthread.h>
#include <signal.h>

static ssize_t total_send;

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

int tcp_server(unsigned short port)
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        return -1;
    
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
        return -2;

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) 
        return -3;

    if (listen(fd, 10000) < 0)
        return -4;

    return fd;    
}

void sig_pipe(int signo)
{
    printf("recv sigpipe signal!\n");
    exit(1);
}

void *echoTextToClient(void *arg)
{
    pthread_detach(pthread_self());
    int connfd = (int)arg;
    char *srcInfo = getPeerInfo(connfd);
    char buf[1024];
    ssize_t nwrite, nread;
    for (;;) {
        if ((nread = readline(connfd, buf, sizeof(buf))) < 0) {
            perror("readline error!");
            break;
        } else if (nread == 0) {
            printf("client %s cut connection\n", srcInfo);
            close(connfd);
            break;
        }
        buf[nread] = 0;
        printf("recv %zd bytes from client %s\n", nread, srcInfo);
        if ((nwrite = writen(connfd, buf, nread)) != nread) {
            perror("writen error!");
            break;
        }
        total_send += nwrite;
        printf("send %zd bytes to client %s, total_send=%zd\n", nwrite, srcInfo, total_send);
    }
    free(srcInfo);
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    setbuf(stdout, NULL);
    pthread_t tid;
    int fd = -1;
    if ((fd = tcp_server(atoi(argv[1]))) < 0) {
        perror("tcp_server error!");
        exit(1);
    }
    
    if (fuck(SIGPIPE, sig_pipe) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }    

    int connfd = -1;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int error = 0;
    for (;;) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            continue;
        }
        char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        printf("tcp connection from %s\n", srcInfo);
        free(srcInfo);

        if ((error = pthread_create(&tid, NULL, echoTextToClient, (void *)connfd)) != 0) {
            printf("pthread_create %s\n", strerror(error));
            exit(1);
        }
    }
}
