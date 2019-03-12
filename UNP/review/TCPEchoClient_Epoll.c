#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include "TimeTool.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include "CommonTool.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        mlogx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        mlogx("tcp_connect_cb error! %s", strerror(errno));
        exit(1);
    }
    struct epoll_event evlist[2];

    struct epoll_event evstd, evfd;
    evstd.data.fd = STDIN_FILENO;
    evstd.events = EPOLLIN;
    evfd.data.fd = fd;
    evfd.events = EPOLLIN;

    int epfd = -1;
    if ((epfd = epoll_create(1000)) < 0) {
        mlogx("epoll_create error! %s", strerror(errno));
        exit(1);
    }

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO,  &evstd) < 0) {
        mlogx("epoll_ctl error! %s", strerror(errno));
        exit(1);
    }
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd,  &evfd) < 0) {
        mlogx("epoll_ctl error! %s", strerror(errno));
        exit(1);
    }

    char buf[8192];
    ssize_t nread, nwrite;
    int nready = 0;
    for (;;) {
        if ((nready = epoll_wait(epfd, evlist, sizeof(evlist), -1)) < 0) {
            if (errno != EINTR)
                mlogx("epoll_wait error! %s", strerror(errno));
            continue;
        }
        for (int i = 0; i < nready; i++) {
            if (evlist[i].data.fd == STDIN_FILENO) {
                if (fgets(buf, sizeof(buf), stdin) == NULL) {
                    if (ferror(stdin)) {
                        mlogx("fgets error!");
                        exit(1);
                    }
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, STDIN_FILENO, &evstd) < 0) {
                        mlogx("epoll_ctl error! %s", strerror(errno));
                        exit(1);
                    }
                    mlogx("client send all data, half close~");
                    shutdown(fd, SHUT_WR);
                }
                ssize_t ntowrite = strlen(buf);
                if (write(fd, buf, ntowrite) != ntowrite) {
                    mlogx("write error! %s", strerror(errno));
                    exit(1);
                }
                mlogx("send %zd bytes to server", ntowrite);
            }
            if (evlist[i].data.fd == fd) {
                if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                    mlogx("read error! %s", strerror(errno));
                    exit(1);
                } else if (nread == 0) {
                    mlogx("server close connection");
                    exit(0);
                }
                mlogx("recv %zd bytes from server", nread);
                if (write(STDOUT_FILENO, buf, nread) != nread) {
                    mlogx("write error! %s", strerror(errno));
                    exit(1);
                }
            }
        }
    }
}
