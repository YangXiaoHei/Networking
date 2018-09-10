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
#include <sys/time.h>

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

#define PING_MAGIC 0x1234567887654321UL

struct ping 
{
    unsigned long magic;
    unsigned int seq;
    unsigned long ping_timestamp;
};

unsigned long curtime_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void setflag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= flag;
    fcntl(fd, F_SETFL, flags);
}

void clrflag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL);
    flags &= ~flag;
    fcntl(fd, F_SETFL, flags);
}

int recv_rsp(int fd, char *buf, size_t buflen, int *timeout, struct sockaddr_in *addr, socklen_t *len)
{
    *timeout = 0;

    setflag(fd, O_NONBLOCK);

    int nbytes;
    unsigned long begin = curtime_us(), end;
    while (1)
    {
        if ((nbytes = recvfrom(fd, buf, buflen, 0, (struct sockaddr *)addr, len)) > 0)
            break;

        end = curtime_us();
        if (nbytes <= 0)
        {
            if ((end - begin) > 1000000)
                break;

            if (errno != EAGAIN && errno != EINTR)
            {
                ERRLOG("recvfrom error!");
                return -1;
            }
        }
    }

    clrflag(fd, O_NONBLOCK);

    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        LOG("usage : %s <#ip> <#port>\n", argv[0]);
        exit(1);
    }

    struct ping pings[10];
    for (int i = 0; i < 10; i++)
    {
        pings[i].magic = PING_MAGIC;
        pings[i].seq = i;
    }

    unsigned short port = atoi(argv[2]);
    struct sockaddr_in svraddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], &svraddr.sin_addr) < 0)
    {
        ERRLOG("inet_pton error");
        exit(1);
    }

    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ERRLOG("socket error");
        exit(1);
    }

    socklen_t len;
    struct ping recvping;
    int timeout;
    for (int i = 0; i < 10; i++)
    {
        pings[i].ping_timestamp = curtime_us();
        if (sendto(fd, pings + i, sizeof(struct ping), 0, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
        {
            ERRLOG("sendto error");
            exit(1);
        }

        len = sizeof(svraddr);
        if (recv_rsp(fd, (char *)&recvping, sizeof(recvping), &timeout, &svraddr, &len))
        {
            LOG("recv_rsp error!");
            exit(1);
        }

        if (timeout)
        {
            LOG("ping_%d timeout...", i + 1);
            continue;
        }

        if (recvping.magic != PING_MAGIC && recvping.seq != i)
        {
            LOG("recv invalid data from [%s:%d]", argv[1], port);
            exit(1);
        }

        unsigned long recv_timestamp = curtime_us();
        LOG("ping_%d from %s, time = %.3f ms", recvping.seq, argv[1], (recv_timestamp - recvping.ping_timestamp) / 1000.0);
    }

    return 0;
}