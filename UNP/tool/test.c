#include <stdio.h>
#include <stdlib.h>
#include "CommonTool.h"

int cb(int fd)
{
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int fd = -1;
    fd = tcp_connect(argv[1], argv[2]);
    fd = tcp_server(argv[2]);
    fd = udp_connect(argv[1], argv[2]);
    fd = udp_server(argv[2]);

    fd = tcp_connect_cb(argv[1], argv[2], cb);
    fd = udp_connect_cb(argv[1], argv[2], cb);

    fd = udp_server_cb(argv[2], cb);
    fd = tcp_server_cb(argv[2], cb);

    return 0;
}
