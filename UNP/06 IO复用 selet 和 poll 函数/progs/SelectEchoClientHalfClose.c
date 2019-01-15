#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
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

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fileno(stdin), &rset);
    FD_SET(fd, &rset);
    ssize_t nread = 0, nwrite = 0;
    char buf[1024];
    int nready = 0;
    int maxfds = (fileno(stdin) > fd ? fileno(stdin) : fd) + 1;
    int isstdinEOF = 0;
    while (1) {
        if (!isstdinEOF) {
            FD_SET(fileno(stdin), &rset);
        }
        FD_SET(fd, &rset);

        select_again:
        if ((nready = select(maxfds, &rset, NULL, NULL, NULL)) < 0) {
            if (nready < 0 && errno == EINTR)
                goto select_again;
            perror("select error!");
            exit(1);
        }

        if (FD_ISSET(fileno(stdin), &rset)) {
            read_again:
            if ((nread = read(fileno(stdin), buf, sizeof(buf))) < 0) {
                if (errno == EINTR)
                    goto read_again;
                perror("read from stdin error!");
                exit(1);
            } else if (nread == 0) {
                shutdown(fd, SHUT_WR);
                FD_CLR(fileno(stdin), &rset);
                isstdinEOF = 1;
            } else {
                if ((nwrite = write(fd, buf, nread)) != nread) {
                    perror("write error!");
                    exit(1);
                }
            }
        }

        if (FD_ISSET(fd, &rset)) {
            r_again:
            if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                if (errno == EINTR)
                    goto r_again;
                perror("read from socket error!");
                exit(1);
            } else if (nread == 0) {
                close(fd);
                printf("server terminated\n");
                break;
            } else {
                if ((nwrite = write(fileno(stdout), buf, nread)) != nread) {
                    perror("write to stdout error!");
                    exit(1);
                }
            }
        }
    }
    return 0;
}