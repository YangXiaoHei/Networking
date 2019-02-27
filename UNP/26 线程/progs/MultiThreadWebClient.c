#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include "../../tool/TimeTool.h"

#define GET_CMD "GET %s HTTP/1.1\r\n\r\n"
#define F_READING 1
#define F_DONE 2
#define F_NONE 3

struct Session {
    int sockfd;
    int filefd;
    int flags;
    char *file;
    pthread_t tid;
    int tindex;
};

static int ndone;
static char *hostname;
static char *service;
static struct Session *ses;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int tcp_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    
    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    int fd = -1;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect error!");
        exit(1);
    }
    return fd;
}

int send_get_req(struct Session *session)
{
    char buf[200];
    snprintf(buf, sizeof(buf), GET_CMD, session->file);
    if (write(session->sockfd, buf, strlen(buf)) != strlen(buf)) {
        perror("write error!");
        exit(1);
    }
    return 0;
}

void *download(void *arg)
{
    struct Session *session = (struct Session *)arg;
    if ((session->sockfd = tcp_connect(hostname, service)) < 0) {
        perror("tcp_connect error!");
        exit(1);
    }
    if (send_get_req(session) != 0) {
        printf("send_get_req error!");
        exit(1);
    }
    if ((session->filefd = open(session->file, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
        perror("open error!");
        exit(1);
    }
    session->flags = F_READING;
    ssize_t nread, nwrite;
    char buf[8192];
    while ((nread = read(session->sockfd, buf, sizeof(buf))) > 0) {
        if ((nwrite = write(session->filefd, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread == 0) {
        printf("%s download succ!\n", session->file);
          
        pthread_mutex_lock(&mtx);
        ndone++;
        session->flags = F_DONE;
        pthread_mutex_unlock(&mtx);        
        
        close(session->sockfd);
    } else {
        perror("read error!");
        exit(1);
    }
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("usage : %s <hostname> <service> <max_conn> [file1] [file2]...\n", argv[0]);
        exit(1);
    }
    hostname = argv[1];
    service = argv[2];
    
    int maxconn = atoi(argv[3]);
    int nfile = argc - 4;
    if ((ses = malloc(nfile * sizeof(struct Session))) == NULL) {
        printf("malloc error!");
        exit(1);
    }
    for (int i = 0; i < nfile; i++) {
        ses[i].sockfd = -1;
        ses[i].filefd = -1;
        ses[i].file = argv[i + 4];
        ses[i].flags = 0; 
        ses[i].tindex = i;
        if (access(ses[i].file, F_OK) == 0) 
            remove(ses[i].file);
    }
    
    int nleft = nfile;
    int ntoconn = nleft;
    int nconn = 0;
    int i = 0;
    pthread_t *tid;
    if ((tid = malloc(nfile * sizeof(pthread_t))) == NULL) {
        printf("malloc error!");
        exit(1);
    }
    int n = 0;
    long beg = curtimeus();
    while (nleft > 0) {
        while (nconn < maxconn && ntoconn > 0) {
            pthread_mutex_lock(&mtx);
            int j = 0;
            for (j = 0; j < nfile; j++)
                if (ses[j].flags == 0) {
                    ses[j].flags = F_READING;
                    break;
                }
            pthread_mutex_unlock(&mtx);
            
            if ((n = pthread_create(&ses[j].tid, NULL, download, &ses[j])) != 0) {
                printf("pthread_create fail! %s\n", strerror(n));
                exit(1); 
            }
            nconn++;
            ntoconn--;
        } 
        pthread_mutex_lock(&mtx);
        if (ndone > 0) {
            for (int i = 0; i < nfile; i++) {
                if (ses[i].flags == F_DONE) {
                    if (pthread_join(ses[i].tid, NULL) != 0) {
                        printf("pthread_join error!\n");
                        exit(1);
                    }
                    nleft--;
                    nconn--;
                    ndone--;
                    ses[i].flags = F_NONE;
                }
            }
        }
        pthread_mutex_unlock(&mtx);
    }
    printf("all download finished! total_cost = %.2f ms\n", (curtimeus() - beg) / 1000.0);
}
