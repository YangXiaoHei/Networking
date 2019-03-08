#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include "../../tool/ReadWriteTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

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

    int lowermark = 5;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, &lowermark, sizeof(lowermark)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    fd_set rset;
    FD_ZERO(&rset);

    char buf[1024];

    while (1) {
        FD_SET(fd, &rset);
        FD_SET(fileno(stdin), &rset);
        int maxfds = (fileno(stdin) > fd ? fileno(stdin) : fd) + 1;
        int nready = 0;
        if ((nready = select(maxfds, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            exit(1);
        }
        printf("%d fds ready to handle: ", nready);

        if (FD_ISSET(fileno(stdin), &rset)) {
            printf("stdin fd\n");
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                if (ferror(stdin)) {
                    printf("fgets error!");
                    exit(1);
                }
                printf("all finished!");
                shutdown(fd, SHUT_WR);
            }
            if (writen(fd, buf, strlen(buf)) < 0) {
                perror("writen error!");
                exit(1);
            }
        }

        if (FD_ISSET(fd, &rset)) {
            printf("socket fd\n");
            ssize_t n = 0;
            if ((n = read(fd, buf, sizeof(buf))) < 0) {
                perror("readline error!");
                exit(1);
            } else if (n == 0) {
                printf("EOF\n");
                break;
            }
            if (fputs(buf, stdout) == EOF) {
                if (ferror(stdout)) {
                    printf("fputs error!");
                    exit(1);
                }
            }
        }
    }
    
    return 0;
}
