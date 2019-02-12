#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    if (argc != 3 && argc != 4) {
        printf("usage : %s <ip> <port> [timeout]\n", argv[0]);
        exit(1);
    }

    int timeout = 3;
    if (argc == 4)
        timeout = atoi(argv[3]);

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect error!");
            exit(1);
        }
    }

    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    wset = rset;

    int nready = 0;
    if ((nready = select(fd + 1, &rset, &wset, NULL, (timeout != 0 ? &tv : NULL))) == 0) {
        close(fd);
        errno = ETIMEDOUT;
        perror("select --- connect error!");
        exit(1);
    } 

    int error = 0;
    socklen_t len = sizeof(error);
    if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) {
        if (FD_ISSET(fd, &rset)) {
            printf("readable\n");
        }
        if (FD_ISSET(fd, &wset)) {
            printf("writeable\n");
        }
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            perror("getsockopt error!");
            exit(1);
        }
    } else {
        printf("select error! sockfd not set!\n");
        exit(1);
    }

    if (error == 0) {
        printf("connect to succ!\n");
    } else {
        errno = error;
        perror("final --- connect error!");
        exit(1);
    }

    return 0;
}