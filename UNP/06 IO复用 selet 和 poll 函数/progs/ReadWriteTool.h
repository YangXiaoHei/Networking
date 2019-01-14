#ifndef _READ_WRITE_TOOL_H
#define _READ_WRITE_TOOL_H

#include <stdio.h>

ssize_t readline(int fd, char *buf, size_t maxlen);
ssize_t writen(int fd, char *ptr, size_t len);

#endif