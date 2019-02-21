#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    svraddr.sin_port = htons(atoi(argv[2]));

    /* 对于 UDP 套接字的 connect 只是个本地操作，不会发送任何分组，
       调用 connect 有性能上的好处，同时异步错误会被提交到应用态，而且本地 IP 地址得以确定 */
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t len = sizeof(localaddr);
    if (getsockname(fd, (struct sockaddr *)&localaddr, &len) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    char *addrInfo = getAddrInfo(&localaddr);
    printf("local addr = %s\n", addrInfo);
    free(addrInfo);

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if ((nwrite = write(fd, buf, strlen(buf))) != strlen(buf)) {
            perror("write error!");
            exit(1);
        }
        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        }
        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        }
    }
    return 0;
}