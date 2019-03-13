#ifndef _FD_TRANSMIT_H_
#define _FD_TRANSMIT_H_

int write_fd(int unixfd, void *ptr, size_t len, int totransfd);
int read_fd(int unixfd, void *buf, size_t buflen, int *recvfd);

#endif
