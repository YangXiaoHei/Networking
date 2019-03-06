#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include "../../tool/AddrTool.h"

#define MAXN (2 << 20)

#define CHECK(num) \
do {    \
    __typeof(num) __num = (num); \
    if ((__num) <= 0) { \
        printf("invalid" #num "%d\n", __num); \
        exit(1); \
    } \
} while (0) \

int tcp_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            fd = -1;
            continue;
        }
        char *info = NULL;
        printf("client connect to %s succ!\n", (info = getSockInfo(fd)));
        free(info);
        break;
    } while ((res = res->ai_next) != NULL);
    freeaddrinfo(res);
    if (res == NULL) {
        printf("connect to %s %s fail!\n", hostname, service);
        exit(1);
    }
    return fd;
}

ssize_t readn(int fd, char *buf, ssize_t nbytes) 
{
    ssize_t nread = 0, nleft = nbytes;
    char *ptr = buf;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR || errno == EAGAIN)
                nread = 0;
            break;
        } else if (nread == 0)
           break;
       nleft -= nread;
       ptr += nread; 
    }
    return nbytes - nleft;
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printf("usage : %s <hostname/ip> <service/port> <child_num> <conn_num_per_child> <req_data_per_conn>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *service = argv[2];

    int child_num = atoi(argv[3]);
    CHECK(child_num);

    int conn_num_per_child = atoi(argv[4]);
    CHECK(conn_num_per_child);
    
    int req_bytes = atoi(argv[5]);
    CHECK(req_bytes);
    if (req_bytes > MAXN) {
        perror("req_bytes beyond max bytes limit\n");
        exit(1);
    }

    pid_t pid; 
    for (int i = 0; i < child_num; i++) {
        if ((pid = fork()) < 0) {
            perror("fork error");
            break;
        } else if (pid == 0) {
            char buf[MAXN]; 
            ssize_t nread;
            for (int j = 0; j < conn_num_per_child; j++) {
                int fd = -1;
                if ((fd = tcp_connect(hostname, service)) < 0) {
                    perror("tcp_connect error!");
                    exit(1);
                }
                snprintf(buf, MAXN, "%d\n", req_bytes);
                if (write(fd, buf, strlen(buf)) != strlen(buf)) {
                    perror("write error!");
                    exit(1);
                }
                printf("client %d send %zd bytes\n",getpid(), strlen(buf));
                if ((nread = readn(fd, buf, req_bytes)) != req_bytes) {
                    perror("readn error!");
                    exit(1);
                }
                printf("client %d recv %zd bytes\n",getpid(), nread);
                close(fd);
            }
            printf("child %d exit\n", getpid());
        }
    }

    while (wait(NULL) > 0)
        ;
    if (errno != 0 && errno != ECHILD) {
        perror("wait perror!");
        exit(1);
    }
}
