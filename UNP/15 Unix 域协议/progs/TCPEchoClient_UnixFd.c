#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include "PathTool.h"

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    strcpy(svraddr.sun_path, getAbsolutePath("unix_hanson"));
    svraddr.sun_family = AF_LOCAL;

    if (connect(fd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    fd_set rset, allset;
    FD_SET(fd, &allset);
    FD_SET(fileno(stdin), &allset);

    int maxfd = fd > fileno(stdin) ? fd : fileno(stdin);

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    for (;;) {
        rset = allset;
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            perror("select error!");
            exit(1);
        }
        if (FD_ISSET(fileno(stdin), &rset)) {
            if ((nread = read(fileno(stdin), buf, sizeof(buf))) < 0) {
                perror("read error!");
                exit(1);
            } else if (nread == 0) {
                printf("client finished!\n");
                shutdown(fd, SHUT_WR);
                FD_CLR(fileno(stdin), &allset);
            } else {
                if ((nwrite = write(fd, buf, nread)) != nread) {
                    perror("write error!");
                    exit(1);
                }
            }
        } 

        if (FD_ISSET(fd, &rset)) {
            if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                exit(1);
            } else if (nread == 0) {
                printf("server terminated\n");
                exit(1);
            } else {
                if ((nwrite = write(fileno(stdout), buf, nread)) != nread) {
                    perror("write error!");
                    exit(1);
                }
            }
        }
    }

    return 0;
}