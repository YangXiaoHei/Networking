#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sysctl.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <netinet/udp_var.h>

int main(int argc, char const *argv[])
{
    int mib[4], val;
    size_t len;
    mib[0] = CTL_NET;
    mib[1] = AF_INET;
    mib[2] = IPPROTO_UDP;
    mib[3] = UDPCTL_CHECKSUM;
    len = sizeof(val);

    if (sysctl(mib, 4, &val, &len, NULL, 0) < 0) {
        perror("sysctl error!");
        exit(1);
    }
    printf("udp checksum flag : %d\n", val);
    return 0;
}