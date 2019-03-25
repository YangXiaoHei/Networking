#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../tool/CommonTool.h"
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int cb(int fd)
{
    int on = 1;
    int interval = 60;
    int idle = 5;
    int count = 3;

    if (setsockopt(fd, IPPROTO_TCP, SO_KEEPALIVE, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        return -1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0) {
        perror("setsockopt error!");
        return -2;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
        perror("setsockopt error!");
        return -3;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0) {
        perror("setsockopt error!");
        return -4;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], cb)) < 0) {
        perror("tcp_connect error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nwrite, nread;
    while ((fgets(buf, sizeof(buf), stdin)) != NULL) {
        if (write(fd, buf, strlen(buf)) != strlen(buf)) {
            perror("write error!");
            exit(1);
        }
    }
}
