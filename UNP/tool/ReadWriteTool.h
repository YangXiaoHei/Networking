#ifndef READ_WRITE_TOOL_H
#define READ_WRITE_TOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

ssize_t readn(int fd, char *buf, ssize_t n);
ssize_t readline(int fd, char *buf, size_t buflen);
ssize_t writen(int fd, void *vptr, ssize_t n);

#endif
