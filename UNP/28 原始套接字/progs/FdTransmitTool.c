#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int write_fd(int unixfd, void *ptr, size_t len, int totransfd)
{
    struct msghdr msg;
    bzero(&msg, sizeof(msg));
    struct cmsghdr *cptr;
    
    union {
        struct msghdr m;
        char control[CMSG_LEN(sizeof(int))];
    } control;

    struct iovec iov[1];
    iov[0].iov_base = ptr;
    iov[0].iov_len = len;
    
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    msg.msg_control = control.control;
    msg.msg_controllen = sizeof(control);
    cptr = CMSG_FIRSTHDR(&msg);
    cptr->cmsg_level = SOL_SOCKET;
    cptr->cmsg_type = SCM_RIGHTS;
    cptr->cmsg_len = CMSG_LEN(sizeof(int));
    *(int *)CMSG_DATA(cptr) = totransfd;
    
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    return sendmsg(unixfd, &msg, 0);
}

int read_fd(int unixfd, void *buf, size_t buflen, int *recvfd)
{
    struct msghdr msg;
    struct cmsghdr *cptr;
    
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = buflen;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    union {
        struct msghdr m;
        char control[CMSG_LEN(sizeof(int))];
    } control;

    msg.msg_control = control.control;
    msg.msg_controllen = sizeof(control);

    ssize_t n;
    if ((n = recvmsg(unixfd, &msg, 0)) <= 0) 
        return -1;
    
    if (recvfd) *recvfd = -1; 
    if ((cptr = CMSG_FIRSTHDR(&msg)) != NULL && cptr->cmsg_len == CMSG_LEN(sizeof(int))) {
        if (cptr->cmsg_level != SOL_SOCKET || cptr->cmsg_type != SCM_RIGHTS)
            return -2;
        if (recvfd) *recvfd = *(int *)CMSG_DATA(cptr); 
    }
    return n;
}


























