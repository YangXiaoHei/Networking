#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>
#include <errno.h>
#include <netdb.h>
#include "../tool/TimeTool.h"
#include <sys/select.h>

/*
 *
 * ➜  review git:(master) ✗ ping www.baidu.com
 * PING www.a.shifen.com (14.215.177.39): 56 data bytes
 * 64 bytes from 14.215.177.39: icmp_seq=0 ttl=55 time=8.345 ms
 * 64 bytes from 14.215.177.39: icmp_seq=1 ttl=55 time=9.093 ms
 * 64 bytes from 14.215.177.39: icmp_seq=2 ttl=55 time=10.071 ms
 * 64 bytes from 14.215.177.39: icmp_seq=3 ttl=55 time=8.799 ms
 * 64 bytes from 14.215.177.39: icmp_seq=4 ttl=55 time=9.795 ms
 * ^C
 * --- www.a.shifen.com ping statistics ---
 * 5 packets transmitted, 5 packets received, 0.0% packet loss
 * round-trip min/avg/max/stddev = 8.345/9.221/10.071/0.635 ms
 */

static int nsent;
extern int log_err;
static size_t datalen = 56;

unsigned short cksum(unsigned short *addr, size_t len)
{
    unsigned short *w = addr;
    unsigned int sum = 0;
    unsigned int anwser = 0;
    while (len > 1) {
        sum += *w++;
        len -= 2;
    }
    if (len == 1) 
        sum += *(unsigned char *)w;
    anwser = (sum & 0xFFFF) + (sum >> 16);
    anwser += (anwser >> 16);
    return ~(unsigned short)anwser; 
}

int main(int argc, char *argv[])
{
    log_err = 1;
    if (argc != 2) {
        logx("usage : %s <ip/hostname>", argv[0]);
        exit(1);
    }
    int icmpfd = -1;
    if ((icmpfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        logx("socket error!");
        exit(1);
    }

    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;

    int error = 0;
    if ((error = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        errno = error;
        logx("getaddrinfo error! %s %s");
        exit(1);
    }

    struct sockaddr_in dstaddr, srcaddr;
    socklen_t srclen = sizeof(srcaddr);
    memcpy(&dstaddr, res->ai_addr, res->ai_addrlen);
    const char *canonname = res->ai_canonname;
    const char *dstip = inet_ntoa(dstaddr.sin_addr);

    printf("PING %s(%s): %zd data length\n", canonname == NULL ? "" : canonname, dstip, datalen);

    char sendbuf[512];
    char recvbuf[512];
    ssize_t nread, nwrite;
    struct icmp *icmp = NULL; 
    struct timeval sendtv, recvtv;
    struct timeval select_tv;
    select_tv.tv_sec = 2;
    select_tv.tv_usec = 0;

    fd_set rset;
    FD_ZERO(&rset);

    int nready = 0;

    for (;;) {
        icmp = (struct icmp *)sendbuf;
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_id = getpid() & 0xFFFF;
        icmp->icmp_seq = nsent++;        
        memset(icmp->icmp_data, 0xA5, datalen);
        gettimeofday(&sendtv, NULL);
        memcpy(icmp->icmp_data, &sendtv, sizeof(struct timeval));

        icmp->icmp_cksum = 0;
        icmp->icmp_cksum = cksum((unsigned short *)sendbuf, 8 + datalen);

        if (sendto(icmpfd, sendbuf, 8 + datalen, 0, (struct sockaddr *)&dstaddr, sizeof(dstaddr)) < 0) {
            logx("sendto error!");
            continue;
        }

recv_again:
        FD_SET(icmpfd, &rset);
        if ((nready = select(icmpfd + 1, &rset, NULL, NULL, &select_tv)) == 0) {
            logx("Request timeout for icmp_seq %d", nsent - 1);
            continue;
        } else if (nread < 0) {
            if (errno != EINTR)
                logx("select error!");
            continue;
        } 

        socklen_t len = srclen;
        if ((nread = recvfrom(icmpfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&srcaddr, &len)) <= 0) {
            logx("read error!");
            continue;
        }

        if (nread < 20) {
            logx("< 20");
            goto recv_again;
        }
        struct ip *ip = (struct ip *)recvbuf;
        int len1 = ip->ip_hl << 2;
        if (len1 < 20) {
            logx("ip header < 20");
            goto recv_again;
        }

        if (ip->ip_p != IPPROTO_ICMP) {
            logx("not icmp");
            goto recv_again;
        }

        struct icmp *icmp = (struct icmp *)(recvbuf + len1);
        if (icmp->icmp_type != ICMP_ECHOREPLY || icmp->icmp_id != (getpid() & 0xFFFF)) {
            logx("icmp not matched icmp_type=%d icmp_id=%d", icmp->icmp_type, icmp->icmp_id);
            goto recv_again;
        }

        int icmplen = nread - len1 - 8;
        if (icmplen < sizeof(struct timeval)) {
            logx("icmp data incomplete %d", icmplen);
            goto recv_again;
        }

        gettimeofday(&recvtv, NULL);
        if ((recvtv.tv_usec -= sendtv.tv_usec) < 0) {
            recvtv.tv_sec--;
            recvtv.tv_usec += 1000000;
        }
        recvtv.tv_sec -= sendtv.tv_sec;
        float cost = recvtv.tv_sec * 1000.0 + recvtv.tv_usec / 1000.0;
        const char *rcvip = inet_ntoa(srcaddr.sin_addr);
        
        /* 64 bytes from 14.215.177.39: icmp_seq=0 ttl=55 time=8.345 ms */
        logx("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms", nread - len1, rcvip, icmp->icmp_seq, ip->ip_ttl, cost);

        sleep(1);
        errno = 0;
    }
}
