#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "checksum.h"
#include "bin_tool.h"

#define SWAP(_x_) (_x_ = ((_x_ << 8) & 0xFF00) | ((_x_ >> 8) & 0xFF))

int main(int argc, char const *argv[])
{
    // char data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // char *data = "BCDEFGHIJK";
    char *data = "bcdefghijk";
    printAsciiString(data);
    unsigned short cks = checksum((unsigned short *)data, 10);
    SWAP(cks);
    printf("checksum is 【%d】\n", cks);
    printf("binary string of cks is 【%s】\n", u16ToBinaryString(cks, EN_GROUP_IN_EIGHTS));
    return 0;
}