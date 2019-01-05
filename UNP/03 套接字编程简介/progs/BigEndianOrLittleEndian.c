#include <stdio.h>

int main(int argc, char const *argv[])
{
    union {
        short s;
        char c[sizeof(short)];
    } u;
    u.s = 0x0102;
    if (sizeof(short) == 2) {
        if (u.c[0] == 2 && u.c[1] == 1) 
            printf("little-endian\n");
        else if (u.c[0] == 1 && u.c[1] == 2)
            printf("big-endian\n");
        else
            printf("unknown\n");
    } else
        printf("sizeof(short) == %lu\n", sizeof(short));
    return 0;
}