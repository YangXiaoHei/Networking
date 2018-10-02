#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tool.h"
#include "log.h"

int main(int argc, char const *argv[])
{
    char msg[8];
    msg[0] = 0x23;
    msg[1] = 0xC5;

    msg[2] = 0x67;
    msg[3] = 0x89;

    msg[4] = 0x44;
    msg[5] = 0x96;

    msg[6] = 0x78;
    msg[7] = 0x98;

    // 1100 0101 0010 0011
    // 1000 1001 0110 0111
    // --------------------
    // 0100 1110 1000 1011

    // 0100 1110 1000 1011
    // 1001 0110 0100 0100
    // -------------------
    // 1110 0100 1100 1111

    // 1110 0100 1100 1111
    // 1001 1000 0111 1000
    // -------------------
    // 0111 1101 0100 1000 

    short res1 = calculate_checksum(msg, sizeof(msg));
    LOG("%s", to_bin(res1));
    
    // 1000 0010 1011 0111

    return 0;
}