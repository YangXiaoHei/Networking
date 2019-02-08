#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <sys/param.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    int on = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_RECVORIGDSTADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd ,(struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    char *buf1 = malloc(5);
    char *buf2  = malloc(5);
    char *buf3  = malloc(10);
    char *buf4  = malloc(10);
    bzero(buf1, 5);
    bzero(buf2, 5);
    bzero(buf3, 10);
    bzero(buf4, 10);

    struct iovec iov[4];
    iov[0].iov_base = buf1;
    iov[0].iov_len = sizeof(buf1);
    iov[1].iov_base = buf2;
    iov[1].iov_len = sizeof(buf2);
    iov[2].iov_base = buf3;
    iov[2].iov_len = sizeof(buf3);
    iov[3].iov_base = buf4;
    iov[3].iov_len = sizeof(buf4);

    struct cmsghdr *cmsgptr;

    char control[64];

    struct msghdr msg;
    bzero(&msg, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = sizeof(iov) / sizeof(iov[0]);
    msg.msg_name = &cliaddr;
    msg.msg_namelen = sizeof(cliaddr);

    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    
    if (recvmsg(fd, &msg, 0) < 0) {
        perror("recvmsg error!");
        exit(1);
    }

    char *srcInfo = getAddrInfo((struct sockaddr_in *)msg.msg_name);
    printf("udp req from %s\n", srcInfo);
    free(srcInfo);

    /* 用这种方法无法得到 udp 套接字的目的 IP */
    char *dstInfo = getSockInfo(fd);
    printf("[uncorrect method] udp dst ip/port %s\n", dstInfo);
    free(dstInfo);

    /* 获得 udp 套接字目的 IP 的正确姿势 */
    for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
        if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_RECVORIGDSTADDR) {
            char *ptr = CMSG_DATA(cmsgptr);
            struct sockaddr_in dstaddr;
            memcpy(&dstaddr, ptr, sizeof(struct sockaddr_in));
            char *addrInfo = getAddrInfo(&dstaddr);
            printf("[correct method] udp dst ip/port %s\n", addrInfo);
            free(addrInfo);
        }
    }

    printf("buf1 = %s\n", buf1);
    printf("buf2 = %s\n", buf2);
    printf("buf3 = %s\n", buf3);
    printf("buf4 = %s\n", buf4);

    return 0;
}