#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"
#include "../tool/ReadWriteTool.h"
#include <sys/select.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        logx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        perror("tcp_connect_cb error!");
        exit(1);
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    FD_SET(STDIN_FILENO, &rset);

    int maxfd = fd > STDIN_FILENO ? fd : STDIN_FILENO;

    char buf[8192];
    ssize_t nread, nwrite;
    int finished = 0;
    for (;;) {
        FD_SET(fd, &rset);
        if (!finished)
            FD_SET(STDIN_FILENO, &rset);

        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            if (errno != EINTR)
                perror("select error!");
            continue;
        }        
       
        if (FD_ISSET(fd, &rset)) {
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

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                if (ferror(stdin)) {
                    logx("fgets error!");
                    break;
                }
                shutdown(fd, SHUT_WR); /* half close */
                logx("client send all data, half close");
                FD_CLR(STDIN_FILENO, &rset);
                finished = 1;
                continue;
            }
            ssize_t ntowrite = strlen(buf);
            if (writen(fd, buf, ntowrite) != ntowrite) {
                logx("writen error!");
                break;
            }
            logx("send %zd bytes to server", ntowrite);
        }
    }
    close(fd);
}
