#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/un.h>
#ifdef HAVE_SOCKADDR_DL_STRUCT
#include <net/if_dl.h>
#endif

/*
 *   linux 

#if __UAPI_DEF_IF_IFREQ
struct ifreq {
#define IFHWADDRLEN 6
    union
    {   
        char    ifrn_name[IFNAMSIZ];        // if name, e.g. "en0" 
    } ifr_ifrn;
    
    union {
        struct  sockaddr ifru_addr;
        struct  sockaddr ifru_dstaddr;
        struct  sockaddr ifru_broadaddr;
        struct  sockaddr ifru_netmask;
        struct  sockaddr ifru_hwaddr;
        short   ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct  ifmap ifru_map;
        char    ifru_slave[IFNAMSIZ];   // Just fits the size 
        char    ifru_newname[IFNAMSIZ];
        void *  ifru_data;
        struct  if_settings ifru_settings;
    } ifr_ifru;
};
#endif 
 */

void printtips(void)
{
    struct ifreq test;
    printf("\ntips : IFNAMSIZ = %d\n"
#ifdef IPV6
                   "sockaddr_in6 = %ld\n"
#endif
#ifdef HAVE_SOCKADDR_DL_STRUCT
                   "sockaddr_dl = %ld\n"
#endif
                   "sockaddr_storage = %ld\n"
                   "sockaddr = %ld\n"
                   "sockaddr_in = %ld\n"
                   "sockaddr_un = %ld\n"
                   "union ifr_ifru = %ld\n", 
                   IFNAMSIZ, 
#ifdef IPV6
                   (long)sizeof(struct sockaddr_in6),
#endif
#ifdef HAVE_SOCKADDR_DL_STRUCT
                   (long)sizeof(struct sockaddr_dl),
#endif
                   (long)sizeof(struct sockaddr_storage),
                   (long)sizeof(struct sockaddr),
                   (long)sizeof(struct sockaddr_in),
                   (long)sizeof(struct sockaddr_un),
                   (long)sizeof(test.ifr_ifru));
#ifdef __linux__
#include <linux/if.h>
    printf("struct  ifmap = %ld\n", (long)sizeof(struct ifmap));
    printf("struct  if_settings = %ld\n\n", (long)sizeof(struct if_settings));
#else
    printf("\n");
#endif
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <how many sizefof(struct ifreq) ?>\n", argv[0]);
        printtips();
        exit(1);
    }
    printtips();

    const int n = atoi(argv[1]);

    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    int len = n;
    char buf[len];
    struct ifconf ifc;
    ifc.ifc_len = len;
    ifc.ifc_buf = buf;

    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl error!");
        exit(1);
    }

    printf("memory size from kernel to user : %d\n", ifc.ifc_len);

    struct ifreq *ifr = (struct ifreq *)buf;
    printf("name : %s\n", ifr->ifr_name);

    int family = ifr->ifr_addr.sa_family;

    if (family == AF_INET) {
        printf("sa_family = AF_INET\n");
        struct sockaddr_in *addr = (struct sockaddr_in *)&ifr->ifr_addr;
        printf("ip : %s\n", inet_ntoa(addr->sin_addr));
    } 
#ifdef HAVE_SOCKADDR_DL_STRUCT
    else if (family == AF_LINK) {
        printf("sa_family = AF_LINK\n");
    } 
#endif
    else {
        printf("%d\n", ifr->ifr_addr.sa_family);
    }

    return 0;
}