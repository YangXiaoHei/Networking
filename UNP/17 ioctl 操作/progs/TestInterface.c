#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include "InterfaceTool.h"
#include "AddrTool.h"

int main(int argc, char const *argv[])
{
    struct ifi_info *ifihead = NULL, *ifi = NULL;  
    for (ifi = ifihead = get_ifi_info(AF_INET, 1); ifi != NULL; ifi = ifi->ifi_next) {
        printf("%s: ", ifi->ifi_name);
        if (ifi->ifi_index != 0) {
            printf("(%d) ", ifi->ifi_index);
        }
        printf("< ");
        if (ifi->ifi_flags & IFF_UP)        printf("UP ");
        if (ifi->ifi_flags & IFF_BROADCAST) printf("BCAST ");
        if (ifi->ifi_flags & IFF_MULTICAST) printf("MCAST ");
        if (ifi->ifi_flags & IFF_LOOPBACK)  printf("LOOP ");
        if (ifi->ifi_flags & IFF_POINTOPOINT) printf("P2P ");
        printf(">\n");

        int i = 0;
        unsigned char *ptr = NULL;
        if ((i = ifi->ifi_hlen) > 0) {
            printf("\tether: ");
            ptr = (unsigned char *)ifi->ifi_haddr;
            do {
                printf("%s%x", (i == ifi->ifi_hlen) ? "" : ":", *ptr++);
            } while (--i > 0);
            printf("\n");
        }
        if (ifi->ifi_mtu != 0)
            printf("\tMTU: %d\n", ifi->ifi_mtu);

        struct sockaddr *sa = NULL;
        if ((sa = ifi->ifi_addr) != NULL)
            printf("\tIP addr: %s\n", getAddrInfo((sa)));
        if ((sa = ifi->ifi_brdaddr) != NULL)
            printf("\tbroadcast addr: %s\n", getAddrInfo(sa));
        if ((sa = ifi->ifi_dstaddr) != NULL)
            printf("\tdestination addr: %s\n", getAddrInfo(sa));
    }  
    free(ifihead);
    return 0;
}