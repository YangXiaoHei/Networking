#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include "../../tool/ReadWriteTool.h"

void sendMessageToSvr(FILE *fp, int fd) 
{
    char buf[1024];
    ssize_t nread = -1;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (writen(fd, buf, strlen(buf)) < 0) {
            perror("writen error!");
            break;
        }
        if ((nread = readline(fd, buf, sizeof(buf))) <= 0) {
            perror("readline error!");
            break;
        } 
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int fd[5];
    memset(fd, -1, sizeof(int) * 5);

    const char *svrip = argv[1];
    unsigned short port = atoi(argv[2]);

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, svrip, &svraddr.sin_addr) < 0) {
        perror("inet_pton error!");
        exit(1);
    } 

    socklen_t clilen = sizeof(cliaddr);
    char cliip[128];
    for (int i = 0; i < 5; i++) {
        if ((fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket error!");
            exit(1);
        }

        if (connect(fd[i], (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            perror("connect error!");
            exit(1);
        }

        if (getsockname(fd[i], (struct sockaddr *)&cliaddr, &clilen) < 0) {
            perror("getsockname error!");
            exit(1);
        }

        printf("TCP connection established [%s:%d] <---> [%s:%d]\n", 
            inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)), ntohs(cliaddr.sin_port),
            svrip, port);
    }

    sendMessageToSvr(stdin, fd[0]);
    close(fd[0]);

    return 0;
}