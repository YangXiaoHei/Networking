#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <net/if_dl.h>
#include "config.h"

const char * getMaskInfo(struct sockaddr *sa)
{
#ifndef HAVE_SOCKADDR_SA_LEN
    printf("not supported!\n");
    exit(1);
#endif

    static char str[INET6_ADDRSTRLEN];
    unsigned char *ptr = (unsigned char *)&sa->sa_data[2];
    bzero(&str, sizeof(str));

    if (sa->sa_len == 0)
        return "0.0.0.0";
    else if (sa->sa_len == 5)
        snprintf(str, sizeof(str), "%d.0.0.0", *ptr);
    else if (sa->sa_len == 6)
        snprintf(str, sizeof(str), "%d.%d.0.0", ptr[0], ptr[1]);
    else if (sa->sa_len == 7)
        snprintf(str, sizeof(str), "%d.%d.%d.0", ptr[0], ptr[1], ptr[2]);
    else if (sa->sa_len == 8)
        snprintf(str, sizeof(str), "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
    return str;
}

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
        if (ntohs(inaddr->sin_port) == 0)
            slen = snprintf(buf, slen, "%s", ip);
        else
            slen = snprintf(buf, slen, "[%s:%d]", ip, ntohs(inaddr->sin_port));
        buf[slen] = 0;

    /* unix 域套接字 */
    } else if (addr->sa_family == AF_LOCAL) {
        slen = sizeof(struct sockaddr_storage);
        if ((buf = malloc(slen)) == NULL) {
            printf("malloc error!");
            return NULL;
        }
        struct sockaddr_un *unaddr = (struct sockaddr_un *)addr;
        snprintf(buf, slen, "[%s]", unaddr->sun_path);
    } 
#ifdef HAVE_SOCKADDR_DL_STRUCT
    else if (addr->sa_family == AF_LINK) {
        slen = sizeof(struct sockaddr_storage);
        if ((buf = malloc(slen)) == NULL) {
            printf("malloc error!");
            return NULL;
        }
        struct sockaddr_dl *dladdr = (struct sockaddr_dl *)addr;
        if (dladdr->sdl_alen) {
            unsigned char *ptr = (unsigned char *)dladdr->sdl_data + dladdr->sdl_nlen; 
            snprintf(buf, slen, "[mac %02x:%02x:%02x:%02x:%02x:%02x]", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
        }
    }
#endif /* HAVE_SOCKADDR_DL_STRUCT */
    else {
        printf("unknow sockaddr type = %d\n", addr->sa_family);
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