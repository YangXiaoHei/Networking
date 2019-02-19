#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <ip>\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    struct arpreq req;
    memcpy(&req.arp_pa, &addr, sizeof(struct sockaddr_in));

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    if (ioctl(fd, SIOCGARP, &req) < 0) {
        perror("ioctl error!");
        exit(1);
    }

    char *ptr = &req.arp_pa.sa_data[0];
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);

    return 0;
}