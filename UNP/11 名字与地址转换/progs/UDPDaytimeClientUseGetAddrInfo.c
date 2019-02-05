#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;

    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (sendto(fd, "", 1, 0, res->ai_addr, res->ai_addrlen) < 0) {
        perror("sendto error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    socklen_t svrlen = sizeof(svraddr);
    char buf[1024];
    ssize_t nrecv = 0;
    if ((nrecv = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&svraddr, &svrlen)) < 0) {
        perror("recvfrom error!");
        exit(1);
    }

    char *addrInfo = getAddrInfo(&svraddr);

    struct sockaddr_in *ptr = (struct sockaddr_in *)res->ai_addr;

    if (svrlen != res->ai_addrlen || 
        svraddr.sin_port != ptr->sin_port || 
        svraddr.sin_addr.s_addr != ptr->sin_addr.s_addr) {
        printf("recvfrom %zd bytes from %s (ignore)\n", nrecv, addrInfo);
        free(addrInfo);
        exit(0);
    }
    buf[nrecv] = 0;
    printf("%s\n", buf);

    return 0;
}