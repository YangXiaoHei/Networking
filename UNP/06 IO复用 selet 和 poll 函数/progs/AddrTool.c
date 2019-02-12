#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>

char *getAddrInfo(struct sockaddr *addr) 
{
    char *buf = NULL;
    size_t slen = 0;

    /* IPv4 套接字 */
    if (addr->sa_family == AF_INET) {
        slen = 1 /* [] */ + 15 /* xxx.xxx.xxx.xxx */ + 1 /* : */ + 5 /* port */ + 1 /* \0 */ + 1 /* ] */;
        if ((buf = malloc(slen)) == NULL) {
            printf("malloc error!");
            return NULL;
        }
        struct sockaddr_in *inaddr = (struct sockaddr_in *)addr;
        const char *ip = inet_ntoa(inaddr->sin_addr);
        slen = snprintf(buf, slen, "[%s:%d]", ip, ntohs(inaddr->sin_port));
        buf[slen] = 0;

    /* unix 域套接字 */
    } else if (addr->sa_family == AF_LOCAL) {
        slen = sizeof(struct sockaddr_un);
        if ((buf = malloc(slen)) == NULL) {
            printf("malloc error!");
            return NULL;
        }
        struct sockaddr_un *unaddr = (struct sockaddr_un *)addr;
        snprintf(buf, slen, "[%s]", unaddr->sun_path);
    }
    return buf;
}

char *getPeerInfo(int fd) 
{   
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    if (getpeername(fd, (struct sockaddr *)&peeraddr, &peerlen) < 0) {
        perror("getpeername error!");
        return NULL;
    }
    return getAddrInfo((struct sockaddr *)&peeraddr);
}

char *getSockInfo(int fd) 
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) < 0) {
        perror("getsockname error!");
        return NULL;
    }
    return getAddrInfo((struct sockaddr *)&addr);
}