#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "../../tool/AddrTool.h"
#include "../../tool/CommonTool.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_server_cb(argv[1], NULL)) < 0) {
        perror("tcp_server_cb error!");
        exit(1);
    }

    int connfd;
    char buf[1024];
    for (;;) {
        sleep(5);
        printf("wake up, and begin to accept\n");
        if ((connfd = accept(fd, NULL, NULL)) < 0) {
            if (errno != EINTR)
                perror("accept error!");
            continue;
        }
        printf("accept succ\n");
        char *srcInfo = getPeerInfo(fd);
        printf("tcp connection from %s\n", srcInfo);
        free(srcInfo);

        close(connfd);
    }
}
