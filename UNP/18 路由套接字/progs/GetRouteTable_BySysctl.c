#include <stdio.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include "../../tool/AddrTool.h"

#define ROUNDUP(a, size) ((a) & ((size) - 1) ? (1 + (a) | ((size) - 1)) : (a))
#define NEXT_SA(sa) sa = (struct sockaddr *)((char *)sa + (sa->sa_len ? ROUNDUP(sa->sa_len, sizeof(long)) : sizeof(long)))

void getrti_addrs(int addrs, struct sockaddr *sa, struct sockaddr **rti_info)
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
    int mib[6];
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_DUMP;
    size_t len = 0;
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        exit(1);
    }

    printf("all size = %zd\n", len);

    char *buf = NULL;
    if ((buf = malloc(len)) == NULL) {
        printf("malloc error!");
        exit(1);
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        exit(1);
    }

    struct sockaddr *rti_info[RTAX_MAX];

    char *ptr = buf;
    char *lim = buf + len;

    struct rt_msghdr *rtm;
    for (ptr = buf; ptr < lim; ptr += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)ptr;
        if (rtm->rtm_type != RTM_GET) {
            printf("unknow rtm_type = %d\n", rtm->rtm_type);
            continue;
        }
        struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
        getrti_addrs(rtm->rtm_addrs, sa, rti_info);

        if ((sa = rti_info[RTAX_DST]) != NULL) {
            printf("%s", getAddrInfo(sa));

            if ((sa = rti_info[RTAX_GATEWAY]) != NULL) {
                printf("\t%s", getAddrInfo(sa));
            } else
                printf("\t");

            if ((sa = rti_info[RTAX_NETMASK]) != NULL) {
                printf("\t%s", getMaskInfo(sa));
            } else 
                printf("\t");
            printf("\n");
        }
    }























    return 0;
}