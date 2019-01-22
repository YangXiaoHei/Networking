#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

char *getAddrInfo(struct sockaddr_in *addr) 
{
    size_t slen = 1 /* [] */ + 15 /* xxx.xxx.xxx.xxx */ + 1 /* : */ + 5 /* port */ + 1 /* \0 */ + 1 /* ] */;
    char *buf = NULL;
    if ((buf = malloc(slen)) == NULL) {
        printf("malloc error!");
        exit(1);
    }
    const char *ip = inet_ntoa(addr->sin_addr);
    slen = snprintf(buf, slen, "[%s:%d]", ip, ntohs(addr->sin_port));
    buf[slen] = 0;
    return buf;
}

char *getPeerInfo(int fd) 
{   
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    if (getpeername(fd, (struct sockaddr *)&peeraddr, &peerlen) < 0) {
        perror("getpeername error!");
        exit(1);
    }
    return getAddrInfo(&peeraddr);
}

char *getSockInfo(int fd) 
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) < 0) {
        perror("getsockname error!");
        exit(1);
    }
    return getAddrInfo(&addr);
}