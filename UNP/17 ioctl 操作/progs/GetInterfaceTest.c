#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include "config.h"

int main(int argc, char const *argv[])
{
    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct ifconf ifc;

    int lastlen = 0;
    char *buf = NULL;
    int len = 100 * sizeof(struct ifreq);
    for (;;) {
        if ((buf = malloc(len)) == NULL) {
            printf("malloc error!");
            exit(1);
        }
        bzero(buf, len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                perror("ioctl error!");
                exit(1);
            }
        } else {
            if (ifc.ifc_len == lastlen)
                break;
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);
        free(buf);
    }

    printf("%ld bytes\n", (long)ifc.ifc_len);

    char *ptr = NULL;
    struct ifreq ifrcpy;
    for (ptr = buf; ptr < buf + ifc.ifc_len;) {
        struct ifreq *ifr = (struct ifreq *)ptr;

        printf("-------------------\n");
        printf("%s\n", ifr->ifr_name);

        ifrcpy = *ifr;
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifrcpy) < 0) {
            perror("ioctl error!");
            exit(1);
        }
        int flags = ifrcpy.ifr_flags;
        if (flags & IFF_UP)
            printf("\tup\n");
        if (flags & IFF_BROADCAST)
            printf("\tBCAST\n");
        if (flags & IFF_MULTICAST)
            printf("\tMCAST\n");
        if (flags & IFF_LOOPBACK)
            printf("\tLOOP\n");
        if (flags & IFF_POINTOPOINT)
            printf("\tP2P\n");

#if defined(SIOCGIFMTU) && defined(HAVE_STRUCT_IFREQ_IFR_MTU)
        if (ioctl(sockfd, SIOCGIFMTU, &ifrcpy) < 0) {
            perror("iotcl error!");
            exit(1);
        }
        printf("\tmtu : %ld\n", (long)ifrcpy.ifr_mtu);
#endif

        switch (ifr->ifr_addr.sa_family) {
            case AF_INET : 
#ifdef SIOCGIFBRDADDR
            if (flags & IFF_BROADCAST) {
                if (ioctl(sockfd, SIOCGIFBRDADDR, &ifrcpy) < 0) {
                    perror("ioctl error!");
                    exit(1);
                }
                struct sockaddr_in *addr = (struct sockaddr_in *)&ifrcpy.ifr_addr;
                char *ip = inet_ntoa(addr->sin_addr);
                printf("\tbroadcast ip : %s\n", ip);
            }
#endif
            break;

#ifdef SIOCGIFDSTADDR
            if (flags & IFF_POINTOPOINT) {
                if (ioctl(sockfd, SIOCGIFDSTADDR, &ifrcpy) < 0) {
                    perror("ioctl error!");
                    exit(1);
                }
                struct sockaddr_in *addr = (struct sockaddr_in *)&ifrcpy.ifr_addr;
                char *ip = inet_ntoa(addr->sin_addr);
                printf("\tp2p ip : %s\n", ip);
            }
#endif
            break;
            default : 
            break;
        }

        printf("--------------------\n");

#ifdef HAVE_SOCKADDR_SA_LEN
        len = ifr->ifr_addr.sa_len;
#else
        switch (ifr->ifr_addr.sa_family) {
#ifdef IPV6 
            case AF_INET6 :
                len = sizeof(struct sockaddr_in6);
                break;
#endif
            case AF_INET : 
                len = sizeof(struct sockaddr_in);
                break;

            default :
                len = sizeof(struct sockaddr);
                break;
        }
#endif
        ptr += sizeof(ifr->ifr_name) + len;
    } 

    return 0;
}