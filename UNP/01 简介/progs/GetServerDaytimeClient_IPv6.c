#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define VEASON_IP ""
#define VEASON_PORT (20013)
#define MAXLINE 2048

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    } 

    struct sockaddr_in6 svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin6_family = AF_INET6;
    svraddr.sin6_port = htons(VEASON_PORT);
    if (inet_pton(AF_INET6, VEASON_IP, &svraddr.sin6_addr) < 0) {
        perror("ipv6 format error!");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    char buf[MAXLINE];
    ssize_t nread = 0;
    while ((nread = read(fd, buf, MAXLINE)) > 0) {
        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF) {
            fprintf(stderr, "fputs error!");
            exit(1);
        }
    }
    if (nread < 0) {
        perror("read error!");
        exit(1);
    }

    close(fd);

    return 0;
}