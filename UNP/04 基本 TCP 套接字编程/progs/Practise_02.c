#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
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

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    if (getsockname(fd, (struct sockaddr *)&cliaddr, &clilen) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    char svrip[128];
    char cliip[128];
    printf("establish TCP connection 【%s:%d】<-->【%s:%d】\n", 
        inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)), ntohs(cliaddr.sin_port),
        inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip)), ntohs(svraddr.sin_port));

    close(fd);

    return 0;
}