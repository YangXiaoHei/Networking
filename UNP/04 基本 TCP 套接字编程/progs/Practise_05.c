#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in cliaddr;
    struct sockaddr_in svraddr;

    /*
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    */

    if (listen(fd, 100) < 0) {
        perror("listen error1");
        exit(1);
    }

    char svrip[128];
    socklen_t svrlen = sizeof(svraddr);
    bzero(&svraddr, sizeof(svraddr));
    if (getsockname(fd, (struct sockaddr *)&svraddr, &svrlen) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    printf("server ip:port is [%s:%d]\n",
        inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip)), ntohs(svraddr.sin_port));

    char cliip[128];
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    while (1) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        bzero(&svraddr, sizeof(svraddr));
        if (getsockname(connfd, (struct sockaddr *)&svraddr, &svrlen) < 0) {
            perror("getsockname error!");
            exit(1);
        }

        printf("TCP connection [%s:%d] <--> [%s:%d]\n",
            inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)), ntohs(cliaddr.sin_port),
            inet_ntop(AF_INET, &svraddr.sin_addr, svrip, sizeof(svrip)), ntohs(svraddr.sin_port));

        const char *msg = "hello world\n";
        if (write(connfd, msg, strlen(msg)) != strlen(msg)) {
            perror("write error!");
            exit(1);
        }
        close(connfd);
    }
    return 0;
}