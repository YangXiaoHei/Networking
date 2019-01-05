#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "tool/bin_tool.h"

int inet_pton_loose(int family, const char *ipaddr, void *addrptr) 
{
    int retcode = -1;
    if (family == AF_INET) {
        retcode = inet_pton(family, ipaddr, addrptr);
        if (retcode == 0) {
            retcode = inet_aton(ipaddr, (struct in_addr *)addrptr);
        }
    } else if (family == AF_INET6) {
        retcode = inet_pton(family, ipaddr, addrptr);
        if (retcode == 0) {
            char *byte_ptr = (char *)addrptr;
            byte_ptr += 12;
            retcode = inet_aton(ipaddr, (struct in_addr *)byte_ptr);
            byte_ptr[-1] = 0xFF;
            byte_ptr[-2] = 0XFF;
        } 
    } else
        errno = EAFNOSUPPORT;
    return retcode;
}

int main(int argc, char const *argv[])
{
    const char *ip = "119.29.207.157";

    struct in6_addr addr;
    if (inet_pton_loose(AF_INET6, ip, &addr) < 0) {
        perror("fail!\n");
        exit(1);
    }

    unsigned long high64 = 0;
    unsigned long low64 = 0;
    memcpy(&high64, (char *)(((char *)&addr.s6_addr) + 8), 8);
    memcpy(&low64, (char *)&addr.s6_addr, 8);
    printf("%s ", u64ToBinaryString(high64, EN_GROUP_IN_EIGHTS));
    printf("%s\n", u64ToBinaryString(low64, EN_GROUP_IN_EIGHTS));

    return 0;
}