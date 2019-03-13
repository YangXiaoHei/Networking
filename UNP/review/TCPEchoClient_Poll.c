#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../tool/TimeTool.h"
#include "../tool/CommonTool.h"
#include <poll.h>
#include <errno.h>

int  main(int argc, char *argv[])
{
    if (argc != 3) {
        logx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        logx("tcp_connect_cb error! %s", strerror(errno));
        exit(1);
    }

    struct pollfd events[2];
    events[0].fd = STDIN_FILENO;
    events[0].events = POLLRDNORM;
    events[1].fd = fd;
    events[1].events = POLLRDNORM;

    ssize_t nwrite, nread;
    int finihsed = 0;
    char buf[8192];
    for (;;) {
        if (poll(events, 2, -1) < 0) {
            if (errno != EINTR)
                logx("poll error! %s", strerror(errno));
            continue;
        }
        if (events[0].revents & POLLRDNORM) {
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                if (ferror(stdin)) {
                    logx("fgets error!");
                    break;
                }
                logx("client send all data, half close~");
                shutdown(fd, SHUT_WR);
                events[0].fd = -1;
                continue;
            } 
            ssize_t ntowrite = strlen(buf);
            if (write(fd, buf, ntowrite) != ntowrite) {
                logx("write error! %s", strerror(errno));
                break;
            }
            logx("send %zd bytes to server", ntowrite);
        }

        if (events[1].revents & POLLRDNORM) {
            if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                logx("read error! %s", strerror(errno));
                break;
            } else if (nread == 0) {
                logx("server closed connection");
                break;
            }
            logx("recv %zd bytes from server", nread);
            if (write(STDOUT_FILENO, buf, nread) != nread) {
                logx("write error! %s", strerror(errno));
                break;
            }
        }
    }
}
