
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

ssize_t logForDebug(const char *file, int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    // len = fprintf(stderr, "[%s:%s:%d] %s\n", file, fun, line, buf);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tmbuf = localtime(&tv.tv_sec);
    len = fprintf(stderr, "[%d:%d:%d:%d] %s\n", 
        tmbuf->tm_hour, 
        tmbuf->tm_min, 
        tmbuf->tm_sec, 
        tv.tv_usec, 
        buf);
    va_end(ap);
    return len;
}

ssize_t errLogForDebug(const char *file, int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    // len = fprintf(stderr, "[%s:%s:%d] %s: %s\n", file, fun, line, buf, strerror(errno));
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tmbuf = localtime(&tv.tv_sec);
    len = fprintf(stderr, "[%d:%d:%d:%d] %s: %s\n", 
        tmbuf->tm_hour, 
        tmbuf->tm_min, 
        tmbuf->tm_sec, 
        tv.tv_usec, 
        buf,
        strerror(errno)
        );
    va_end(ap);
    return len;
}