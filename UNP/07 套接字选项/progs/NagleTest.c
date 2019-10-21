#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../tool/TimeTool.h"
#include "../../tool/CommonTool.h"
#include <string.h>
#include <unistd.h>
#include <netinet/tcp.h>

int on;

int cb(int fd)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0) {
        logx("setsockopt error!");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        logx("usage : %s <ip/hostname> <service/port> <enable_nagle>", argv[0]);
        exit(1);
    }

    on = atoi(argv[3]) == 1 ? 0 : 1;

    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        logx("tcp_connect error!");
        exit(1);
    }

    char msg[128];
    memset(msg, 'a', sizeof(msg));
    msg[127] = 0;
    for (;;) {
        for (int i = 0; i < strlen(msg); i++) {
            write(fd, msg + i, 1);
            logx("send %c to server", msg[i]);
        }
    }
    close(fd);
}
