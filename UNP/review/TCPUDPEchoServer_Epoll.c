#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "../tool/TimeTool.h"
#include "../tool/CommonTool.h"
#include "../tool/AddrTool.h"

extern int log_err;
static int nclient;

#define MAXCLI 1024

int udp_cb(int fd)
{
    int on = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}
int tcp_cb(int fd)
{
    int on = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

int check_valid(int connfd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    return getpeername(connfd, (struct sockaddr *)&addr, &len) == 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        logx("usage : %s <port/service>", argv[0]);
        exit(1);
    }

    log_err = 1;

    int listenfd = -1;
    if ((listenfd = tcp_server_cb(argv[1], tcp_cb)) < 0) {
        logx("tcp_server error!");
        exit(1);
    }

    int udpfd = -1;
    if ((udpfd = udp_server_cb(argv[1], udp_cb)) < 0) {
        logx("udp_server error!");
        exit(1);
    }

    struct epoll_event evlist[MAXCLI];
    int epfd = -1;
    if ((epfd = epoll_create(100000)) < 0) {
        logx("epoll error!");
        exit(1);
    }

    struct epoll_event evtcp, evudp;
    evtcp.data.fd = listenfd;
    evudp.data.fd = udpfd;
    evtcp.events = EPOLLIN;
    evudp.events = EPOLLIN;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &evtcp) < 0) {
        logx("epoll_ctl error!");
        exit(1);
    }
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, udpfd, &evudp) < 0) {
        logx("epoll_ctl error!");
        exit(1);
    }

    ssize_t nread, nwrite;
    int nready = 0;
    char buf[8192];
    int connfd = -1;

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    for (;;) {
        if ((nready = epoll_wait(epfd, evlist, MAXCLI, -1)) < 0) {
            if (errno != EINTR)
                logx("epoll_wait error!");
            continue;
        }

        for (int i = 0; i < nready; i++) {
            int fd = evlist[i].data.fd;
            if (fd == listenfd) {
                if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
                    logx("accept error!");
                    continue;
                } 
                int erroccur = 0;
                if (!check_valid(connfd)) {
                    logx("new connection invalid! connfd = %d", connfd);
                    continue;
                }
                char *info = getPeerInfo(connfd);
                char *dst = getSockInfo(connfd);
                logx("new tcp connection from %s -> %s", info, dst);
                if (nclient >= MAXCLI) {
                    logx("service client beyond limit! reject it! connfd = %d, from %s", connfd, info);
                    erroccur = 1;
                    goto listen_err;
                }

                struct epoll_event ev;
                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
                    logx("epoll_ctl error!");
                    erroccur = 1;
                    goto listen_err;
                }
             listen_err:
                if (erroccur)
                    close(connfd);
                free(info);
                free(dst);
            } else if (fd == udpfd) {
                socklen_t len = clilen;
                if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                    logx("recvfrom error!");
                    continue;
                }
                char *info = getAddrInfo((struct sockaddr *)&cliaddr); 
                logx("udp data %zd bytes from %s", nread, info);
                free(info);

                if (sendto(udpfd, buf, nread, 0, (struct sockaddr *)&cliaddr, len) < 0) {
                    logx("sendto error!");
                    continue;
                }
            } else {
                char *info = getPeerInfo(connfd);
                char *dst = getSockInfo(connfd);
                int erroccur = 0;
                if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                    logx("read error! from %s", info);
                    erroccur = 1;
                    goto read_err;
                } else if (nread == 0) {
                    logx("client closed connection from %s", info);
                    erroccur = 1;
                    goto read_err;
                }
                logx("recv %zd tcp bytes from %s -> %s", nread, info, dst);
                if (write(fd, buf, nread) != nread) {
                    logx("write error! to %s", info);
                    erroccur = 1;
                    goto read_err;
                }
            read_err:
                if (erroccur) {
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, evlist + i) < 0) 
                        logx("epoll_ctl error! for del connfd = %d", fd);
                    close(fd);
                    nclient--;
                }
                free(info);
                free(dst);
            }
        }
    }

}
































