
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

ssize_t YHLog(const char *file, int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s\n", file, fun, line, buf);
    va_end(ap);
    return len;
}

ssize_t YHLog_err(const char *file, int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s: %s\n", file, fun, line, buf, strerror(errno));
    va_end(ap);
    return len;
}