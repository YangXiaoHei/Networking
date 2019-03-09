#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include "../../tool/AddrTool.h"
#include "../../tool/TimeTool.h"

#define NCONN 5

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int client[NCONN];
    int fd;
    fd_set rset, wset;
    fd_set rallset, wallset;
    FD_ZERO(&rallset);
    FD_ZERO(&wallset);

    int maxfd = 0;

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    struct sockaddr_in cliaddr;

    for (int i = 0; i < NCONN; i++) {
        client[i] = -1;
        if ((client[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            logx("socket fd create fail!");
            exit(1);
        }
        int fd = client[i];
        if (fd > maxfd)
            maxfd = fd;
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);

        logx("fd = %d connect invoked", fd);
        if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            if (errno != EINPROGRESS) {
                logx("connect error!");
                continue;
            }
            FD_SET(fd, &rallset);
            FD_SET(fd, &wallset);
        } else {
            char *sockInfo = getSockInfo(fd);
            logx("fd = %d connect succ, sockaddr is %s", sockInfo);
            free(sockInfo);
        }
    }

    int error = 0;
    int nfinish = 0;
    for (;;) {
        if (nfinish == NCONN)
            break;
        rset = rallset;
        wset = wallset;
        if ((select(maxfd + 1, &rset, &wset, NULL, NULL)) < 0) {
            if (errno != EINTR)
                logx("select error! %s", strerror(errno));
            continue;
        }
        for (int i = 0; i < NCONN; i++) {
            int fd = client[i];
            if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) {
                nfinish++;
                socklen_t len = sizeof(error);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    logx("fd = %d connect fail! %s", fd, strerror(error == 0 ? errno : error));
                } else {
                    char *sockInfo = getSockInfo(fd);
                    logx("fd = %d connect succ! sockaddr is %s", fd, sockInfo);
                    free(sockInfo);
                }
                FD_CLR(fd, &rallset);
                FD_CLR(fd, &wallset);
            }
        }
    }
}
