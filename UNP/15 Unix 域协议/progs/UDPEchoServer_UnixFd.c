#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include "PathTool.h"

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    const char *path = getAbsolutePath("unix_hanson");
    unlink(path);

    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sun_family = AF_LOCAL;
    strcpy(svraddr.sun_path, path);

    if (bind(fd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    struct sockaddr_un cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    for (;;) {
        socklen_t len = clilen;
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }
        buf[nread] = 0;
        printf("svr recv %zd bytes from client content = %s\n", nread, buf);
        if ((nwrite = sendto(fd, buf, nread, 0, (struct sockaddr *)&cliaddr, len)) < 0) {
            perror("sendto error!");
            exit(1);
        }
    }
    return 0;
}