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

ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
    struct msghdr msg;
    bzero(&msg, sizeof(msg));
    struct iovec iov[1];

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    struct cmsghdr *cmsgptr;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    cmsgptr = CMSG_FIRSTHDR(&msg);
    cmsgptr->cmsg_len = CMSG_LEN(sizeof(int));  /* CMSG_LEN 是头部再加数据长度，因为描述符是一个 int 型 */
    cmsgptr->cmsg_level = SOL_SOCKET;
    cmsgptr->cmsg_type = SCM_RIGHTS;

    *((int *)CMSG_DATA(cmsgptr)) = sendfd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    return sendmsg(fd, &msg, 0);
}

/* ./progs sockfd pathname mode */
int main(int argc, char const *argv[])
{
    if (argc != 4) {
        printf("usage : %s <sockfd> <pathname> <mode>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = open(argv[2], atoi(argv[3]))) < 0) 
        exit((errno > 0) ? errno : 255);

    if (write_fd(atoi(argv[1]), "", 1, fd) < 0)
        exit((errno > 0) ? errno : 255);

    exit(0);
}