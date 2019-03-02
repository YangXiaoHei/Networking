#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include "../../tool/AddrTool.h"
#include <signal.h>
#include <sys/time.h>

static char *hostname;
static char ipstr[40];

char recvbuf[4096];
char sendbuf[4096];
static int nsent;

int sockfd;
pid_t pid;
int datalen = 56;

struct sockaddr_in dstaddr;
socklen_t addrlen;

typedef void(*sig_handler)(int);

sig_handler fuck(int signo, sig_handler newhandler)
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = newhandler;
    
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        newact.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        newact.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &newact, &oldact) < 0)
        return SIG_ERR;
    return oldact.sa_handler;
}

unsigned short checksum(unsigned short *data, size_t len)
{
    unsigned short *w = data;
    unsigned int sum = 0;
    unsigned short anwser = 0;    

    while (len > 1) {
        sum += *w++;
        len -= 2;
    }

    if (len == 1) {
        anwser = *(unsigned char *)w;
        sum += anwser;
    }
    sum = (sum >> 16) + (sum & 0XFFFF);
    sum += (sum >> 16);
    anwser = ~sum;
    return anwser; 
}

void sig_alarm(int signo)
{
    struct icmp *icmp_ptr = (struct icmp *)sendbuf;
    icmp_ptr->icmp_type = ICMP_ECHO;
    icmp_ptr->icmp_code = 0;
    icmp_ptr->icmp_id = pid;
    icmp_ptr->icmp_seq = nsent++;
    memset(icmp_ptr->icmp_data, 0xA5, datalen); 
    gettimeofday((struct timeval *)icmp_ptr->icmp_data, NULL);    

    size_t len = 8 + datalen;

    icmp_ptr->icmp_cksum = 0;
    icmp_ptr->icmp_cksum = checksum((unsigned short *)icmp_ptr, len);

    ssize_t nwrite;
    if ((nwrite = sendto(sockfd, sendbuf, len, 0, (struct sockaddr *)&dstaddr, addrlen)) < 0) {
        perror("sendto error!");
        exit(1);
    }
    alarm(1);
}

void readloop(void)
{
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("create raw socket fd fail!");
        exit(1);
    }

    struct iovec iov[1];
    iov[0].iov_base = recvbuf;
    iov[0].iov_len = sizeof(recvbuf);
    
    if (fuck(SIGALRM, sig_alarm) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }   
    struct ip *ip_ptr;
    struct icmp *icmp_ptr; 
    int iphdrlen;    
    int icmphdrlen;
    int icmpdatalen;
    int icmplen;
    sig_alarm(SIGALRM);
    ssize_t nread;
    char *ptr = recvbuf;
    for (;;) {
        if ((nread = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, NULL, NULL)) < 0) {
            if (errno == EINTR)
                continue;
            perror("readv error!");
            exit(1);
        }

        ip_ptr = (struct ip *)recvbuf;
        iphdrlen = ip_ptr->ip_hl << 2;
        icmp_ptr = (struct icmp *)(ptr + iphdrlen); 
        
        /* ip upper protocol must be IPPROTO_ICMP */
        if (ip_ptr->ip_p != IPPROTO_ICMP) {
            printf("response not IPPROTO_ICMP");
            continue;
        }

        /* icmp header length must be 8 bytes */
        if ((icmplen = nread - iphdrlen) < 8) {
            printf("icmp header length is %zd\n", icmplen);
            continue;
        }

        /* not echo replay icmp  */
        if (icmp_ptr->icmp_type != ICMP_ECHOREPLY) {
            printf("icmp type must be ICMP_ECHOREPLY %zd\n", icmp_ptr->icmp_type);
            continue;
        }
        
        /* not sent by this process */
        if (icmp_ptr->icmp_id != pid) {
            printf("icmp response sent by pid=%d\n", pid);
            continue;
        }
        
        /* data incompelet */
        if (icmplen < 8 + sizeof(struct timeval)) {
            printf("icmp response length is %zd\n", icmplen);
            continue;
        }

        struct timeval tvsend = *(struct timeval *)icmp_ptr->icmp_data;
        struct timeval tvrecv;
        gettimeofday(&tvrecv, NULL);

        float rtt = 0;
        if ((tvrecv.tv_usec -= tvsend.tv_usec) < 0) {
            tvrecv.tv_sec--;
            tvrecv.tv_usec += 1000000;
        }
        tvrecv.tv_sec -= tvsend.tv_sec;
        rtt = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0;
        printf("%d bytes from %s : seq=%u, ttl=%d, rtt=%.3f ms id = %d, code = %d type = %d\n", icmplen, ipstr, icmp_ptr->icmp_seq, ip_ptr->ip_ttl, rtt, icmp_ptr->icmp_id, icmp_ptr->icmp_code, icmp_ptr->icmp_type);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <hostname/ip>\n", argv[0]);
        exit(1);
    }
    hostname = argv[1];
    
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    
    pid = getpid() & 0xFFFF;
    printf("process id = %ld\n", (long)pid);

    int error = 0;
    if ((error = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n", hostname, gai_strerror(error));
        exit(1);
    }
    char *str = getAddrInfo(res->ai_addr);
    memcpy(ipstr, str, strlen(str));
    free(str);
    printf("PING %s(%s): 56 data bytes\n", res->ai_canonname, ipstr);
    
    memcpy((char *)&dstaddr, res->ai_addr, res->ai_addrlen);
    addrlen = res->ai_addrlen;

    readloop(); 
}
