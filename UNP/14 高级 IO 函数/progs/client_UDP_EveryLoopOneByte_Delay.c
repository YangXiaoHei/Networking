#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <time.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    printf("sleep 5 seconds\n");
    sleep(5);

    char *msg = "abcdefghijklmnopqrstuvwxyz";
    int len = strlen(msg);
    char *p = msg;

    srand((unsigned int)time(NULL));

    int rlen = 0;
    while (len > 0) {
        rlen = rand() % len + 1; /* 1 ~ len */
        printf("send %d byte to svr, left %d bytes\n", rlen, len - rlen);
        if (write(fd, p, rlen) != rlen) {
            perror("write error!");
            exit(1);
        }
        p += rlen;
        len -= rlen;
        usleep(1000 * 500); // 500 ms = 0.5 s
    }
    return 0;
}
