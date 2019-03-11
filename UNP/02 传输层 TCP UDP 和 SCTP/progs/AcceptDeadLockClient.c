#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "../../tool/CommonTool.h"

int cb(int fd)
{
    struct  linger l;
    l.l_onoff = 1;
    l.l_linger = 0;
    if ((setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l))) < 0) {
        perror("setsockopt error!");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], cb)) < 0) {
        perror("tcp_connect_cb error!");
        exit(1);
    }
    printf("connect to %s %s succ!\n", argv[1], argv[2]);
    close(fd);
}
