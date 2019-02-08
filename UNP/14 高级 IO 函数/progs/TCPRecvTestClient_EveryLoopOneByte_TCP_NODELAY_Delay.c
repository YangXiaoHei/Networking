#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    int on = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    char *dstInfo = getAddrInfo(&svraddr);
    printf("connect to %s succ!\n", dstInfo);
    free(dstInfo);

    printf("sleep 5 seconds\n");
    sleep(5);

    char *msg = "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < strlen(msg); i++) {
        printf("send one byte %c to svr\n", msg[i]);
        if (write(fd, &msg[i], 1) != 1) {
            perror("write error!");
            exit(1);
        }
        usleep(1000 * 500); // 500 ms = 0.5 s
    }
    
    return 0;
}