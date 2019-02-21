#ifndef _TIME_TOOL_H
#define _TIME_TOOL_H

const char *curtimestr();
unsigned long curtimeus();
char *tcpdump_timestamp(void);
ssize_t logx(const char *fmt, ...);

#endif
