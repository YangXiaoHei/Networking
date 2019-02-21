#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "../../tool/bin_tool.h"

int main(int argc, char const *argv[])
{
    const char *ip = "119.29.207.157";

    /*
     * inet_aton 这些函数默认了 in_addr 中存储的是按照大端序列存放的字节
     */
    struct in_addr addr;
    inet_aton(ip, &addr);

    printf("%s\n", u32ToBinaryString(addr.s_addr, EN_GROUP_IN_EIGHTS));

    char tmp[16];
    unsigned char *p = (unsigned char *)&addr;
    snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    printf("%s\n", tmp);


    return 0;
}