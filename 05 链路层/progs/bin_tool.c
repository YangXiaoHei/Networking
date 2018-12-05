#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static unsigned char _buf[128];

typedef unsigned long      TUINT64;
typedef unsigned           TUINT32;
typedef long long          TINT128;
typedef long               TINT64;
typedef int                TINT32;
typedef unsigned short     TUINT16;
typedef unsigned char      TUINT8;
typedef short              TINT16;
typedef char               TINT8;

typedef enum EN_BIN_TOOL_TYPE {
    EN_IGNORE_LEADING_ZERO  = 1 << 0,
    EN_GROUP_IN_FOURS       = 1 << 1,
    EN_GROUP_IN_EIGHTS      = 1 << 2
} EN_BIN_TOOL_TYPE;

const char *u64ToBinaryString(TUINT64 a, EN_BIN_TOOL_TYPE type)
{
    int i = 63, j = 0, len = 0;
    int counter = 0;
    if (type & EN_IGNORE_LEADING_ZERO) 
        while(i >= 0 && ((a >> i) & 1) == 0) i--;   
    while(i >= 0) 
        _buf[j++] = ((a >> i--) & 1) + '0';
    _buf[j] = 0;
    // if (type & EN_GROUP_IN_EIGHTS) {
    //     while (++counter < j) {
    //         if (counter % 8 == 0) {

    //         }
    //     }
    // }

    return _buf;
}

TUINT64 binaryStringToU64(const char *binaryStr)
{
    unsigned long long reg = 0;
    unsigned char c;
    int i, j, k;
    size_t len = strlen(binaryStr);
    for (i = 0, k = 0; k < len; k++) {
        c = binaryStr[len - k - 1];
        j = c - '0';
        if (c == ' ') continue;
        if (j != 0 && j != 1) 
            goto invalid;
        reg |= (j << i++);
        if (i >= 64)
            goto overflow;
    }
    return reg;
invalid:
    fprintf(stderr, "%s must be value of {1, 0, space}\n", binaryStr);
    return 0;
overflow:
    fprintf(stderr, "%s cause overflow for long long type!");
    return 0;
}

TUINT32 binaryStringToU32(const char *binaryStr)
{

}

TUINT16 binaryStringToU16(const char *binaryStr)
{

}

TUINT8 binaryStringToU8(const char *binaryStr)
{

}

TINT64 binaryStringTo64(const char *binaryStr)
{

}

TINT32 binaryStringTo32(const char *binaryStr)
{

}

TINT16 binaryStringTo16(const char *binaryStr)
{

}

TINT8 binaryStringTo8(const char *binaryStr)
{

}

int main(int argc, char const *argv[])
{
    // printf("%lld\n", binaryStringToU128("000 1 1 1 1 1 1"));
    printf("%s\n", u64ToBinaryString(256, EN_IGNORE_LEADING_ZERO));


    return 0;
}