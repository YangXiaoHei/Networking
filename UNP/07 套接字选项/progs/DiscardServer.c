#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"

int main(int argc, char *argv[])
{

    if (argc != 2) {
        logx("usage : %s <port>", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], NULL)) < 0) {
        logx("tcp_server_cb error!");
        exit(1);
    }

    int connfd = -1;
    ssize_t nread = 0;
    char buf[1024];
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            logx("accept error!");
            break;
        }
        while ((nread = read(connfd, buf, sizeof(buf))) > 0) {
            logx("read %zd bytes, discard it", nread);
        }
        if (nread < 0) {
            logx("read error!");
        } else if (nread == 0) {
            logx("client closed connection\n");
        }
        close(connfd);
    }
}
