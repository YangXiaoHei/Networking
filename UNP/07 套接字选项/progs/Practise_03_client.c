#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct linger l;
    l.l_onoff = 1;
    l.l_linger = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        perror("setsockopt error!");
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

    struct pollfd fds[2];
    fds[0].fd = fileno(stdin);
    fds[0].events = POLLRDNORM;
    fds[1].fd = fd;
    fds[1].events = POLLRDNORM;

    ssize_t nread = 0, nwrite = 0;

    int nready = 0;
    char buf[1024];
    for (;;) {
        if ((nready = poll(fds, 2, -1)) < 0) {
            perror("poll error!");
            exit(1);
        }

        if (fds[0].revents & POLLRDNORM) {
            if ((nread = read(fileno(stdin), buf, sizeof(buf))) < 0) {
                perror("read error!");
                exit(1);
            }   
            if ((nwrite = write(fds[1].fd, buf, nread)) != nread) {
                perror("write error!");
                exit(1);
            }
            if (--nready <= 0)
                continue;
        }

        if (fds[1].revents & POLLRDNORM) {
            if ((nread = read(fds[1].fd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                exit(1);
            }   
            if ((nwrite = write(fileno(stdout), buf, nread)) != nread) {
                perror("write error!");
                exit(1);
            }

            if (--nready <= 0)
                continue;
        }
    }


    return 0;
}