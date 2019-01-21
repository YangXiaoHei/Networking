#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "TimeTool.h"
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
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
        perror("listen error");
        exit(1);
    }

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    char buf[1024];
    for (;;) {
        int connfd = accept(fd, (struct sockaddr *)&cliaddr, &clilen);

        unsigned long begin = getCurTimeUs();
        ssize_t nread = 0;
        while ((nread = read(connfd, buf, sizeof(buf))) > 0);
        if (nread < 0) {
            perror("read error!");
            exit(1);
        } else if (nread == 0) {
            printf("read EOF\n");
        }
        printf("read finsihed! %s total_cost = %ld\n", curTime(), getCurTimeUs() - begin);

        printf("close begin! %s\n", curTime());
        begin = getCurTimeUs();
        close(connfd);
        printf("close finished! %s total_cost = %ld\n\n\n\n\n\n\n", curTime(), getCurTimeUs() - begin);
    }
    
    return 0;
}