#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket create fd fail!");
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

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    socklen_t olen = sizeof(cliaddr);
    for (;;) {
        socklen_t len = olen;
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        char *peerInfo = getAddrInfo(&cliaddr);
        printf("recv %zd bytes from %s\n", nread, peerInfo);

        if ((nwrite = sendto(fd, buf, nread, 0, (struct sockaddr *)&cliaddr, len)) < 0) {
            perror("sendto error!");
            exit(1);
        }
        printf("send %zd bytes to %s\n", nwrite, peerInfo);
        free(peerInfo);
    }
    return 0;
}