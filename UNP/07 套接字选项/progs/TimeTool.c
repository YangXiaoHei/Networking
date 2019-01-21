#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "TimeTool.h"

const char *currentTime()
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
    ssize_t len = snprintf(buf, 256, "%0d/%0d/%0d [%0d:%0d:%0d:%0d]", 
            tmbuf->tm_year + 1900, 
            tmbuf->tm_mon + 1, 
            tmbuf->tm_mday, 
            tmbuf->tm_hour, 
            tmbuf->tm_min,
            tmbuf->tm_sec,
            tv.tv_usec);
    buf[len] = 0;
    return buf;
}