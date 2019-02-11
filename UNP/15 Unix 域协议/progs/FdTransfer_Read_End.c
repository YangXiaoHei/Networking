#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
    struct msghdr msg;
    bzero(&msg, sizeof(msg));
    struct iovec iov[1];
    ssize_t n;

    struct cmsghdr *cmsgptr;

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if ((n = recvmsg(fd, &msg, 0)) <= 0)
        return n;

    if ((cmsgptr = CMSG_FIRSTHDR(&msg)) != NULL && cmsgptr->cmsg_len == CMSG_LEN(sizeof(int))) {
        if (cmsgptr->cmsg_level != SOL_SOCKET) {
            printf("control level != SOL_SOCKET!\n");
            exit(1);
        }
        if (cmsgptr->cmsg_type != SCM_RIGHTS) {
            printf("control type != SCM_RIGHTs!\n");
            exit(1);
        }
        *recvfd = *(int *)CMSG_DATA(cmsgptr);
    } else
        *recvfd = -1;

    return n;
}

int my_open(const char *pathname, int mode)
{
    int sockfds[2];

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfds) < 0) {
        perror("socketpair error!");
        exit(1);
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        /* 子进程使用 fds[0] */
        close(sockfds[1]);

        char argmode[64];
        char argsockfd[64];

        snprintf(argmode, sizeof(argmode), "%d", mode);
        snprintf(argsockfd, sizeof(argsockfd), "%d", sockfds[0]);

        execl("./openfile", "openfile", argsockfd, pathname, argmode, (char *)NULL);
        printf("execl error!\n");
        exit(1);
    }

    /* 父进程使用 fds[1] */
    close(sockfds[0]);

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid error!");
        exit(1);
    }
    if (WIFEXITED(status) == 0) {
        printf("child did not terminated!\n");
        exit(1);
    }
    int fd = 0;
    char c = 0;
    if ((status = WEXITSTATUS(status)) == 0) {
        read_fd(sockfds[1], &c, 1, &fd);
    } else {
        errno = status;
        fd = -1;
    }
    close(sockfds[1]);
    return fd;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <pathname>\n", argv[0]);
        exit(1);
    }

    char buf[1024];
    int fd = -1;
    if ((fd = my_open(argv[1], O_RDONLY)) < 0) {
        perror("my_open error!");
        exit(1);
    }

    ssize_t nread = 0, nwrite = 0;
    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        if ((nwrite = write(fileno(stdout), buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread < 0) {
        perror("read error!");
        exit(1);
    } else {
        printf("all data read finished!\n");
    }

    return 0;
}