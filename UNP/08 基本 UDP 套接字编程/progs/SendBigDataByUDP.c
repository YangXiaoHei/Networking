#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define VEASON_IP "119.29.207.157"

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);

    if (argc < 2) {
        printf("usage : %s <port> [send bytes KB default 10k]\n", argv[0]);
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
    svraddr.sin_port = htons(atoi(argv[1]));
    if (inet_pton(AF_INET, VEASON_IP, &svraddr.sin_addr) < 0) {
        perror("ip format error");
        exit(1);
    }

    int size = 10 << 10;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    const int bufsize = (argc == 3 ? atoi(argv[2]) : 10) << 10;
    char *buf = malloc(bufsize);
    memset(buf, 'a', bufsize);

    if (sendto(fd, buf, bufsize, 0, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("sendto error!");
        exit(1);
    }

    printf("sento succ!\n");

    close(fd);

    return 0;
}
