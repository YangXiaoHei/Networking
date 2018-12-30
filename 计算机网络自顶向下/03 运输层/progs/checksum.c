#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char bits[(sizeof(short) << 3) + 1];

short pton(const char *str)
{
    short res = 0;
    size_t len = 0;

    /* 获得数字真实长度 */
    for (int i = 0; i < strlen(str); i++)
        if (str[i] >= '0' && str[i] <= '9')
            len++;

    if (len > 16)
    {
        printf("error\n");
        exit(1);
    }

    for (size_t i = 0, k = 0; i < len; k++)
    {
        int bit = str[k] - '0';
        if (bit != 0 && bit != 1) /* 只要数字，忽略空格和其他字符 */
            continue;
        res |= (bit << (len - i - 1));
        i++;
    }
    return res;
}

char *ntop(short num)
{
    for (int i = (sizeof(num) << 3) - 1, k = 0; i >= 0; i--, k++)
    {
        int bit = (num >> i) & 1;
        bits[k] = bit + '0';
    }
    int size = sizeof(bits) / sizeof(bits[0]);
    bits[size - 1] = 0; /* '\0' */
    return bits;
}

void printp(short num)
{
    int leading_zero = 1;
    for (int i = (sizeof(num) << 3) - 1; i >= 0; i--)
    {
        int bit = (num >> i) & 1;

        if (bit == 1)
            leading_zero = 0;

        if (leading_zero) /* 不打印前导 0 */
            continue;

        printf("%d", bit);
    }
    if (leading_zero)
        printf("0");
    printf("\n");
}

short _check_add(const char *n1, const char *n2)
{
    short num1 = pton(n1);
    short num2 = pton(n2);
    short res = num1 + num2;
    while (((num1 & 0x8000) || (num2 & 0x8000)) && !(res & 0x8000))
    {
        num1 = res;
        num2 = 1;
        res = num1 + num2;
    }
    return res;  /* 只要低位 16 bit */
}

short check_add(int count, ...)
{
    va_list ap;
    va_start(ap, count);

    if (count == 1)
        return pton(va_arg(ap, char *));
    
    short res = pton(va_arg(ap, char *));
    for (int i = 0; i < count - 1; i++)
        res = _check_add(ntop(res), va_arg(ap, char *));

    va_end(ap);
    return ~res;
}

#define _DEBUG_ 0

#if _DEBUG_
#define P2N_TEST_CASE(_str_) printp(pton(_str_))
#else
#define P2N_TEST_CASE(_str_)
#endif

int main(int argc, char const *argv[])
{
    P2N_TEST_CASE("11100011101");
    P2N_TEST_CASE("000000000000");
    P2N_TEST_CASE("111111111111111");
    P2N_TEST_CASE("1010101010101010");
    P2N_TEST_CASE("01010101010110");
    P2N_TEST_CASE("11111100000111");
    P2N_TEST_CASE("000000111111");
    P2N_TEST_CASE("0");
    P2N_TEST_CASE("1");
    P2N_TEST_CASE("1 1 1");
    P2N_TEST_CASE("01 01 01 01 ");
    P2N_TEST_CASE("10 111 0001 101010 111");

    const char *s1 = "0110 0110 0110 0000";
    const char *s2 = "0101 0101 0101 0101";
    const char *s3 = "1000 1111 0000 1100";

    char *s3_case[] = {
        "1000 1111 0000 1101", 
        "1000 1111 0001 1100", 
        "1000 1111 0100 1100",
        "1000 1011 1000 1100",
        "1000 1111 0000 1100",
        "1011 1001 0000 1100",
        "1100 1101 0000 1100",
        "1000 1111 0000 1100"
    };
    int size = sizeof(s3_case) / sizeof(s3_case[0]);

    short checksum = check_add(3, s1, s2, s3);

    for (int i = 0; i < size; i++)
    {
        if (check_add(4, ntop(checksum), s1, s2, s3_case[i]) != 0)
            printf("%d ---- bit error!\n", i + 1);
        else
            printf("%d ---- no error!\n", i + 1);
    }
    
    return 0;
}