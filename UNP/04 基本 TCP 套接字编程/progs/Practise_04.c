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

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    /*
    if (liste(fd, 100) < 0) {
        perror("listen error1");
        exit(1);
    }
    */

    char cliip[128];
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);
    while (1) {
        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error!");
            exit(1);
        }

        printf("connection from [%s:%d]\n",
            inet_ntop(AF_INET, &cliaddr.sin_addr, cliip, sizeof(cliip)),
            ntohs(cliaddr.sin_port));

        const char *msg = "hello world\n";
        if (write(fd, msg, strlen(msg)) != strlen(msg)) {
            perror("write error!");
            exit(1);
        }
        close(connfd);
    }
    return 0;
}