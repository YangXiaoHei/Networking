#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "AddrTool.h"

pthread_t *tid;

#define MAXN (2 << 20)

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static char response[MAXN];
int listenfd = -1;

ssize_t writen(int fd, char *buf, ssize_t n)
{
    ssize_t nwrite, nleft = n;
    char *ptr = buf;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno != EINTR && errno != EAGAIN)
                break;
            nwrite = 0;
        }
        ptr += nwrite;
        nleft -= nwrite;
    }
    return n - nleft;
}

void handle_req(int connfd)
{
    ssize_t nwrite, nread;
    ssize_t nbytes;
    char buf[100];
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) <= 0) {
            if (errno != 0)
                perror("read error!");
            break;
        } else {
            buf[nread] = 0;
            nbytes = atoi(buf);
            if (writen(connfd, response, nbytes) != nbytes) {
                perror("writen error!");
                break;
            }
        }
    }
}

void *thread_main(void *arg)
{
    int i = (int *)arg;
    int connfd = -1;
    for (;;) {
        pthread_mutex_lock(&mtx);
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno != EINTR)
                perror("accept error!");
            pthread_mutex_unlock(&mtx);
            continue;
        }
        pthread_mutex_unlock(&mtx);
        char *info = getPeerInfo(connfd);
        printf("thread - %d tcp connection from %s\n",i, info);
        handle_req(connfd);
        close(connfd);
        printf("thread - %d cut connection from %s\n",i, info);
        free(info);
    }
}

void thread_make(int i)
{
    pthread_create(tid + i, NULL, thread_main, (void *)i);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <nthread>\n", argv[0]);
        exit(1);
    }
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    unsigned short port = atoi(argv[1]);
    int nthread = atoi(argv[2]);

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_port = htons(port);
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100000) < 0) {
        perror("listen error!");
        exit(1);
    }
    
    if ((tid = calloc(nthread, sizeof(pthread_t))) == NULL) {
        printf("calloc error!");
        exit(1);
    }

    for (int i = 0; i < nthread; i++)
        thread_make(i);

    for (;;)
        pause();
}
