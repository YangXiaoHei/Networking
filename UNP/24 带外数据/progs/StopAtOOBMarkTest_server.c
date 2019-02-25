#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

int fuck(int fd)
{
    int flag = 0;
    if (ioctl(fd, SIOCATMARK, &flag) < 0) {
        return -1;
    }
    return flag != 0;
}

#ifndef FUCK
#define fuck(fd) sockatmark(fd)
#endif

int main(int argc, char *argv[])
{
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
    
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }    
    
    on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    int connfd = -1;
    if ((connfd = accept(fd, NULL, NULL)) < 0) {
        perror("accept error!");
        exit(1);
    } 
    sleep(5);
    printf("server wake up\n");

    char buf[1024];
    ssize_t nread = 0;
    for (;;) {
        if (fuck(connfd)) {
            printf("at OOB mark\n");
        }
        if ((nread = recv(connfd, buf, sizeof(buf), MSG_PEEK)) < 0) {
            perror("recv error!");
            exit(1);
        } else if (nread == 0) {
            printf("client cut connection\n");  
            exit(0);
        }
        printf("there are [%zd] bytes in recv buffer\n", nread);
        
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        } else if (nread == 0) {
            printf("client cut connection\n");
            exit(0);
        }
        buf[nread] = 0;
        printf("read %zd bytes from recv buffer, %s\n", nread, buf);
    }
}
