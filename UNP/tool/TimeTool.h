#ifndef _TIME_TOOL_H
#define _TIME_TOOL_H

#include <sys/_types/_ssize_t.h>

void cpu_time(void);
const char *curtimestr();
unsigned long curtimeus();
char *tcpdump_timestamp(void);
ssize_t logx(const char *fmt, ...);

#endif
