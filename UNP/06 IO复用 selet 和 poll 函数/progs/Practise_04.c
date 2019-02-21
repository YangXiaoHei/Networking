#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include "../../tool/AddrTool.h"
#include <sys/resource.h>

void printClifds(int *clifds) 
{
    for (int i = 0; i < 5; i++) {
        printf("%-3d", clifds[i]);
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    int connfd = -1;
    int maxfd = listenfd;
    int nready = 0;
    int maxi = 0;
    socklen_t clilen = sizeof(cliaddr);

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("getrlimit error!");
        exit(1);
    }
    printf("FD_SETSIZE = %llu; rlimit_cur = %llu, rlimit_max = %llu\n", (unsigned long long)FD_SETSIZE, rl.rlim_cur, rl.rlim_max);

    rl.rlim_max = rl.rlim_cur;
    if (setrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("setrlimit error!");
        exit(1);
    }

    int clifds[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++)
        clifds[i] = -1;

    while (1) {
        rset = allset;
        select_again:
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
                goto select_again;
            perror("select error!");
            exit(1);
        }
        printf("%d fds readable ", nready);

        // 新连接进来
        if (FD_ISSET(listenfd, &rset)) {
            printf("listenfd\n");
            if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept error!");
                exit(1);
            }

            // 存储一份 connfd 以方便之后通信
            int i = 0;
            for (; i < FD_SETSIZE; i++) {
                if (clifds[i] < 0) {
                    clifds[i] = connfd;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                printf("too many clients\n");
                exit(1);
            }

            char *peerInfo = getPeerInfo(connfd);
            char *addrInfo = getSockInfo(connfd);
            printf("%s <---> %s\n", addrInfo, peerInfo);
            free(peerInfo);
            free(addrInfo);

            // 把新客户端添加进监听可读事件列表中
            FD_SET(connfd, &allset);

            // 给 select 用
            if (connfd > maxfd)
                maxfd = connfd;

            // 给查找客户端 connfd 通信用
            if (i > maxi)
                maxi = i;

            if (--nready <= 0) 
                continue;
        }

        printf("connfd\n");

        for (int j = 0; j <= maxi; j++) {

            if (clifds[j] < 0) 
                continue;
            
            if (FD_ISSET(clifds[j], &rset)) {
                if ((nread = read(clifds[j], buf, sizeof(buf))) == 0) {
                    close(clifds[j]);
                    FD_CLR(clifds[j], &allset);
                    clifds[j] = -1;
                } else if (nready < 0) {
                    /* 为什么会发生这种情况呢？ */
                } else {
                    if ((nwrite = write(clifds[j], buf, nread)) != nread) {
                        perror("write error!");
                        exit(1);
                    }
                }
                if (--nready <= 0)
                    break;
            }
        }
    }

    return 0;
}