#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "AddrTool.h"
#include "TimeTool.h"

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    int on = 1;
    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
        perror("setsockopt error!");
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

    if (listen(fd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    int nready = 0;
    int maxfd = fd;
    int connfd = -1;
    socklen_t clilen = sizeof(cliaddr);

    for (;;) {
        socklen_t len = clilen;
        again:
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
                goto again;
            logx("select error! %s", strerror(errno));
            exit(1);
        }

        logx("busy server cannot handle next accept right away! sleep 5 sec to simulate");
        sleep(5);
        logx("busy server now is ready to handle next accept!");

        if ((connfd = accept(fd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            logx("accept error! %s", strerror(errno));
            continue;
        }

        char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        logx("TCP connection from %s", srcInfo);
        free(srcInfo);

        char c = 0;
        if (read(connfd, &c, 1) < 0) {
            logx("read error! %s", strerror(errno));
        }

        close(connfd);
    }

    return 0;
}