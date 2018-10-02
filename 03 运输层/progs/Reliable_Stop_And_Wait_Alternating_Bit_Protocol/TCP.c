#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include "log.h"

int TCP_service_init(unsigned short bind_port)
{
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(bind_port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error");
        exit(1);
    }
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("bind error");
        exit(1);
    }
    if (listen(listenfd, 1000) < 0)
    {
        ERRLOG("listen error");
        exit(1);
    }
    LOG("service init succ!");
    return listenfd;
}

int TCP_connect(const char *peer_ip, unsigned short peer_port)
{
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof(peeraddr));
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(peer_port);
    if (inet_pton(AF_INET, peer_ip, &peeraddr.sin_addr) < 0)
    {
        ERRLOG("inet_pton error!");
        exit(1);
    }
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error!");
        exit(1);
    }
    int try_count = 5;
try_again:
    if (connect(fd, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) < 0)
    {
        ERRLOG("connect fail!, try_count = %d", try_count);
        if (try_count-- == 0)
            exit(1);
        goto try_again;
    }
    LOG("connect to [%s:%d] succ!", peer_ip, peer_port);
    return fd;
}

static void clrflags(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL);
    flags &= ~flag;
    fcntl(fd, F_SETFL, flags);
}

static void setflags(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= flag;
    fcntl(fd, F_SETFL, flags);
}

ssize_t TCP_send(int fd, const char *send_buf, ssize_t send_len)
{
    if (send_len < 0)
    {
        LOG("send_buf_len cannot be negative!");
        return -1;
    }
    setflags(fd, O_NONBLOCK);
    ssize_t nsent, nfinished = 0, counter = 0;
    while (nfinished < send_len)
    {
        if ((nsent = write(fd, send_buf + nfinished, send_len - nfinished)) < 0)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                ERRLOG("write error");
                goto SEND_END;
            }
            nsent = 0;
        }
        nfinished += nsent;
        counter++;
    }
SEND_END:
    clrflags(fd, O_NONBLOCK);
    LOG("TCP_send finished! [loop_count=%d][finished_percentage=%.3f%%]", counter, (nfinished * 1.0 / send_len) * 100);
    return nfinished;
}

ssize_t TCP_recv(int fd, char *recv_buf, ssize_t recvlen)
{
    if (recvlen < 0)
    {
        LOG("maxlen cannot be negative!");
        return -1;
    }
    setflags(fd, O_NONBLOCK);
    ssize_t nread, nfinished = 0, counter = 0;
    while(nfinished < recvlen) 
    {
        if ((nread = read(fd, recv_buf + nfinished, recvlen - nfinished)) < 0)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                ERRLOG("read error");
                goto RECV_END;
            }
            nread = 0;
        }
        nfinished += nread;
        counter++;
    }
RECV_END:
    clrflags(fd, O_NONBLOCK);
    LOG("TCP_recv finished! [loop_count=%d][total_read=%d]", counter, nfinished);
    return nfinished;
}

int TCP_close(int fd)
{
    return close(fd);
}



