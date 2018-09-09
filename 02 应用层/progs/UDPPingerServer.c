#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

ssize_t YHLog(int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s\n", __FILE__, fun, line, buf);
    va_end(ap);
    return len;
}

ssize_t YHLog_err(int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s: %s\n", __FILE__, fun, line, buf, strerror(errno));
    va_end(ap);
    return len;
}

#define LOG(_format_, ...) YHLog(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)
#define ERRLOG(_format_, ...) YHLog_err(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)

int start_service(unsigned short port)
{
    int retcode = 0;
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ERRLOG("socket error");
        retcode = -1;
        goto err;
    }
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("bind error");
        retcode = -2;
        goto err;
    }
    return fd;
err:
    return retcode;
}

#define PING_MAGIC 0x1234567887654321UL

struct ping 
{
    unsigned long magic;
    unsigned int seq;
    unsigned long ping_timestamp;
};

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        LOG("usage : %s <#port>", argv[0]);
        exit(1);
    }
    unsigned short port = atoi(argv[1]);
    int fd;
    if ((fd = (start_service(port))) < 0)
    {
        LOG("start_service error");
        exit(1);
    }

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    struct ping pbuf;
    char ipbuf[128];
    while (1)
    {
        if (recvfrom(fd, &pbuf, sizeof(pbuf), 0, (struct sockaddr *)&cliaddr, &len) < 0)
        {
            ERRLOG("recvfrom error");
            break;
        }
        const char *ip = inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf));
        unsigned short port = htons(cliaddr.sin_port);
        LOG("receive data from [%s:%d]", ip, port);
        if (pbuf.magic != PING_MAGIC && !(pbuf.seq >= 1 && pbuf.seq <= 10))
        {
            LOG("error ping pack from [%s:%d]! discard it\n", ip, port);
            continue;
        }
        if (sendto(fd, &pbuf, sizeof(pbuf), 0, (struct sockaddr *)&cliaddr, len) < 0)
        {
            ERRLOG("sendto error");
            break;
        }
        LOG("send ping_%d back to [%s:%d] succ!", pbuf.seq, ip, port);
    }
    close(fd);

    return 0;
}