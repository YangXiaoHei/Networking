#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/servname>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    struct addrinfo hints, *res, *ressave;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    const char *host = argv[1];
    const char *service = argv[2];
    int error = 0;
    if ((error = getaddrinfo(host, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", host, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;

    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket error!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(fd);
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("connect error! for %s %s\n", host, service);
        exit(1);
    } else {
        char *localinfo = getAddrInfo((struct sockaddr_in *)res->ai_addr);
        printf("connect to %s succ!\n", localinfo);
        free(localinfo);
    }

    struct pollfd fds[2];
    fds[0].fd = fd;
    fds[0].events = POLLRDNORM;

    fds[1].fd = fileno(stdin);
    fds[1].events = POLLRDNORM;
    
    char buf[1024];
    int nready = 0;
    ssize_t nread = 0, nwrite = 0;

    for (;;) {
        if ((nready = poll(fds, 2, -1)) < 0) {
            perror("poll error!");
            exit(1);
        }

        /* 标准输入 */
        if (fds[1].revents & POLLRDNORM) {
            if ((nread = read(fds[1].fd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                exit(1);
            } else if (nread == 0) {
                printf("all data send finished! client exit\n");
                shutdown(fds[0].fd, SHUT_WR);
                fds[1].fd = -1;
                if (--nready <= 0)
                    continue;
            }
            if ((nwrite = write(fds[0].fd, buf, nread)) != nread) {
                perror("write error!");
                exit(1);
            }
            if (--nready <= 0)
                continue;
        }

        /* 套接字 */
        if (fds[0].revents & POLLRDNORM) {
            if ((nread = read(fds[0].fd, buf, sizeof(buf))) < 0) {
                perror("read error!");
                if (errno == ECONNRESET) {
                    printf("server terminated abnormal\n");
                    close(fds[0].fd);
                    break;
                }
            } else if (nread == 0) {
                printf("server termianted\n");
                close(fds[0].fd);
                break;
            }
            if ((nwrite = write(fileno(stdout), buf, nread)) != nread) {
                perror("write error!");
                exit(1);
            }

            if (--nready <= 0)
                continue;
        }
    }

    freeaddrinfo(ressave);

    exit(0);
}