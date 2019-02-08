#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
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

    ssize_t nCanReadInRecvBuffer = 0;
    ssize_t nread = 0;
    char buf[1024];
    for (;;) {
        if (ioctl(fd, FIONREAD, &nCanReadInRecvBuffer) < 0) {
            perror("ioctl error!");
            exit(1);
        }
        sleep(1);
        printf("there are [ %zd ] bytes in recv buffer\n", nCanReadInRecvBuffer);

        if (nCanReadInRecvBuffer == 0)
            continue;

        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }
        buf[nread] = 0;
        printf("read from udp socket fd %zd bytes, content = %s\n", nread, buf);
    }
    return 0;
}