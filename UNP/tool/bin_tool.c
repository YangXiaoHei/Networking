#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bin_tool.h"

static char _buf[128];

#define INT_2_BIN_PROTOTYPE(_prefix_, _intype_)                         \
const char *_prefix_##ToBinaryString(_intype_ a, EN_BIN_TOOL_TYPE type) \
{                                                               \
    int size = sizeof(a) << 3;                                  \
    int i = size - 1, j = 0, m = 0, n = 0, groups = 0;          \
    int mod = 0;                                                \
                                                                \
    /* 忽略先导零 */                                              \
    if (type & ignore_leading)                          \
    while(i >= 0 && ((a >> i) & 1) == 0) i--;                   \
                                                                \
    /* 拼完 0 1 串 */                                            \
    while(i >= 0)                                               \
        _buf[j++] = ((a >> i--) & 1) + '0';                     \
    _buf[j] = 0;                                                \
                                                                \
    /* 要分成多少组？空格分隔 */                                    \
    groups = (type & eight) ? 8 : (type & four) ? 4 : 128;    \
    if (groups == 128)                                          \
    return _buf;                                                \
                                                                \
    /* 如果要分组，先找出余数 ->[xxx] xxxxxxxx xxxxxxxx... */       \
    mod = j % groups;                                           \
    char tmp[128];                                              \
    while (mod--) {                                             \
        tmp[n] = _buf[n];                                       \
        n++;                                                    \
    }                                                           \
    /* 此时 n == mod，但 mod 可能为 0 */                          \
    if (n != 0) {                                               \
        tmp[n++] = ' ';                                         \
        m = n - 1;                                              \
    }                                                           \
    /* 分组，用空格来分隔字符串 */                                  \
    mod = j % groups;                                           \
    while (m < j) {                                             \
        if (m - mod > 0 && (m - mod) % groups == 0)             \
            tmp[n++] = ' ';                                     \
            tmp[n++] = _buf[m++];                               \
    }                                                           \
    tmp[n] = 0;                                                 \
    memmove(_buf, tmp, n + 1);                                  \
    return _buf;                                                \
}

INT_2_BIN_PROTOTYPE(u64, TUINT64)
INT_2_BIN_PROTOTYPE(i64, TINT64)
INT_2_BIN_PROTOTYPE(u32, TUINT32)
INT_2_BIN_PROTOTYPE(i32, TINT32)
INT_2_BIN_PROTOTYPE(u16, TUINT16)
INT_2_BIN_PROTOTYPE(i16, TINT16)
INT_2_BIN_PROTOTYPE(u8, TUINT8)
INT_2_BIN_PROTOTYPE(i8, TINT8)

#define BIN_2_INT_PROTOTYPE(_inttype_, _suffix_)          \
_inttype_ binaryStringTo##_suffix_(const char *binary)    \
{                                                \
    _inttype_ reg = 0;                           \
    int size = sizeof(reg) << 3;                 \
    unsigned char c;                             \
    int i = 0, j = 0, k = 0;                     \
    size_t len = strlen(binary);                 \
    for (; k < len; k++) {                       \
        c = binary[len - k - 1];                 \
        j = c - '0';                             \
        if (c == ' ') continue;                  \
        if (j != 0 && j != 1)                    \
            goto invalid;                        \
        reg |= (j << i++);                       \
        if (i > size)                            \
            goto overflow;                       \
    }                                            \
    return reg;                                  \
invalid:                                                            \
    fprintf(stderr, "%s must be value of {1, 0, ' '}\n", binary);   \
    return 0;                                                       \
overflow:                                                           \
    fprintf(stderr, "%s cause overflow\n", binary);                 \
    return 0;                                                       \
}

BIN_2_INT_PROTOTYPE(TUINT64, U64)
BIN_2_INT_PROTOTYPE(TINT64, I64)
BIN_2_INT_PROTOTYPE(TUINT32, U32)
BIN_2_INT_PROTOTYPE(TINT32, I32)
BIN_2_INT_PROTOTYPE(TUINT16, U16)
BIN_2_INT_PROTOTYPE(TINT16, I16)
BIN_2_INT_PROTOTYPE(TUINT8, U8)
BIN_2_INT_PROTOTYPE(TINT8, I8)

/*
 TUINT64 binaryStringToU64(const char *binary);
 TINT64 binaryStringToI64(const char *binary);
 TUINT32 binaryStringToU32(const char *binary);
 TINT32 binaryStringToI32(const char *binary);
 TUINT16 binaryStringToU16(const char *binary);
 TINT16 binaryStringToI16(const char *binary);
 TUINT8 binaryStringToU8(const char *binary);
 TINT8 binaryStringToI8(const char *binary);
 
 const char *u64ToBinaryString(TUINT64, EN_BIN_TOOL_TYPE type);
 const char *i64ToBinaryString(TINT64, EN_BIN_TOOL_TYPE type);
 const char *u32ToBinaryString(TUINT32, EN_BIN_TOOL_TYPE type);
 const char *i32ToBinaryString(TINT32, EN_BIN_TOOL_TYPE type);
 const char *u16ToBinaryString(TUINT16, EN_BIN_TOOL_TYPE type);
 const char *i16ToBinaryString(TINT16, EN_BIN_TOOL_TYPE type);
 const char *u8ToBinaryString(TUINT8, EN_BIN_TOOL_TYPE type);
 const char *i8ToBinaryString(TINT8, EN_BIN_TOOL_TYPE type);
 */

const char *asciiToBinaryString(unsigned char ascii)
{
    return u8ToBinaryString(ascii, four);
}

unsigned char binaryStringToAscii(const char *binary)
{
    return binaryStringToU8(binary);
}

void printAsciiString(const char *asciiString)
{
    for (int i = 0; i < strlen(asciiString); i++)
        printf("%5d %c -> %s\n", asciiString[i], asciiString[i], asciiToBinaryString(asciiString[i]));
}
