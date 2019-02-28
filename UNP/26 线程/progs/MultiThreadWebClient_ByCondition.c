#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../../tool/TimeTool.h"

#define GET_CMD "GET %s HTTP/1.1\r\n\r\n"

#define F_READING 1
#define F_DONE    2
#define F_JOINED  3

struct Session {
    int sockfd;
    int filefd;
    int tidx;
    pthread_t tid;
    char *file;
    int flags; 
};

static struct Session *ses;

pthread_cond_t ndone_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static char *hostname;
static char *service; 

static int ndone;

int tcp_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    int fd = -1;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket error!");
        exit(1);
    }
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect to error!");
        exit(1);
    }
    printf("connect to %s %s succ!\n", hostname, service);
    freeaddrinfo(res);
    return fd;
}

void send_get_req(struct Session *s)
{
    char buf[256];
    snprintf(buf, sizeof(buf), GET_CMD, s->file);
    if (write(s->sockfd, buf, strlen(buf)) != strlen(buf)) {
        printf("send get cmd error for %s : %s\n", s->file, strerror(errno));
        exit(1);
    }
}

void *download(void *arg)
{
    struct Session *s = (struct Session *)arg;
    s->sockfd = tcp_connect(hostname, service);
    s->filefd = open(s->file, O_CREAT | O_APPEND | O_RDWR, 0644);
    if (s->filefd < 0) {
        printf("open %s error! : %s\n", s->file, strerror(errno));
        exit(1);
    }
    send_get_req(s);
    char buf[4096];
    ssize_t nwrite, nread;
    while ((nread = read(s->sockfd, buf, sizeof(buf))) > 0) {
        /* printf("read %zd bytes for %s\n", nread, s->file); */
        if ((nwrite = write(s->filefd, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread == 0) {
        printf("file %s download finished!\n", s->file);
        
        pthread_mutex_lock(&mtx);
        ndone++;
        s->flags = F_DONE;
        pthread_cond_signal(&ndone_cond);
        pthread_mutex_unlock(&mtx);
    } else {
        printf("file %s read fail! : %s\n", s->file, strerror(errno));
        exit(1);
    }
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        printf("usage : %s <hostname> <service> <maxconn> [file1] [file2] ...\n", argv[0]);
        exit(1);
    }
    int nfile = argc - 4;
    int maxconn = atoi(argv[3]);
    hostname = argv[1];
    service = argv[2];
    
    if ((ses = calloc(nfile, sizeof(struct Session))) == NULL) {
        printf("calloc error!");
        exit(1);
    }
    for (int i = 0; i < nfile; i++) {
        ses[i].sockfd = -1;
        ses[i].filefd = -1;
        ses[i].file = argv[i + 4];
        ses[i].tidx = -1;
        if (access(ses[i].file, F_OK) == 0) 
            remove(ses[i].file);
    }

    int nconn = 0;
    int nleft = nfile;
    int nlefttoconn = nfile;

    long beg = curtimeus();
    while (nleft > 0) {
        while (nconn < maxconn && nlefttoconn > 0) {
            pthread_mutex_lock(&mtx);
            int i = 0;
            for (i = 0; i < nfile; i++)
                if (ses[i].flags == 0)
                    break;
            if (i == nfile) {
                printf("all file flags not zero!\n");
                exit(1);
            } 
            ses[i].flags = F_READING;
            pthread_mutex_unlock(&mtx);

            nconn++;
            nlefttoconn--;
            pthread_create(&ses[i].tid, NULL, download, &ses[i]);
        }
        pthread_mutex_lock(&mtx);
        while (ndone == 0)
            pthread_cond_wait(&ndone_cond, &mtx);

        for (int i = 0; i < nfile; i++) {
            if (ses[i].flags == F_DONE) {
                pthread_join(ses[i].tid, NULL);
                ses[i].flags = F_JOINED;
                nleft--;
                nconn--;
            }
        }
        pthread_mutex_unlock(&mtx);
    }
    printf("all download finished! total_cost = %.2f ms\n", (curtimeus() - beg) / 1000.0);
}
