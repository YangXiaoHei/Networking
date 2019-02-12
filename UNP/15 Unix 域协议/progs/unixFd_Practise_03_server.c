#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <sys/un.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    int listenfd = -1;
    if ((listenfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    int on = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    const char *pathname = "unix_hanson";
    unlink(pathname);

    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    strcpy(svraddr.sun_path, pathname);
    svraddr.sun_family = AF_LOCAL;

    if (bind(listenfd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    if (listen(listenfd, 100000) < 0) {
        perror("listen error!");
        exit(1);
    }

    int connfd = -1;
    char buf[1024];
    ssize_t nwrite = 0;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        socklen_t len = clilen;
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            continue;
        }

        char *srcInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        printf("TCP connection from %s\n", srcInfo);
        free(srcInfo);

        time_t ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
        for (int i = 0; i < strlen(buf); i++)
            if (write(connfd, &buf[i], 1) != 1) {
                perror("write error!");
                close(connfd);
                continue;
            }
        close(connfd);
    }

    return 0;
}