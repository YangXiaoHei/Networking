#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "CommonTool.h"
#include "AddrTool.h"
#include "ReadWriteTool.h"
#include "TimeTool.h"

int cb(int fd)
{
    int on = 1;
    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
        logx("setsockopt error! %s", strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        logx("usage : %s <port>", argv[0]);
        exit(1);
    }
    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], cb)) < 0) {
        logx("tcp_server_cb error! %s", strerror(errno));
        exit(1);
    }

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = -1;
    ssize_t nread, nwrite;
    char buf[8192];
    for (;;) {
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno != EINTR)
                logx("accept error! %s", strerror(errno));
            continue;
        }
        socklen_t len = clilen;
        if (getpeername(connfd, (struct sockaddr *)&cliaddr, &len) < 0) {
            logx("connnfd = %d invalid! : %s", connfd, strerror(errno));
            close(connfd);
            continue;
        }

        char *srcInfo = getPeerInfo(connfd);
        logx("tcp connection from %s", srcInfo);

        for (;;) {
            if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
                logx("read error! %s", strerror(errno));
                break;
            } else if (nread == 0) {
                logx("client %s close connection", srcInfo);
                break;
            }
            if (writen(connfd, buf, nread) != nread) {
                logx("writen error! %s", strerror(errno));
                break;
            }
        }
        logx("server close connection of client %s", srcInfo);
        free(srcInfo);
        close(connfd);
    }
}
