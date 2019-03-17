#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "../../tool/CommonTool.h"
#include "../../tool/TimeTool.h"

extern int log_err;

int cb(int fd)
{
    int sndbuf = 8192;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0)
        return -1;
    /*
     * int mss = 1024;
     * if (setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(mss)) < 0)
     *     return -1;
     */
    /*
     * int win = 4096;
     * if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &win, sizeof(win)) < 0)
     *     return -1;
     */
    return 0;
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        logx("usage : %s <hostname/ip> <service/port>", argv[0]);
        exit(1);
    }

    log_err = 1;
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], cb)) < 0) {
        logx("tcp_connect_cb error!");
        exit(1);
    }

    char buf[8192];
    for (int i = 0; i < 1; i++)
        if (write(fd, buf, sizeof(buf)) != sizeof(buf)) {
            logx("write error!");
            exit(1);
        }
    logx("client exit");
}
