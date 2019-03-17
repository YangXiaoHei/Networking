#ifndef _READLINE_SAFE_H_
#define _READLINE_SAFE_H_

ssize_t readline(int fd, void *vptr, size_t len);
ssize_t writen(int fd, void *vptr, ssize_t n);
#endif
