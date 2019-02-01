#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd error!");
        exit(1);
    }

    int bufsize = 0;
    socklen_t len = sizeof(bufsize);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }

    printf("send buff size = %d\n", bufsize);

    len = sizeof(bufsize);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }

    printf("recv buff size = %d\n", bufsize);

    return 0;
}