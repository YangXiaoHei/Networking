#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"
#include "../tool/AddrTool.h"

#define MAXCLI 1024
extern int log_err;

int max(int a, int b) { return a > b ? a : b;  }

int check_valid(int connfd)
{
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    return getpeername(connfd, (struct sockaddr *)&cliaddr, &clilen) ==  0;
}

int tcp_cb(int fd)
{
    int on = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

int udp_cb(int fd)
{
    int on = 1;
    return setsockopt(fd, IPPROTO_IP, IP_RECVORIGDSTADDR, &on, sizeof(on));
}

ssize_t recvfrom_dstaddr(int fd, char *buf, size_t len, int flags, struct sockaddr_in *srcaddr, struct sockaddr_in *dstaddr)
{
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = len;

    struct msghdr msg;
    struct cmsghdr *cmsgptr;

    union {
        struct msghdr msg;
        char ctx[CMSG_LEN(sizeof(struct sockaddr_in))];
    } control;

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control.ctx;
    msg.msg_controllen = sizeof(control);
    msg.msg_name = srcaddr;
    msg.msg_namelen = sizeof(struct sockaddr_in);

    ssize_t n;
    if ((n = recvmsg(fd, &msg, flags)) < 0)
        return n;

    if (srcaddr != NULL) 
        memcpy(srcaddr, msg.msg_name, sizeof(struct sockaddr_in));

    for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) 
        if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_RECVORIGDSTADDR) 
            if (dstaddr != NULL) 
                memcpy(dstaddr, (struct sockaddr_in *)CMSG_DATA(cmsgptr), sizeof(struct sockaddr_in));
    return n;
}

int main(int argc, char *argv[])
{
    log_err = 1;
    if (argc != 2) {
        logx("usage : %s <port/service>", argv[0]);
        exit(1);
    }
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

    int client[MAXCLI]; 
    for (int i = 0; i < MAXCLI; i++)
        client[i] = -1;

    int maxfd = max(udpfd, listenfd);
    int maxi = 0; 

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(udpfd, &allset);
    FD_SET(listenfd, &allset);

    int nready = 0;
    char buf[8192];
    ssize_t nread, nwrite;

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    struct sockaddr_in dstaddr;
    int connfd = -1;
    for (;;) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno != EINTR)
                logx("select error!");
            continue;
        }

        /* TCP 连接接入 */
        if (FD_ISSET(listenfd, &rset)) {
            int i = 0;
            for (i = 0; i < MAXCLI; i++)
                if (client[i] < 0)
                    break;

            int not_service = i == MAXCLI;

            if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
                logx("accept error!");
                goto listen_end;
            }
            
            if (!check_valid(connfd)) {
                logx("new connfd is invalid! ignore it");
                close(connfd);
                goto listen_end;
            }
            char *info = getPeerInfo(connfd);
            char *dst = getSockInfo(connfd);
            logx("tcp connection from %s -> %s, connfd = %d", info, dst, connfd);

            if (not_service) {
                logx("service client beyond limit!, reject this client %s", info);
                close(connfd);
                free(info);
                free(dst);
                goto listen_end;
            }

            client[i] = connfd;
            FD_SET(connfd, &allset);
            maxfd = max(maxfd, connfd);
            maxi = max(maxi, i);
            free(info);
            free(dst);

        listen_end:
            if (--nready <= 0)
                continue;
        }

        /* UDP 数据报待读取 */
        if (FD_ISSET(udpfd, &rset)) {
            if ((nread = recvfrom_dstaddr(udpfd, buf, sizeof(buf), 0, &cliaddr, &dstaddr)) < 0) {
                logx("recvfrom error!");
                goto udp_end;
            }    
            char *info = getAddrInfo((struct sockaddr *)&cliaddr);
            char *dst = getAddrInfo((struct sockaddr *)&dstaddr);
            logx("recv %zd bytes udp data from %s -> %s", nread, info, dst);
            free(info);
            free(dst);
            if (sendto(udpfd, buf, nread, 0, (struct sockaddr *)&cliaddr, clilen) < 0) {
                logx("sendto error!");
                goto udp_end;
            }
        udp_end:
            if (--nready <= 0)
                continue;
        }

        /* 已连接套接字上有数据待读取 */
        for (int i = 0; i <= maxi; i++) {
            if (client[i] < 0) 
                continue;
            if (FD_ISSET(client[i], &rset)) {
                int fd = client[i];
                int erroccur = 0;
                if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                    logx("read error!");
                    erroccur = 1;
                    goto read_end;
                } else if (nread == 0) {
                    logx("client closed connection");
                    erroccur = 1;
                    goto read_end;
                }

                char *info = getPeerInfo(fd);
                char *dst = getSockInfo(fd);
                logx("recv %zd tcp data from %s -> %s", nread, info, dst);
                free(info);
                free(dst);

                if (write(fd, buf, nread) != nread) {
                    logx("write error!");
                    erroccur = 1;
                    goto read_end;
                }
            read_end:
                if (erroccur) {
                    client[i] = -1;
                    FD_CLR(fd, &allset);
                    close(fd);
                }
                if (--nready <= 0)
                    continue;
            }
        }
    }
}
