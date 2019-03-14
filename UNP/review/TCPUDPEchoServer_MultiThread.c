#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"
#include "../tool/AddrTool.h"

int listenfd;
int udpfd;
pthread_t tid;

fd_set rset;

int hhhh;

int check_valid(int connfd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    return getpeername(connfd, (struct sockaddr *)&addr, &len) == 0;
}

void *thread_main(void *arg)
{
    int connfd = (int *)arg;
    ssize_t nread, nwrite;
    char buf[8192];

    pthread_detach(pthread_self());
    char *src = getPeerInfo(connfd);

    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            logx("read error from %s!", src);
            break;
        } else if (nread == 0) {
            logx("client %s closed connection", src);
            break;
        }
        logx("recv %zd bytes from client %s", nread, src);
        if ((write(connfd, buf, nread)) != nread) {
            logx("write error!");
            break;
        }
        logx("send %zd bytes to client %s", nread, src);
    }
    close(connfd);
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        logx("usage : %s <port/service>", argv[0]);
        exit(1);
    }
    if ((listenfd = tcp_server_cb(argv[1], NULL)) < 0) {
        logx("tcp_server_cb error!");
        exit(1);
    }

    if ((udpfd = udp_server_cb(argv[1], NULL)) < 0) {
        logx("udp_server_cb error!");
        exit(1);
    }

    int maxfd = udpfd > listenfd ? udpfd : listenfd;

    FD_ZERO(&rset);

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    char buf[8192];
    ssize_t nread, nwrite;
    int connfd = -1;
    for (;;) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            logx("select error!");
            continue;
        }

        if (FD_ISSET(udpfd, &rset)) {
            socklen_t len = clilen;
            if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                logx("recvfrom error!");
                goto udp_end;
            }
            char *info = getAddrInfo((struct sockaddr *)&cliaddr);
            logx("recv %zd bytes udp data from %s", nread, info);
            if (sendto(udpfd, buf, nread, 0, (struct sockaddr *)&cliaddr, len) < 0) {
                logx("sendto error to %s!", info);
                free(info);
                goto udp_end;
            }
            logx("send %zd bytes udp data to %s", nread, info);
            free(info);

        udp_end:
            hhhh = 1;
        }

        if (FD_ISSET(listenfd, &rset)) {
            if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
                logx("accpet error!");
                goto listen_end;
            }
            if (!check_valid(connfd)) {
                logx("new connection invalid! connfd=%d", connfd);
                 goto listen_end;
            }

            char *info = getPeerInfo(connfd);
            logx("new tcp connection from %s", info);
            free(info);

            int error;
            if ((error = pthread_create(&tid, NULL, thread_main, (void *)connfd)) != 0) {
                errno = error;
                logx("pthread_create error!");
                goto listen_end;
            }        
        listen_end:
            hhhh = 1;
        }
    }
}
