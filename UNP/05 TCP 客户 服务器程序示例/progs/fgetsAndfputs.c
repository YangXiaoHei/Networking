#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printAllAscii(char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%-4d", buf[i]);
    }
}

int main(int argc, char const *argv[])
{
    /*
     * fgets 会自动添加 \0 吗
     * ✅ 会
     *
     *
     * fgets 一直读到 \n 才停止吗？
     * ❌ 不是，如果缓冲区大小不够，那么只读 bufsize - 1 个字符，然后在最后一个坑填 \0
     */

    printf("\\n = %d\n", '\n');
    printf("a = %d\n", 'a');

    // char buf[4];
    // memset(buf, 'a', 4);
    // fgets(buf, sizeof(buf), stdin);
    // printAllAscii(buf, sizeof(buf));

    /*
     * 如果字符串不以 \0 结尾会如何？
     * 
     * fputs 会 GG
     */
    // char buf2[4];
    // memset(buf2, 'a', 4);
    // memcpy(buf2, "123", 3);
    // fputs(buf2, stdout);

    /*
     * 如果字符串以 \n 结尾会如何？
     * fputs 会 GG
     */
    char buf3[4];
    memset(buf3, 'a', 4);
    memcpy(buf3, "12\n", 3);
    fputs(buf3, stdout);

    /*
     * 结论 fputs 会一直读到 \0
     */


    return 0;
}