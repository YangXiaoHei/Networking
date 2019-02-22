#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/route.h>
#include <string.h>
#include "../../tool/AddrTool.h"

#define BUFLEN (sizeof(struct rt_msghdr) + 512)
#define ROUNDUP(a, size) (((a) & ((size) - 1)) ? (1 + ((a) | ((size) - 1))) : (a))
#define NEXT_SA(sa) sa = (struct sockaddr *)((char *)sa + (sa->sa_len ? ROUNDUP(sa->sa_len, sizeof(long)) : sizeof(long)))

void get_rtaddrs(int addrs, struct sockaddr *sa, struct sockaddr **rti_info)
{
    for (int i = 0; i < RTAX_MAX; i++) {
        if (addrs & (1 << i)) {
            rti_info[i] = sa;
            NEXT_SA(sa);
        } else
            rti_info[i] = NULL;
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <dest ip addr>\n", argv[0]);
        exit(1);
    }

    int sockfd = -1;
    if ((sockfd = socket(AF_ROUTE, SOCK_RAW, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

#ifdef OFF
    int on = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_USELOOPBACK, &on, sizeof(on)) < 0) {
        perror("setsockopt error1");
        exit(1);
    }
#endif

    char *buf = NULL;
    if ((buf = calloc(1, BUFLEN)) == NULL) {
        printf("calloc error!");
        exit(1);
    }

    struct sockaddr *rti_info[RTAX_MAX];

    pid_t pid;

    struct rt_msghdr *rtm = (struct rt_msghdr *)buf;
    struct sockaddr_in *inptr = (struct sockaddr_in *)(rtm + 1);
    inptr->sin_family = AF_INET;
    inptr->sin_len = sizeof(struct sockaddr_in);
    inptr->sin_addr.s_addr = inet_addr(argv[1]);

    rtm->rtm_msglen = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_in);
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_type = RTM_GET;
    rtm->rtm_addrs = RTA_DST;
    rtm->rtm_pid = pid = getpid();
    rtm->rtm_seq = 9999;

    if (write(sockfd, rtm, rtm->rtm_msglen) != rtm->rtm_msglen) {
        perror("write error!");
        exit(1);
    }

    ssize_t n;
    do {
        if ((n = read(sockfd, rtm, BUFLEN)) < 0) {
            perror("read error!");
            exit(1);
        } 
    } while (rtm->rtm_type != RTM_GET || rtm->rtm_seq != 9999 || rtm->rtm_pid != pid);

    rtm = (struct rt_msghdr *)buf;
    struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
    get_rtaddrs(rtm->rtm_addrs, sa, rti_info);

    char *tofree = NULL;
    if ((sa = rti_info[RTAX_DST]) != NULL) {
        printf("dest : %s\n", tofree = getAddrInfo(rti_info[RTAX_DST]));
    }

    if ((sa = rti_info[RTAX_GATEWAY]) != NULL) {
        printf("gateway : %s\n", tofree = getAddrInfo(rti_info[RTAX_GATEWAY]));
    }

    if ((sa = rti_info[RTAX_NETMASK]) != NULL) {
        printf("netmask : %s\n", getMaskInfo(rti_info[RTAX_NETMASK]));
    }

    if ((sa = rti_info[RTAX_GENMASK]) != NULL) {
        printf("genmask : %s\n", getMaskInfo(rti_info[RTAX_GENMASK]));
    }


    return 0;
}
