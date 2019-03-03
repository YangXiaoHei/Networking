#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

static pid_t pid;
static int maxttl = 30;
unsigned short dport = 32768 + 666;
unsigned short sport;

int sendfd, recvfd;
int nsent;

char sendbuf[1500];
char recvbuf[1500];

int nprobes = 3;
struct udpdata {
    unsigned short seq;
    unsigned short ttl;
    struct timeval tv;
};
int datalen = sizeof(struct udpdata);

static int goalarm;

char *host;

struct timeval tvrecv, tvsend;

static struct sockaddr_in sendaddr;
static struct sockaddr_in recvaddr;
static struct sockaddr_in lastaddr;
static struct sockaddr_in bindaddr;
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

void sig_alrm(int signo)
{
    goalarm = 1;
}

int sockaddr_same(struct sockaddr_in *left, struct sockaddr_in *right)
{
    return left->sin_port == right->sin_port && left->sin_addr.s_addr == right->sin_addr.s_addr && left->sin_family == right->sin_family;
}

int process_icmpv4_response(int seq, struct timeval *tv)
{
    alarm(3);
    
    socklen_t len = sizeof(recvaddr);
    ssize_t n = 0;
    struct ip *ip_ptr = NULL;
    struct icmp *icmp_ptr = NULL;
    struct udphdr *udp_ptr = NULL;
    struct ip *ip_ptr_2 = NULL;
    
    int hlen1 = 0, hlen2 = 0;
    int icmplen = 0;
    
    goalarm = 0;  
    
    char *ptr = recvbuf; 
    int retcode = 0; 
    for (;;) {
        if (goalarm) 
            return -3;
        len = sizeof(recvaddr);
        if ((n = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&recvaddr, &len)) < 0) {
            if (errno == EINTR)
                continue;
            perror("recvfrom error!");
            exit(1);
        }
        ip_ptr = (struct ip *)recvbuf;
        hlen1 = ip_ptr->ip_hl << 2;
        if ((icmplen = n - hlen1) < 8)
            continue;
        
        icmp_ptr = (struct icmp *)(ptr + hlen1);
        if (icmp_ptr->icmp_type == ICMP_TIMXCEED && icmp_ptr->icmp_code == ICMP_TIMXCEED_INTRANS) {
            if (icmplen < 8 + sizeof(struct ip)) {
                continue;
            }

            ip_ptr_2 = (struct ip *)(ptr + hlen1 + 8);
            hlen2 = ip_ptr_2->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 8) {
                continue;
            }

            udp_ptr = (struct udphdr *)(ptr + hlen1 + 8 + hlen2);
            if (ip_ptr_2->ip_p == IPPROTO_UDP && udp_ptr->uh_sport == htons(sport) && udp_ptr->uh_dport == htons(dport + seq)) {
                retcode = -2;
                break;
            } else {
                printf("%d %d %d sport = %d dport = %d\n", ip_ptr_2->ip_p,(udp_ptr->uh_sport),(udp_ptr->uh_dport), sport, dport);
            }
        } else if (icmp_ptr->icmp_type == ICMP_UNREACH) {
            if (icmplen < 8 + sizeof(struct ip))
                continue;

            ip_ptr_2 = (struct ip *)(icmp_ptr + 1);
            hlen2 = ip_ptr_2->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 8)
                continue;

            udp_ptr = (struct udphdr *)(ptr + hlen1 + 8 + hlen2);
            if (ip_ptr_2->ip_p == IPPROTO_UDP && udp_ptr->uh_sport == htons(sport) && udp_ptr->uh_dport == htons(dport + seq)) {
                if (icmp_ptr->icmp_code == ICMP_UNREACH_PORT) 
                    retcode = -1;
                else
                    retcode = icmp_ptr->icmp_code;
                break;
            }
        }
    }
    alarm(0);
    gettimeofday(tv, NULL);
    return retcode;
}

void traceloop()
{
    if ((recvfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("icmp socket fd create fail!");
        exit(1);
    }
    setuid(getuid());
    if ((sendfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("udp socket fd create fail!");
        exit(1);
    }

    bindaddr.sin_family = AF_INET;
    sport = (getpid() & 0xFFFF) | 0x8000;
    bindaddr.sin_port = htons(sport);
    if (bind(sendfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        perror("bind error!");
        exit(1);
    } 
    
    int done = 0;
    int seq = 0;
    for (int ttl = 1; ttl < maxttl; ttl++) {
        if (setsockopt(sendfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            perror("setsockopt error!");
            exit(1);
        }
        bzero(&lastaddr, sizeof(lastaddr));
        printf("%2d\t", ttl); 
        
        for (int probe = 0; probe < nprobes; probe++) {
            struct udpdata *ud = (struct udpdata *)sendbuf;
            ud->seq = ++seq;
            ud->ttl = ttl;
            gettimeofday(&tvsend, NULL);
            
            sendaddr.sin_port = htons(dport + seq);
            if ((sendto(sendfd, sendbuf, datalen, 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr))) != datalen) {
                perror("write error!");
                exit(1);
            }
            
            int retcode = 0; 
            if ((retcode = process_icmpv4_response(seq, &tvrecv)) == -3) {
                printf(" *");
            } else {
                if (!sockaddr_same(&lastaddr, &recvaddr)) {
                    printf("%s\t", inet_ntoa(recvaddr.sin_addr));
                    memcpy(&lastaddr, &recvaddr, sizeof(recvaddr));
                }
                if ((tvrecv.tv_usec -= tvsend.tv_usec) < 0) {
                    tvrecv.tv_sec--;
                    tvrecv.tv_usec += 1000000; 
                }   
                tvrecv.tv_sec -= tvsend.tv_sec;
                double costtime = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0;
                printf("%.3f ms ", costtime);

                if (retcode == -1)
                    done = 1;
                else if (retcode >= 0)
                    printf(" (ICMP %d) ", retcode);
            }
        }
        printf("\n"); 
    } 
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <hostname/ip>\n", argv[0]);
        exit(1);
    }    
    host = argv[1];
    pid = getpid();
    
    setbuf(stdout, NULL);   
 
    if (fuck(SIGALRM, sig_alrm) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    
    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;
    int error = 0;
    if ((error = getaddrinfo(host, NULL, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n", host, gai_strerror(error));
        exit(1);
    }
    char *ip = inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr);
    printf("traceroute to %s (%s)  %d hops max, %d data bytes\n", res->ai_canonname ? res->ai_canonname : ip, ip, maxttl, datalen);

    memcpy(&sendaddr, res->ai_addr, sizeof(struct sockaddr_in));    
    sendaddr.sin_port = htons(dport);
    traceloop();
}
