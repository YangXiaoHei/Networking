#include "InterfaceTool_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <net/route.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <string.h>
#include <net/if_dl.h>

#define ROUNDUP(a, size) (((a) & ((size) - 1)) ? (1 + ((a) | ((size) - 1))) : (a))
#define NEXT_SA(sa) sa = (struct sockaddr *)((char *)sa + (sa->sa_len ? ROUNDUP(sa->sa_len, sizeof(long)) : sizeof(long)))

int min(int a, int b)
{
    return a < b ? a : b;
}

void getaddrs_info(int addrs, struct sockaddr *sa, struct sockaddr **rti_info)
{
    for (int i = 0; i < RTAX_MAX; i++) {
        if (addrs & (1 << i)) {
            rti_info[i] = sa;
            NEXT_SA(sa);
        } else
            rti_info[i] = NULL;
    }
}

struct ifi_info *get_ifi_info(int family, int doaliaes)
{
    /* 使用 sysctl 获取接口列表 */
    char *buf = NULL;
    int mib[6];
    bzero(mib, sizeof(mib));
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = family;
    mib[4] = NET_RT_IFLIST;
    size_t len = 0;
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        exit(1);
    }
    if ((buf = calloc(1, len)) == NULL) {
        printf("calloc error!");
        exit(1);
    }
    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        free(buf);
        exit(1);
    }

    struct ifi_info *ifihead = NULL, **ifipnext = &ifihead;

    struct sockaddr *rti_info[RTAX_MAX];

    char *next = NULL;
    struct if_msghdr *ifm = (struct if_msghdr *)buf;
    char *lim = buf + len;
    struct ifi_info *ifi = NULL;
    for (next = buf; next < lim; next += ifm->ifm_msglen) {
        
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type == RTM_IFINFO) {
            if ((ifm->ifm_flags & IFF_UP) == 0)
                continue;
    
            struct sockaddr *sa = (struct sockaddr *)(ifm + 1);
            getaddrs_info(ifm->ifm_addrs, sa, rti_info);

            if ((sa = rti_info[RTAX_IFP]) != NULL) {
                if ((ifi = calloc(1, sizeof(struct ifi_info))) == NULL) {
                    printf("calloc error!");
                    exit(1);
                }
                *ifipnext = ifi;
                ifipnext = &ifi->ifi_next;
                ifi->ifi_flags = ifm->ifm_flags;

                if (sa->sa_family == AF_LINK) {
                    struct sockaddr_dl *dl = (struct sockaddr_dl *)sa;
                    ifi->ifi_index = dl->sdl_index;
                    if (dl->sdl_nlen)
                        strncpy(ifi->ifi_name, dl->sdl_data, min(IFI_NAME, dl->sdl_nlen));
                    else
                        snprintf(ifi->ifi_name, IFI_NAME, "index %d", dl->sdl_index);
                    if ((ifi->ifi_hlen = dl->sdl_alen) > 0) 
                        memmove(ifi->ifi_haddr, LLADDR(dl), min(IFI_HADDR, dl->sdl_alen));
                }
            }
        } else if (ifm->ifm_type == RTM_NEWADDR) {
            if (ifi->ifi_addr) {
                if (doaliaes == 0)
                    continue;

                struct ifi_info *ifisave = ifi;
                if ((ifi = calloc(1, sizeof(struct ifi_info))) == NULL) {
                    printf("calloc error!");
                    exit(1);
                }
                *ifipnext = ifi;
                ifipnext = &ifi->ifi_next;
                ifi->ifi_flags = ifisave->ifi_flags;
                ifi->ifi_index = ifisave->ifi_index;
                ifi->ifi_hlen = ifisave->ifi_hlen;
                memmove(ifi->ifi_name, ifisave->ifi_name, IFI_NAME);
                memmove(ifi->ifi_haddr, ifisave->ifi_haddr, IFI_HADDR);
             }

            struct ifa_msghdr *ifam = (struct ifa_msghdr *)next;
            struct sockaddr *sa = (struct sockaddr *)(ifam + 1);
            getaddrs_info(ifam->ifam_addrs, sa, rti_info);

            if ((sa = rti_info[RTAX_IFA]) != NULL) {
                if ((ifi->ifi_addr = calloc(1, sa->sa_len)) == NULL) {
                    printf("calloc error!");
                    exit(1);
                }
                memmove(ifi->ifi_addr, sa, sa->sa_len);
            }
            if ((sa = rti_info[RTAX_BRD]) != NULL && (ifi->ifi_flags & IFF_BROADCAST)) {
                if ((ifi->ifi_brdaddr = calloc(1, sa->sa_len)) == NULL) {
                    printf("calloc error!");
                    exit(1);
                }
                memmove(ifi->ifi_brdaddr, sa, sa->sa_len);
            }
            if ((sa = rti_info[RTAX_BRD]) != NULL && (ifi->ifi_flags & IFF_POINTOPOINT)) {
                if ((ifi->ifi_dstaddr = calloc(1, sa->sa_len)) == NULL) {
                    printf("calloc error!");
                    exit(1);
                }
                memmove(ifi->ifi_dstaddr, sa, sa->sa_len);
            }
        } else {
            printf("unexpected message type %d\n", ifm->ifm_type);
            exit(1);
        }
    } 
    return ifihead;
}
struct iff_info *Get_ifi_info(int family, int doaliaes)
{
    return NULL;
}
void free_ifi_info(struct ifi_info *tofree)
{

}
