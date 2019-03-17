#ifndef _BIN_TOOL_H_
#define _BIN_TOOL_H_

#ifndef YXH_TYPE_DEF
#define YXH_TYPE_DEF
typedef unsigned long      TUINT64;
typedef unsigned           TUINT32;
typedef long long          TINT128;
typedef long               TINT64;
typedef int                TINT32;
typedef unsigned short     TUINT16;
typedef unsigned char      TUINT8;
typedef short              TINT16;
typedef char               TINT8;
#else
#error "what a fuck?!"
#endif

typedef enum EN_BIN_TOOL_TYPE {
    ignore_leading  = 1 << 0,
    four       = 1 << 1,
    eight      = 1 << 2
} EN_BIN_TOOL_TYPE;

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

const char *asciiToBinaryString(unsigned char ascii);
unsigned char binaryStringToAscii(const char *binary);
void printAsciiString(const char *asciiString);

#endif
