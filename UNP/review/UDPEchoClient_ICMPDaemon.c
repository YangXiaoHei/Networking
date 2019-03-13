#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>
#include "../tool/TimeTool.h"
#include "../tool/AddrTool.h"
#include "../tool/FdTransmitTool.h"

#define DEFAULT_PATH "unix_hanson"

struct icmp_info {
    int icmp_type;
    int icmp_code;
    int icmp_errno;
};
extern int log_err;

int main(int argc, char *argv[])
{
    log_err = 1;
    if (argc != 3) {
        logx("usage : %s <ip> <port>", argv[0]);
        exit(1);
    }
    int udpfd = -1;
    if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        logx("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    int unixfd = -1;
    if ((unixfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        logx("unix fd create fail!");
        exit(1);
    }

    struct sockaddr_un sun;
    sun.sun_family = AF_LOCAL;
    strcpy(sun.sun_path, DEFAULT_PATH);
    if (connect(unixfd, (struct sockaddr *)&sun, SUN_LEN(&sun)) < 0) {
        logx("connect error!");
        exit(1);
    }
    
    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_family = AF_INET;
    if (bind(udpfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        logx("bind error!");
        exit(1);
    }

    if (write_fd(unixfd, "", 1, udpfd) < 0) {
        logx("write_fd error!");
        exit(1);
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(unixfd, &rset);
    FD_SET(udpfd, &rset);

    int nready = 0;
    int maxfd = udpfd > unixfd ? udpfd : unixfd;
    ssize_t nread, nwrite;
    char buf[8192];

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    int svr_crash = 0;

    for (;;) {
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (ferror(stdin)) {
                logx("fgets error!");
                break;
            }
        }
        ssize_t ntowrite = strlen(buf);
        if (sendto(udpfd, buf, ntowrite, 0, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            logx("sendto error!");
            continue;
        }

        if (!svr_crash)
            FD_SET(unixfd, &rset);
        FD_SET(udpfd, &rset);
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, &tv)) < 0) {
            if (errno != EINTR)
                logx("select error!");
            continue;
        } else if (nready == 0) {
            logx("timeout occur!");
            continue;
        }

        if (FD_ISSET(unixfd, &rset)) {
            struct icmp_info info;
            if ((nread = read(unixfd, &info, sizeof(info))) < 0) {
                logx("icmp_daemon server terminated");
                FD_CLR(unixfd, &rset);
                svr_crash = 1;
                continue;
            } else if (nread == 0) {
                logx("icmp_daemon server close connection");
                svr_crash = 1;
                FD_CLR(unixfd, &rset);
                continue;
            }
            logx("ICMP-info icmp_type=%d icmp_code=%d %s", info.icmp_type, info.icmp_code, strerror(info.icmp_errno));
        }
        if (FD_ISSET(udpfd, &rset)) {
            socklen_t len = clilen;
            if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                logx("recvfrom error!");
                continue;
            }
            char *info = getAddrInfo((struct sockaddr *)&cliaddr);
            logx("recv %zd bytes from %s", nread, info);
            free(info);

            if (cliaddr.sin_port != svraddr.sin_port || cliaddr.sin_addr.s_addr != svraddr.sin_addr.s_addr) {
                logx("udp not from the right sender, ignored");
                continue;
            }
            if (write(STDOUT_FILENO, buf, nread) != nread) {
                logx("write error!");
                continue;
            }
        }
    }
}
