#include "config.h"
#include "InterfaceTool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#ifdef HAVE_SOCKADDR_DL_STRUCT
#include <net/if_dl.h>
#endif

#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>

static int max(int a, int b) { return a > b ? a : b; }

struct ifi_info *get_ifi_info(int family, int doaliases)
{
    /* 创建套接字描述符 */
    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    /* 分配缓冲区 */
    struct ifconf ifc;
    char *buf = NULL;
    int len = 100 * sizeof(struct ifreq);
    int lastlen = 0;
    for (;;) {
        if ((buf = malloc(len)) == NULL) {
            printf("malloc error! %d\n", len);
            exit(1);
        }
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        /*
         * 如果是分配缓冲区过小会报错的系统，那么理应一开始就报错，
         * 一直到缓冲区足够大，才不报错。此时 lastlen 才第一次被赋非 0 值。
         * 因此假如进去了 < 0 分支，若不是缓冲区过小的问题，退出程序。若是缓冲区
         * 过小的问题，并且 lastlen 已经被赋值过，这种情况是不可能出现的（内核没有 bug 的话）
         * 
         */
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                perror("ioctl error!");
                exit(1);
            }
            // errno == EINVAL && lastlen == 0
            // 分配缓冲区过小，并且一直在往上涨进行尝试
        } else {
            /*
             * 即使第一次进入到该分支，也不意味着缓冲区足够大到装下所有的 struct ifreq 结构
             * 因此还需要再涨一些缓冲区大小，再看看是否还是那么多，那就说明只有那么多了
             */
            if (ifc.ifc_len == lastlen)
                break;
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);
        free(buf);
    }

    /* 缓冲区足够大，足以容纳所有的 struct ifreq 结构 */
    printf("final memory alloc size = %ld, kernel tell = %ld\n", (long)len, (long)ifc.ifc_len);

    struct ifi_info *ifihead = NULL, **ifipnext = &ifihead;
    char lastname[IFI_NAME];
    lastname[0] = 0;
    char *sdlname = NULL, *haddr = NULL;
    int idx = -1, hlen = 0;

    char *ptr = NULL;
    struct ifreq *ifr = NULL;
    for (ptr = buf; ptr < buf + ifc.ifc_len;) {
        ifr = (struct ifreq *)ptr;

#ifdef HAVE_SOCKADDR_SA_LEN
        len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
        switch (ifr->ifr_addr.sa_family) {
#ifdef IPV6
            case AF_INET6 : 
                len = sizeof(struct sockaddr_in6);
                break;
#endif /* IPV6 */
            case AF_INET : 
            default : {
                struct ifreq test;
                /*
                 * 在 UNPv3 版书上，作者写的是 len = sizeof(struct sockaddr);
                 * 因为在作者写书时用的系统中，struct ifreq 结构如下

                   struct ifreq {
                        char ifr_name[IFNAMESIZ];
                        union {
                            struct sockaddr ifru_addr;
                            ...
                            short ifru_flags;
                            int ifru_metric;
                            caddr_t ifru_data;
                        } ifr_ifru;
                   }

                   结构体中的 union 的尺寸由 struct sockaddr 决定（因为它最长）
                   于是，作者把 sizeof(struct sockaddr) 当作 union 的长度直接用了。
                   然而，现在的 linux 源码中，struct ifreq 结构如下

                   struct ifreq {
                        union
                        {   
                            char    ifrn_name[IFNAMSIZ];        
                        } ifr_ifrn;
                        
                        union {
                            struct  sockaddr ifru_addr;
                            ...
                            short   ifru_flags;
                            int ifru_ivalue;
                            int ifru_mtu;
                            struct  ifmap ifru_map;
                            char    ifru_slave[IFNAMSIZ];   
                            char    ifru_newname[IFNAMSIZ];
                            void *  ifru_data;
                            struct  if_settings ifru_settings;
                        } ifr_ifru;
                    };

                    其中这个 union 的尺寸已经不是 sizeof(struct sockaddr) 了，最长的类型是
                    struct ifmap，而 sizeof(struct ifmap) 为 24，
                    因此，若当前指向一个 IPv4 套接字地址的 struct ifreq，在寻找下一个缓冲区中的 struct ifreq 时，
                    指针本来应该偏移 24 字节，而 UNPv3 版的代码只偏移了 16 字节。

                 */
                len = sizeof(test.ifr_ifru);  /* 能正确工作的修正，由 GNU/Linux 4.15.0-42-generic x86_64 测试  */
            } break;
        }
#endif /* HAVE_SOCKADDR_SA_LEN */

        ptr += sizeof(ifr->ifr_name) + len;

#ifdef HAVE_SOCKADDR_DL_STRUCT
        if (ifr->ifr_addr.sa_family == AF_LINK) {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)&ifr->ifr_addr;
            sdlname = ifr->ifr_name;
            idx = sdl->sdl_index;
            haddr = sdl->sdl_data + sdl->sdl_nlen;
            hlen = sdl->sdl_alen;
        }
#endif /* HAVE_SOCKADDR_DL_STRUCT */

        if (ifr->ifr_addr.sa_family != family) {
            printf("(ignored) family = [%d] name = %s\n", ifr->ifr_addr.sa_family, ifr->ifr_name);
            continue;
        }

        int myflags = 0;
        char *cptr = NULL;
        if ((cptr = strchr(ifr->ifr_name, ':')) == NULL)
            cptr = 0;
        if (strncmp(lastname, ifr->ifr_name, IFI_NAME) == 0)
            if (!doaliases)
                continue;
        myflags = IFI_ALIAS;

        memcpy(lastname, ifr->ifr_name, IFI_NAME);

        struct ifreq ifrcopy = *ifr;
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0) {
            perror("ioctl error!");
            exit(1);
        }

        int flags = ifrcopy.ifr_flags;
        if ((flags & IFF_UP) == 0)
            continue;

        struct ifi_info *ifi = NULL;
        if ((ifi = calloc(1, sizeof(struct ifi_info))) == NULL) {
            printf("calloc error!");
            exit(1);
        }
        *ifipnext = ifi;
        ifipnext = &ifi->ifi_next;

        ifi->ifi_flags = flags;
        ifi->ifi_myflags = myflags;

/* 获取 MTU */
#if defined(SIOCGIFMTU) && defined(HAVE_STRUCT_IFREQ_IFR_MTU)
        if (ioctl(sockfd, SIOCGIFMTU, &ifrcopy) < 0) {
            perror("ioctl error!");
            exit(1);
        }
        ifi->ifi_mtu = ifrcopy.ifr_mtu;
#endif
        memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
        ifi->ifi_name[IFI_NAME - 1] = 0;

        if (sdlname == NULL || strcmp(sdlname, ifr->ifr_name) != 0)
            idx = hlen = 0;
        ifi->ifi_index = idx;
        ifi->ifi_hlen = hlen;
        if (ifi->ifi_hlen > IFI_HADDR)
            ifi->ifi_hlen = IFI_HADDR;

        if (hlen)
            memcpy(ifi->ifi_haddr, haddr, ifi->ifi_hlen);

        switch (ifr->ifr_addr.sa_family) {
            case AF_INET : {
                struct sockaddr_in *addr = (struct sockaddr_in *)&ifr->ifr_addr;
                if ((ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in))) == NULL) {
                    perror("calloc error!");
                    exit(1);
                }
                memcpy(ifi->ifi_addr, addr, sizeof(struct sockaddr_in));

/* 广播 */
#ifdef SIOCGIFBRDADDR
                if (flags & IFF_BROADCAST) {
                    if (ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy) < 0) {
                        perror("ioctl error!");
                        exit(1);
                    }
                    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr->ifr_broadaddr;
                    if ((ifi->ifi_brdaddr = calloc(1, sizeof(struct sockaddr_in))) == NULL) {
                        perror("calloc error!");
                        exit(1);
                    }
                    memcpy(ifi->ifi_brdaddr, addr, sizeof(struct sockaddr_in));
                }
#endif

/* 点对点 */
#ifdef SIOCGIFDSTADDR
                if (flags & IFF_POINTOPOINT) {
                    if (ioctl(sockfd, IFF_POINTOPOINT, &ifrcopy) < 0) {
                        perror("ioctl error!");
                        exit(1);
                    }
                    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr->ifr_dstaddr;
                    if ((ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in))) == NULL) {
                        perror("calloc error!");
                        exit(1);
                    }
                    memcpy(ifi->ifi_dstaddr, addr, sizeof(struct sockaddr_in));
                }
#endif
            } break;

            case AF_INET6 : {
                struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&ifr->ifr_addr;
                if ((ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in6))) == NULL) {
                    perror("calloc error!");
                    exit(1);
                }
                memcpy(ifi->ifi_addr, addr, sizeof(struct sockaddr_in6));
#ifdef SIOCGIFDSTADDR
                if (flags & IFF_POINTOPOINT) {
                    if (ioctl(sockfd, IFF_POINTOPOINT, &ifrcopy) < 0) {
                        perror("ioctl error!");
                        exit(1);
                    }
                    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&ifr->ifr_dstaddr;
                    if ((ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in6))) == NULL) {
                        perror("calloc error!");
                        exit(1);
                    }
                    memcpy(ifi->ifi_dstaddr, addr, sizeof(struct sockaddr_in6));
                }
#endif
            } break;
            default :
              break;
        }
    } 
    free(buf);
    return ifihead;
}
struct iff_info *Get_ifi_info(int family, int doaliaes)
{
    return NULL;
}
void free_ifi_info(struct ifi_info *tofree)
{
    struct ifi_info *ifi, *ifinext;
    for (ifi = tofree; ifi != NULL; ifi = ifinext) {
        if (ifi->ifi_addr)
            free(ifi->ifi_addr);
        if (ifi->ifi_brdaddr)
            free(ifi->ifi_brdaddr);
        if (ifi->ifi_dstaddr)
            free(ifi->ifi_dstaddr);
        ifinext = ifi->ifi_next;
        free(ifi);
    }
}