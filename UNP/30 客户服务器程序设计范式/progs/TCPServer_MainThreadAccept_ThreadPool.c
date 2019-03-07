#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include "AddrTool.h"
#include <fcntl.h>

static int iget;
static int iput;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

#define MAXCLI (100)
#define MAXN (2 << 20)

int client[MAXCLI];
static int naccepted;
char response[MAXN];

pthread_t *tid;

ssize_t writen(int fd, char *buf, ssize_t n)
{
    ssize_t nleft = n, nwrite;
    char *ptr = buf;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno != EINTR && errno != EAGAIN)
                break;
            nwrite = 0;
        } 
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n - nleft;
}

void handle_req(int connfd)
{
    ssize_t nread;
    ssize_t nbytes;
    char buf[100];
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) <= 0) {
            if (errno != 0)
                perror("read error!");
            break;
        } 
        buf[nread] = 0;
        nbytes = atoi(buf);
        if (writen(connfd, response, nbytes) != nbytes) {
            perror("writen error!");
            break;
        }
    }
}

void *thread_main(void *arg)
{
    int connfd = -1;
    for (;;) {
        pthread_mutex_lock(&mtx);
        while (naccepted == 0) {
            printf("thread - %d wait in cond empty\n", (int *)arg);
            pthread_cond_wait(&empty, &mtx);
        }

        connfd = client[iget];
        client[iget] = -1;
        if (++iget == MAXCLI)
            iget = 0;
        naccepted--;

        printf("thread - %d wake up, got connfd=%d\n", (int *)arg, connfd);
        char *info = getPeerInfo(connfd);
        printf("thread - %d handle req from %s connfd=%d\n", (int *)arg, info, connfd);

        if (naccepted == MAXCLI - 1)
            pthread_cond_broadcast(&full);
        pthread_mutex_unlock(&mtx);

        handle_req(connfd);
        close(connfd);
        printf("thread - %d cut connection from %s\n", (int *)arg, info);
        free(info);
    } 
}

void make_thread(int i)
{
    pthread_create(tid + i, NULL, thread_main, (void *)i);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <nthread>\n", argv[0]);
        exit(1);
    }
    setbuf(stdout, NULL);
    int nthread = atoi(argv[2]);
    unsigned short port = atoi(argv[1]);

    if ((tid = calloc(nthread, sizeof(pthread_t))) == NULL) {
        printf("calloc error!");
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

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

    for (int i = 0; i < nthread; i++)
        make_thread(i);

    int flags = fcntl(listenfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(listenfd, F_SETFL, flags);

    int connfd = -1;
    for (;;) {
        pthread_mutex_lock(&mtx);
        while (naccepted == MAXCLI)
            pthread_cond_wait(&full, &mtx);
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno != EINTR && errno != EAGAIN)
                perror("accept error!");
            goto again;
        }
        printf("main thread got connfd=%d\n", connfd);
        naccepted++;
        client[iput] = connfd; 
        if (++iput == MAXCLI)
            iput = 0;
    again: 
        if (naccepted == 1) 
            pthread_cond_broadcast(&empty);
        pthread_mutex_unlock(&mtx);
    }
}
