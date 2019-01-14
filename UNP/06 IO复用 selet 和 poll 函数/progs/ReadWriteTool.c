#include "ReadWriteTool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAXLEN 2048
static char read_buf[MAXLEN];
static char *read_ptr;
static ssize_t read_cnt;

ssize_t _read_one_char(int fd, char *ptr) 
{
    if (read_cnt <= 0) {
    again:
        if ((read_cnt = read(fd, read_buf, MAXLEN)) < 0) {
            if (errno == EINTR)
                goto again;
            return -1;
        } else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1; 
}

ssize_t readline(int fd, char *buf, size_t maxlen)
{
    ssize_t n = 0;
    char c = 0;
    ssize_t nread = -1;
    for (n = 1; n < maxlen; n++) {
        if ((nread = _read_one_char(fd, &c)) == 1) {
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

ssize_t writen(int fd, char *ptr, size_t len)
{
    ssize_t nleft = len;
    ssize_t nwrite = 0;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        ptr += nwrite;
        nleft -= nwrite;
    }
    return len;
}

