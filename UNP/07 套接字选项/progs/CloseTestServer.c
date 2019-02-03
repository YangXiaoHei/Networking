#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "TimeTool.h"
#include <netinet/in.h>
#include "AddrTool.h"
int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

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
        ssize_t totalRead = 0;
        while ((nread = read(connfd, buf, sizeof(buf))) > 0)
            totalRead += nread;

        if (nread < 0) {
            perror("read error!");
            char *peerInfo = getPeerInfo(connfd);
            printf("cut client connection %s\n", peerInfo);
            free(peerInfo);
            close(connfd);

        } else if (nread == 0) {
            printf("read EOF\n");
            char *peerInfo = getPeerInfo(connfd);
            printf("cut client connection %s\n", peerInfo);
            free(peerInfo);
            close(connfd);

        }
        printf("read finsihed! %s [total_read=%ld] [total_cost=%ld]\n", curTime(), totalRead, getCurTimeUs() - begin);

        printf("close begin! %s\n", curTime());
        begin = getCurTimeUs();
        close(connfd);
        printf("close finished! %s total_cost = %ld\n\n\n\n\n\n\n", curTime(), getCurTimeUs() - begin);
    }
    
    return 0;
}
