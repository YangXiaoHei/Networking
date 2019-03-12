#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "TimeTool.h"
#include <sys/resource.h>
#include <errno.h>

int log_err = 0;

void cpu_time(void)
{
    struct rusage myusage, childusage;

    double user, sys;
    if (getrusage(RUSAGE_SELF, &myusage) < 0) {
        perror("getrusage error!");
        exit(1);
    }
    if (getrusage(RUSAGE_CHILDREN, &childusage) < 0) {
        perror("getrusage error!");
        exit(1);
    }

    user = (double)(myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec / 1000000.0);
    user += (double)(childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec / 1000000.0);

    sys = (double)(myusage.ru_stime.tv_sec + myusage.ru_stime.tv_usec / 1000000.0);
    sys += (double)(childusage.ru_stime.tv_sec + childusage.ru_stime.tv_usec / 1000000.0);

    printf("\nuser time = %.3f, sys time = %.3f\n", user,sys);
}

char *tcpdump_timestamp(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        fprintf(stderr, "gettimeofday error!");
        exit(1);
    }
    static char str[30];
    /* Tue Feb 12 09:17:48 2019 */
    char *ptr = ctime(&tv.tv_sec);
    strcpy(str, ptr + 11);
    snprintf(str + 8, sizeof(str) - 8, ".%06ld", (long)tv.tv_usec);
    return str;
}

ssize_t _mlogx(const char *file, int line, const char *fmt, ...) 
{
    size_t n = 0;
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    n += snprintf(buf + n, sizeof(buf) - n, "%s:%d %s: ", file, line, tcpdump_timestamp());
    n += vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
    va_end(ap);
    if (log_err && errno != 0) {
        n += snprintf(buf + n, sizeof(buf) - n, " : %s", strerror(errno));
        errno = 0;
    }
    return fprintf(stderr, "%s\n", buf);
}

ssize_t logx(const char *fmt, ...) 
{
    size_t n = 0;
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    n += snprintf(buf + n, sizeof(buf) - n, "%s: ", tcpdump_timestamp());
    n += vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
    va_end(ap);
    if (log_err && errno != 0) {
        n += snprintf(buf + n, sizeof(buf) - n, " : %s", strerror(errno));
        errno = 0;
    }
    return fprintf(stderr, "%s\n", buf);
}

unsigned long curtimeus()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

const char *curtimestr()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm tmb;
    localtime_r(&tv.tv_sec, &tmb);
    struct tm *tmbuf = &tmb;
        
    char *buf = NULL;
    if ((buf = malloc(256)) == NULL) {
        printf("malloc error!");
        exit(1);
    }
    ssize_t len = snprintf(buf, 256, "[%02d:%02d:%02d:%0ld]", 
            tmbuf->tm_hour, 
            tmbuf->tm_min,
            tmbuf->tm_sec,
            (long)tv.tv_usec);
    buf[len] = 0;
    return buf;
}
