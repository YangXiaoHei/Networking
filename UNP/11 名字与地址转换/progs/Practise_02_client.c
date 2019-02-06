#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *hostname = argv[1];
    const char *service = argv[2];

    struct in_addr **pptr;
    struct in_addr *inetaddrp[2];

    struct in_addr inetaddr;
    struct hostent *ent = NULL;
    if ((ent = gethostbyname(hostname)) == NULL) {
        if (inet_aton(hostname, &inetaddr) == 0) {
            perror("inet_aton error!");
            exit(1);
        } else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }   
    } else {
        pptr = (struct in_addr **)ent->h_addr_list;
    }

    struct servent *svrent = NULL;
    if ((svrent = getservbyname(service, "tcp")) == NULL) {
        perror("getservbyname error!");
        exit(1);
    }

    int fd = -1;
    struct sockaddr_in svraddr;
    for (; *pptr != NULL; pptr++) {
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket fd create fail!");
            continue;
        }

        bzero(&svraddr, sizeof(svraddr));
        svraddr.sin_family = AF_INET;
        memcpy(&svraddr.sin_addr, *pptr, sizeof(struct in_addr));
        svraddr.sin_port = svrent->s_port;

        if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            perror("connect error!");
            continue;
        }

        char *addrInfo = getAddrInfo(&svraddr);
        printf("connect to %s succ!\n", addrInfo);
        free(addrInfo);
        break;
    }

    char buf[1024];
    ssize_t nread = 0;
    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        buf[nread] = 0;
        printf("%s\n", buf);
    }

    return 0;
}