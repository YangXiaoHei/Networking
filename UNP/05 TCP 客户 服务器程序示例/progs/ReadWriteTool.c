#include "ReadWriteTool.h"

ssize_t writen(int fd, void *vptr, size_t n)
{
    ssize_t nleft = n;
    char *ptr = vptr;
    ssize_t nwrite = 0;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return ptr - (char *)vptr;
}

static ssize_t read_cnt;
static char read_buf[4096];
static char *read_ptr;

ssize_t __internal_read(int fd, char *c)
{
    if (read_cnt <= 0) {
    again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            else
                return -1;
        } else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }
    read_cnt--;
    *c = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, char *buf, size_t buflen)
{
    char c = 0;
    ssize_t n = 0;
    ssize_t nread = 0;
    for (n = 1; n < buflen; n++) {
        if ((nread = __internal_read(fd, &c)) == 1) {
            *buf++ = c;
            if (c == '\n')
                break;
        } else if (nread == 0) {
            *buf = 0;
            return n - 1;
        } else
            return -1;
    }
    *buf = 0;
    return n;
}