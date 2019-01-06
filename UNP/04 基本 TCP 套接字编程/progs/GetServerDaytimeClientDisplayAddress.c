#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{

    const char *ip = "119.29.207.157";
    unsigned short port = 20013;

    if (argc == 1) {
        /* nothing to do */
    } else if (argc == 2) {
        port = atoi(argv[1]);
    } else if (argc == 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    } else {
        printf("usage : ./%s [ip] [port]\n", argv[0]);
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
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &svraddr.sin_addr) < 0) {
        perror("inet_pton error!");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    bzero(&cliaddr, sizeof(cliaddr));
    if (getsockname(fd, (struct sockaddr *)&cliaddr, &len) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    char svrip[128];
    char cliip[128];
    printf("establish TCP connection 【%s:%d】 <-> 【%s:%d】\n", 
        inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)), ntohs(cliaddr.sin_port),
        inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip)), ntohs(svraddr.sin_port));

    ssize_t nread = -1;
    char buf[1024];
    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        }
    }
    if (nread < 0) {
        perror("read error!");
        exit(1);
    }

    return 0;
}