#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int ip_2_num(int family, const char *strptr, void *addrptr) {
    struct in_addr ival;
    int counter = 0;
    char *p = (char *)&ival, *dot = NULL, *seg = (char *)strptr;

    /* 只支持 IPv4 哈 */
    if (family == AF_INET) {
        /* 对于 aaa.bbb.ccc.ddd 的 IPv4 地址，先处理 aaa.bbb.ccc.  */
        while (counter < 3 && (dot = strchr(seg, '.')) != NULL) {
            
            /* 转换成大端存储，因此把高位放在低地址 */
            p[counter++] = strtol(seg, NULL, 10);

            /* 遍历字符串 */
            seg = dot + 1;
        }

        /* 输入的 IPv4 地址格式不对 */
        if (counter != 3) {
            errno = EINVAL;
            return -1;
        }

        /* 最后处理 ddd */
        p[counter] = strtol(seg, NULL, 10);
        memmove(addrptr, &ival, sizeof(struct in_addr));
        return 0;
    } 
    errno = EAFNOSUPPORT; 
    return -1;
}

const char *num_2_ip(int family, void *addrptr, char *ipbuf, size_t ipbuflen) {
    if (family == AF_INET) {
        unsigned char *p = (unsigned char *)addrptr;

        /* INET_ADDRSTRLEN 16  INET6_ADDRSTRLEN 48 */
        char buf[INET_ADDRSTRLEN];

        /* 把 addrptr 看成按大端存储 4 字节，因此取出字节最低位作为字符串的最右边  */
        snprintf(buf, sizeof(buf), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        if (strlen(buf) > ipbuflen) {
            errno = ENOSPC;
            return NULL;
        }
        strcpy(ipbuf, buf);
        return ipbuf;
    }
    errno = EAFNOSUPPORT;
    return NULL;
}

int main(int argc, char const *argv[])
{
    const char *ip = "119.29.207.157";

    struct in_addr addr;
    /*
     * 字符串转数字
     */
    if (ip_2_num(AF_INET, ip, &addr) < 0) 
        printf("ip_2_num transform fail!\n");
    else {

        /*
         * 数字转字符串
         */
        char ipbuf[INET_ADDRSTRLEN];
        if (num_2_ip(AF_INET, &addr, ipbuf, sizeof(ipbuf)) == NULL) 
            printf("num_2_ip transform fail!\n");
        else 
            printf("ip_2_num transform succ, ip str value = %s\n", ipbuf);
    }

    return 0;
}