#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include "reliable_udp_review.h"
#include "../../tool/TimeTool.h"

static struct rtt_info rttinfo;
static struct hdr {
    long seq;
    long ts;
} sendhdr, recvhdr;
static int rttinit;

int udp_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        logx("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        return -1;
    }

    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            fd = -1;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        logx("udp connect to %s %s fail!\n", hostname, service);
        exit(1);
    }
    return fd;
}

int readable_timeout(int fd, long timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    return select(fd + 1, &rset, NULL, NULL, &tv); 
}

ssize_t udp_reliable_send_recv(int fd, char *outbuf, ssize_t outlen, char *inbuf, ssize_t inlen)
{
    struct msghdr rmsg, smsg;
    bzero(&rmsg, sizeof(rmsg));
    bzero(&smsg, sizeof(smsg));    
 
    struct iovec iovsend[2];
    struct iovec iovrecv[2];
    
    iovsend[0].iov_base = &sendhdr;
    iovsend[0].iov_len = sizeof(sendhdr);
    iovsend[1].iov_base = outbuf;
    iovsend[1].iov_len = outlen;

    iovrecv[0].iov_base = &recvhdr;
    iovrecv[0].iov_len = sizeof(recvhdr);
    iovrecv[1].iov_base = inbuf;
    iovrecv[1].iov_len = inlen;
    
    sendhdr.seq++;
        
    rmsg.msg_name = NULL;
    rmsg.msg_namelen = 0;
    rmsg.msg_iov = iovrecv;
    rmsg.msg_iovlen = 2;

    smsg.msg_name = NULL;
    smsg.msg_namelen = 0;
    smsg.msg_iov = iovsend;
    smsg.msg_iovlen = 2;
    
    if (rttinit == 0) {
        rtt_init(&rttinfo);
        rttinit = 1;
    }
    rtt_newpack(&rttinfo);

sendagain:
    sendhdr.ts = rtt_ts(&rttinfo);
    if (sendmsg(fd, &smsg, 0) < 0) {
        logx("sendmsg error! %s", strerror(errno));
        return -1;
    }
    int retcode = 0;
    if ((retcode = readable_timeout(fd, rtt_start(&rttinfo))) == 0) {
        if (rtt_timeout(&rttinfo) < 0) {
            errno = ETIMEDOUT;
            return -2;
        }
        goto sendagain;
    } 
    if (retcode < 0)
        return retcode;
    
    ssize_t nread = 0;
    if ((nread = recvmsg(fd, &rmsg, 0)) < 0) {
        logx("recvmsg error! %s", strerror(errno));
        return -3;
    } 
    if (nread < sizeof(recvhdr) || recvhdr.seq != sendhdr.seq) {
        logx("nread = %d, recvhdr.seq = %d", nread, recvhdr.seq);
        goto sendagain;
    }

    rtt_stop(&rttinfo, rtt_ts(&rttinfo) - sendhdr.ts);

    return nread - sizeof(recvhdr);
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        logx("usage : %s <hostname/ip> <service/port>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = udp_connect(argv[1], argv[2])) < 0) {
        perror("udp_connect error!");
        exit(1);
    }    
    
    ssize_t nread = 0;    
    char outbuf[1400];
    ssize_t outlen = sizeof(outbuf);
    char inbuf[1400];
    ssize_t inlen = sizeof(inbuf);
    for (;;) {
        /*
         * 从标准输入读取要发送到服务器的数据
         */
        if (fgets(outbuf, sizeof(outbuf), stdin) == NULL) {
            if (ferror(stdin)) {
                logx("fgets error!\n");
                break;
            } else {
                logx("all data finsihed!\n");
                break;
            }
        } 
        /*
         * 使用增加了超时重传，带确认序号的udp
         */
        if ((nread = udp_reliable_send_recv(fd, outbuf, outlen, inbuf, inlen)) < 0) {
            if (errno == ETIMEDOUT) {
                logx("packet loss\n");
                continue;
            } else {
                logx("udp_reliable_send_recv error!");
                break;
            }
        }
        /*
         * 将服务器写回的响应打印到标准输出
         */
        if (fputs(inbuf, stdout) == EOF) {
            if (ferror(stdout)) {
                logx("fputs error!");    
                break;
            }
        }
    }
}
