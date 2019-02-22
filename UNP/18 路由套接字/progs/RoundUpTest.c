#include <stdio.h>
#include <stdlib.h>

#define ROUNDUP(a, size) (((a) & ((size) - 1)) ? (1 + ((a) | ((size) - 1))) : (a))

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 20; i++) {
        printf("%d roundup to 8 ---- %d\n",i, ROUNDUP(i, 8));
    }
    printf("\n");
    for (int i = 0; i < 20; i++) {
        printf("%d roundup to 16 ---- %d\n",i, ROUNDUP(i, 16));
    }
    return 0;
}