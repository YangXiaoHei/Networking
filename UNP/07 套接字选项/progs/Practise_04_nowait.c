#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    int portToBind = 20005;

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    svraddr.sin_port = htons(atoi(argv[2]));

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(portToBind);
    if (bind(fd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    for (;;);

    return 0;
} 