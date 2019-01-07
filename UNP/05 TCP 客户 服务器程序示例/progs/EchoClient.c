#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "ReadWriteTool.h"

void clientSendTextNeedEcho(FILE *fp, int fd) 
{   
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        size_t len = strlen(buf);
        if (writen(fd, buf, len) != len) {
            perror("writen error!");
            exit(1);
        }
        if (readline(fd, buf, sizeof(buf)) <= 0) {
            perror("readline error!");
            exit(1);
        } 
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        } 
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM ,0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &svraddr.sin_addr) < 0) {
        perror("inet_pton error!");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }
    socklen_t clilen = sizeof(cliaddr);
    const char *svrip = argv[1];
    char cliip[128];
    if (getsockname(fd, (struct sockaddr *)&cliaddr, &clilen) < 0) {
        perror("getsockname error!");
        exit(1);
    }
    if (inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)) < 0) {
        perror("inet_ntop error!");
        exit(1);
    }

    printf("TCP connection established [%s:%d] <---> [%s:%d]\n", 
        cliip, 
        ntohs(cliaddr.sin_port), 
        svrip, 
        ntohs(svraddr.sin_port));

    clientSendTextNeedEcho(stdin, fd);

    return 0;
}