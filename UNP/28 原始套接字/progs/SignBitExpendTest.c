#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "../../tool/bin_tool.h"

int main(int argc, char *argv[])
{
    unsigned int value = 0xFFFFFF;
    unsigned short *w = &value;
    w++;
    unsigned short sum = 0;
    sum = *(char *)w; 
    printf("%s\n", u16ToBinaryString(sum, eight));
    
    unsigned int h = 65535 + 65535;
    printf("%s\n", u32ToBinaryString(h, eight));
}
