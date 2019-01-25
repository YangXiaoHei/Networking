#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        printf("usage : %s <ip> <port> <is_on>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &svraddr.sin_addr) < 0) {
        perror("inet_pton error!");
        exit(1);
    }

    int on = atoi(argv[3]) != 0;
    socklen_t len = sizeof(on);
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, len) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }
    printf("after set, on = %d\n", on);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    char *msg = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < strlen(msg); i++) {
        if (write(fd, &msg[i], 1) != 1) {
            perror("write error!");
            exit(1);
        }
    }

    shutdown(fd, SHUT_WR);

    return 0;
}