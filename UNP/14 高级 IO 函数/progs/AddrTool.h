#ifndef _ADDR_TOOL_H
#define _ADDR_TOOL_H

#include <stdio.h>
char *getAddrInfo(struct sockaddr_in *addr);
char *getPeerInfo(int fd);
char *getSockInfo(int fd);

#endif