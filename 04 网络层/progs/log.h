
#ifndef LOG_H
#define LOG_H

#include<stdlib.h>
#include <stdio.h>

ssize_t YHLog(const char *file, int line, const char *fun, const char *format, ...);
ssize_t YHLog_err(const char *file, int line, const char *fun, const char *format, ...);

#define LOG(_format_, ...)      YHLog(__FILE__, __LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)
#define ERRLOG(_format_, ...)   YHLog_err(__FILE__, __LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)

#endif
