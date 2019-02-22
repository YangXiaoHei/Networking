#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <sys/sysctl.h>
#include <string.h>

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
        printf("usage : %s <interface index>\n", argv[0]);
        exit(1);
    }

    int mib[6];
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = 0;
    mib[4] = NET_RT_IFLIST;
    mib[5] = 0;

    size_t len = 0;
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        exit(1);
    }

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

    struct if_msghdr *ifm = NULL;
    char *ptr = buf;
    char *lim = buf + len;
    for (ptr = buf; ptr < lim; ptr += ifm->ifm_msglen) {
        ifm = (struct if_msghdr *)ptr;
        if (ifm->ifm_type != RTM_IFINFO) {
            // printf("unknown ifm_type %d\n", ifm->ifm_type);
            continue;
        }

        struct sockaddr *sa = (struct sockaddr *)(ifm + 1);
        get_rtaddrs(ifm->ifm_addrs, sa, rti_info);
        if ((sa = rti_info[RTAX_IFP]) != NULL) {
            if (sa->sa_family != AF_LINK)
                continue;
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;
            if (sdl->sdl_index != atoi(argv[1]))
                continue;

            char name[30];
            bzero(name, sizeof(name));
            /* strncpy 不会自动添加 \0 */
            strncpy(name, sdl->sdl_data, sdl->sdl_nlen);
            printf("interface name = %s\n", name);
        }
    }

    return 0;
}