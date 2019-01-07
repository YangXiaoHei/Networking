#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ReadWriteTool.h"

void echoTextSentByClient(int connfd)
{
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    if (getpeername(connfd, (struct sockaddr *)&cliaddr, &clilen) < 0) {
        perror("getpeername error!");
        exit(1);
    }
    char cliip[128];
    inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip));

    char buf[1024];
    ssize_t nread = -1;
    while ((nread = readline(connfd, buf, sizeof(buf))) > 0) {
        if (writen(connfd, buf, strlen(buf)) < 0) {
            perror("writen error!");
            exit(1);
        }
    }
    if (nread == 0) {
        printf("client [%s:%d] cut the connection\n", cliip, ntohs(cliaddr.sin_port));
        close(connfd);
    } else if (nread < 0) {
        perror("readline error!");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    char cliip[128];
    char svrip[128];
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    pid_t child_pid = -1;
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip));
        inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip));
        printf("TCP connection established [%s:%d] <---> [%s:%d]\n", 
            cliip, ntohs(cliaddr.sin_port), 
            svrip, ntohs(svraddr.sin_port));

        if((child_pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (child_pid == 0) {
            close(listenfd);
            echoTextSentByClient(connfd);
            exit(0);
        }
        close(connfd);
    }

    return 0;
}