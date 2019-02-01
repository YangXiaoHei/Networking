#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    int mss = 0;
    socklen_t len = sizeof(mss);
    if (getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }
    printf("before TCP established, mss = %d\n", mss);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    len = sizeof(mss);
    if (getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }
    printf("after TCP established, mss = %d\n", mss);


    return 0;
}