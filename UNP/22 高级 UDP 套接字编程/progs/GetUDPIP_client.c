#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }   

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    ssize_t nread, nwrite;
    char c = 1;
    char buf[1024];
    for (;;) {
        if ((nread = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        }
        if (sendto(sockfd, buf, nread, 0, NULL, 0) != nread) {
            perror("sendto error!");
            exit(1);
        }

        if ((nread = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }
        if ((nwrite = write(STDOUT_FILENO, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }

    return 0;
}