#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#ifdef PRI
#define fuck POLLPRI
#elif defined(BAND)
#define fuck POLLRDBAND
#else
#error "you must define macro either PRI or BAND"
#endif

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
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

    int connfd = -1;
    if ((connfd = accept(fd, NULL, NULL)) < 0) {
        perror("accept error!");
        exit(1);
    }
    struct pollfd fds[1];
    fds[0].fd = connfd;
    fds[0].events = POLLRDNORM | fuck;
    
    char buf[1024]; 
    for (;;) {
        int nready = 0;
        if ((nready = poll(fds, 1, -1)) < 0) {
            perror("poll error!");
            exit(1);
        }
        
        /*
         * 普通数据 
         */
        if (fds[0].revents & POLLRDNORM) {
            ssize_t nread = 0;
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                continue;
            } else if (nread == 0) {
                printf("client cut connection\n");
                close(connfd);
                break;
            } 
            buf[nread] = 0;
            printf("read %zd bytes, %s\n", nread, buf);
        }
        /*
         * 带外数据
         */
        if (fds[0].revents & fuck) {
            ssize_t nread = 0;
            if ((nread = recv(connfd, buf, sizeof(buf), MSG_OOB)) < 0) {
                if (errno != EAGAIN) {
                    perror("recv error!");
                    exit(1);
                }
                continue;
            } else if (nread == 0) {
                printf("client cut connection\n");
                close(connfd);
                break;
            }   
            buf[nread] = 0;
            printf("read %zd bytes OOB data, %s\n", nread, buf);
        }
    }
}
