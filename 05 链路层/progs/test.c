#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "checksum.h"
#include "bin_tool.h"

#define SWAP(_x_) (_x_ = ((_x_ << 8) & 0xFF00) | ((_x_ >> 8) & 0xFF))

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <#msg>", argv[0]);
        exit(1);
    }    

    printAsciiString(argv[1]);

    int len = strlen(argv[1]);
    unsigned short cks = checksum((unsigned short *)argv[1], len);
    SWAP(cks);
    printf("checksum of %s is 【%d】\n",argv[1], cks);
    printf("binary string of cks is 【%s】\n", u16ToBinaryString(cks, EN_GROUP_IN_EIGHTS));
    
    return 0;
}