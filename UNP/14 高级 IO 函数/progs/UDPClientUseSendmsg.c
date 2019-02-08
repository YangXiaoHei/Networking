#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/uio.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    static char *msg1 = "hello world";
    static char *msg2 = "i love you";
    static char *msg3 = "hanson yang";

    struct msghdr msg;
    bzero(&msg, sizeof(msg));
    struct iovec iov[3];
    iov[0].iov_base = msg1;
    iov[0].iov_len = strlen(msg1);
    iov[1].iov_base = msg2;
    iov[1].iov_len = strlen(msg2);
    iov[2].iov_base = msg3;
    iov[2].iov_len = strlen(msg3);

    msg.msg_iov = iov;
    msg.msg_iovlen = sizeof(iov) / sizeof(iov[0]);

    /* 已连接套接字不能在发送数据时再指定套接字地址，即便用的和 connect 时一样也不行 */
    // msg.msg_name = &svraddr;
    // msg.msg_namelen = sizeof(svraddr);

    ssize_t nwrite = 0;
    if ((nwrite = sendmsg(fd, &msg, 0)) < 0) {
        perror("sendmsg error!");
        exit(1);
    }
    printf("sendmsg succ!\n");

    return 0;
}