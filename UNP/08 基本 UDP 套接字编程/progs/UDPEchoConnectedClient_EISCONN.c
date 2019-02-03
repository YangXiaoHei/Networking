#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

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
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    svraddr.sin_port = htons(atoi(argv[2]));

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nwrite = 0, nread = 0;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            perror("sendto error!");
            exit(1);
        }

        if (recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        }
    }

    return 0;
}