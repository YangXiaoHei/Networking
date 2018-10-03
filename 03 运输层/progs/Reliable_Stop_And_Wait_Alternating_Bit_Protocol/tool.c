#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define _DEBUG_ 0

void init_rand()
{
    srand((unsigned)time(NULL));
}

unsigned long curtime_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

short _calculate_checksum(short num1, short num2)
{
    short res = num1 + num2;
    while (((num1 & 0x8000) || (num2 & 0x8000)) && !(res & 0x8000))
    {
        num1 = res;
        num2 = 1;
        res = num1 + num2;
    }
    return res; 
}

short calculate_checksum(char *data, size_t datalen)
{
    short *num1 = (short *)data;
    short res = *num1;
    for (int i = 2; i < datalen; i += 2) 
        res = _calculate_checksum(res, *(++num1));
    return ~res;
}

char *to_bin(unsigned long num)
{
    static char bits[(sizeof(long) << 3) + 1];
    int leading_zero = 1, k = 0;
    int bit;
    for (int i = (sizeof(num) << 3) - 1; i >= 0; i--)
    {
        if ((bit = (num >> i) & 1) != 0)
            leading_zero = 0;

        if (leading_zero)
            continue;

        bits[k++] = bit + '0';
    }
    if (k == 0)
        bits[k++] = '0';
    bits[k] = 0; /* '\0' */
    return bits;
}

int yh_random(int n, int m)
{
    return rand() % (m - n + 1) + n; 
}

int probability(double pro)
{
    return (rand() * 1.0 / RAND_MAX) <= pro;
}

void gen_one_bit_error(char *msg, size_t msg_len)
{
    int index = yh_random(0, (msg_len << 3) - 1);
    int row = index / 8;
    int column = index % 8;
#if _DEBUG_
    LOG("将第 %d 个比特位反转，反转前 %15s", column, to_bin(msg[row]));
#endif
    msg[row] ^= (1 << (7 - column));
#if _DEBUG_
    LOG("将第 %d 个比特位反转，反转后 %15s", column, to_bin(msg[row]));
#endif
}
