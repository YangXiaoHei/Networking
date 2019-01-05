#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    /*
     * 在 Mac OSX 上这个值是 0xffffffff
     */
    printf("INADDR_NONE = %x\n", INADDR_NONE);

    const char *ip = "255.255.255.255";

    /*
     * 使用 inet_addr
     * 缺陷 : 无法转换广播地址 255.255.255.255
     * 缺陷 : 只支持 IPv4
     */
    in_addr_t ipaddr = INADDR_NONE;
    if ((ipaddr = inet_addr(ip)) == INADDR_NONE) {
        printf("inet_addr transform fail!\n");
    } else {
        printf("inet_addr transform succ! ip int value = %d\n", ipaddr);
    }

    /*
     * 使用 inet_aton
     * 缺陷 : 只支持 IPv4
     */
    struct in_addr addr;
    if ((inet_aton(ip, &addr)) == 0) {
        printf("inet_aton transform fail!\n");
    } else {
        printf("inet_aton transform succ! ip int value = %x\n", addr.s_addr);
    }

    /*
     * 使用 inet_ntoa
     * 缺陷 : 返回的是静态内存的指针，不可重入
     */
    const char *ipresult = NULL;
    if ((ipresult = inet_ntoa(addr)) == NULL) {
        printf("inet_ntoa transform fail!\n");
    } else {
        printf("inet_ntoa transform succ! ip str value = %s\n", ipresult);
    }

    return 0;
}