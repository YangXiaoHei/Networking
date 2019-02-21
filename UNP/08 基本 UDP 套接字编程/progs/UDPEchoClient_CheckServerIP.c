#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, replyaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    svraddr.sin_port = htons(atoi(argv[2]));

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    socklen_t replaylen = sizeof(replyaddr);
    struct sockaddr_in *preply = malloc(sizeof(svraddr));

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if ((nwrite = sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&svraddr, sizeof(svraddr))) < 0) {
            perror("sendto error!");
            exit(1);
        }
        socklen_t len = replaylen;
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)preply, &len)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        /* 校验响应的 IP 和 port */
        int a1 = 0, a2 = 0;
        if (len != sizeof(svraddr) || 
            svraddr.sin_addr.s_addr != preply->sin_addr.s_addr || 
            svraddr.sin_port != preply->sin_port) {
            
            char *addrInfo = getAddrInfo(preply);
            char *svraddrInfo = getAddrInfo(&svraddr);
            printf("a1 = %d, a2 = %d, reply %zd bytes from %s %s (ignored)\n", a1, a2, nread, addrInfo, svraddrInfo);
            free(addrInfo);
            free(svraddrInfo);
            continue;
        }

        char *addrInfo = getAddrInfo(&svraddr);
        printf("recv %zd bytes from %s\n", nread, addrInfo);
        free(addrInfo);

        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        }
    }

    return 0;
}
