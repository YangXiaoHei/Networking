#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"

int fd;
static sig_atomic_t svr_crash;
void sig_usr1(int signo) { svr_crash = 1; /* just for interrupt */  }

void child(void)
{
    if (fuck_int(SIGUSR1, sig_usr1) == SIG_ERR) {
        logx("fuck_int error! %s", strerror(errno));
        exit(1);
    }
    char buf[8192];
    ssize_t nread, nwrite;
    while ((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        ssize_t ntowrite = nread;
        if (write(fd, buf, ntowrite) != ntowrite) {
            logx("write error! %s", strerror(errno));
            exit(1);
        }
        logx("send %zd to server", ntowrite);
    }
    if (nread == 0) {
        logx("client send all data, half close~");
    } else if (!svr_crash) {
        logx("read error! %s", strerror(errno));
    } else {
        logx("server terminated abnormally");
    }
    shutdown(fd, SHUT_WR);
    exit(0);
}

void parent(pid_t pid)
{
    char buf[8192];
    ssize_t nread, nwrite;
    for (;;) {
        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            logx("read error! %s", strerror(errno));
            exit(1);
        } else if (nread == 0) {
            logx("server closed connection");
            kill(pid, SIGUSR1);
            exit(0);
        }
        logx("recv %zd bytes from server", nread);
        if (write(STDOUT_FILENO, buf, nread) != nread) {
            logx("write error! %s", strerror(errno));
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        logx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        logx("tcp_connect_cb error! %s", strerror(errno));
        exit(1);
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        logx("fork error! %s", strerror(errno));
        exit(1);
    } else if (pid == 0)
        child();

    parent(pid);

    int status;
    if (wait(&status) < 0) {
        logx("wait error! %s", strerror(errno));
        exit(1);
    }
}
