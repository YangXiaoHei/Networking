#ifndef _TIME_TOOL_H
#define _TIME_TOOL_H

#define mlogx(fmt, ...) _mlogx(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void cpu_time(void);
const char *curtimestr();
unsigned long curtimeus();
char *tcpdump_timestamp(void);
ssize_t logx(const char *fmt, ...);

ssize_t _mlogx(const char *file, int line, const char *fmt, ...);

#endif
