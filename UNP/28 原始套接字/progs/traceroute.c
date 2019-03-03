#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

static int sendfd;
static int recvfd;

struct udpdata {
    int seq;
    int ttl;
    struct timeval tv;
};

static int nsent;
static int maxttl = 256;
static char *hostname;
static int nprobes = 3;
static char dstcanonname[256];
static char dstip[128];
static unsigned short dstport = 40000 + 666;

static struct sockaddr_in lastaddr;
static struct sockaddr_in recvaddr;
static socklen_t recvaddrlen;

static char recvbuf[1024];
static char sendbuf[1024];

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

/*
 * -1 代表超时
 * -2 代表收到的 icmp 包是错误的
 * -3 代表 TTL 减为 0 被路由器丢弃
 * -4 代表端口不可达
 */
int process_icmpv4_response(struct sockaddr *addr, socklen_t *addrlen)
{
    alarm(3);
    
    ssize_t nread;
    if ((nread = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, addr, addrlen)) < 0) {
        if (errno == EINTR)
            return -1;
        perror("recvfrom error!");
        exit(1);
    }

    int icmplen = 0;
    int hlen1 = 0, hlen2 = 0;
    struct ip *ip_ptr = (struct ip *)recvbuf;
    struct ip *ip_ptr_2 = NULL;
    struct icmp *icmp_ptr = NULL; 
    char *udp_ptr = NULL;

    if (ip_ptr->ip_p != IPPROTO_ICMP) {
        printf("recv packet not icmp packet!\n");
        return -2;
    }

    hlen1 = ip_ptr->ip_hl << 2;
    /* icmp header 8 bytes + ip header 20~40 bytes + udp header 8 bytes */
    if ((icmplen = nread - hlen1) < 8 + sizeof(struct ip) + 8) { 
        printf("incomplete icmp data\n");
        return -2;
    }
    icmp_ptr = (struct icmp *)(ip_ptr + 1);
    ip_ptr_2 = (struct ip *)(icmp_ptr + 1);
    udp_ptr = (char *)(ip_ptr_2 + 1);
    if (icmp_ptr->icmp_type == ICMP_TIMXCEED && icmp_ptr->icmp_code == ICMP_TIMXCEED_INTRANS) {
        return -3;
    } else if (icmp_ptr->icmp_type == ICMP_UNREACH) {
        if (icmp_ptr->icmp_code == ICMP_UNREACH_PORT) {
            return -4;
        } else {
            return icmp_ptr->icmp_code;
        }
    }    
    return 0;
}

void sig_alrm(int signo) 
{
    /*
     * nothing to do
     */
}

int sockaddr_same(struct sockaddr_in *left, struct sockaddr_in *right)
{
    if (left->sin_port != right->sin_port || left->sin_addr.s_addr != right->sin_addr.s_addr)
        return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <hostname/ip>\n", argv[0]);
        exit(1);
    }
    hostname = argv[1];
    if ((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("udp fd create fail!");
        exit(1);
    }

    if ((recvfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("raw fd create fail!");
        exit(1);
    }
    
    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME; 
    hints.ai_family = AF_INET;
    int error = 0;
    if ((error = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        printf("getaddrinfo error %s : %s\n", hostname, gai_strerror(error));
        exit(1);
    } 
    struct sockaddr_in *inaddr = (struct sockaddr_in *)res->ai_addr;
    char *ip = inet_ntoa(inaddr->sin_addr);
    printf("dst ip = %s canonname = %s\n", ip, res->ai_canonname);
    memcpy(dstip, ip, strlen(ip));
    memcpy(dstcanonname, res->ai_canonname, strlen(res->ai_canonname));
    
    struct sockaddr_in connaddr;
    bzero(&connaddr, sizeof(connaddr));
    connaddr.sin_addr.s_addr = inaddr->sin_addr.s_addr;
    connaddr.sin_family = AF_INET;
    connaddr.sin_port = htons(dstport);
    if (connect(sendfd, (struct sockaddr *)&connaddr, sizeof(connaddr)) < 0) {
        perror("connect error!");
        exit(1);
    }  

    if (fuck(SIGALRM, sig_alrm) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
 
    int done = 0; 
    int i = 0;
    ssize_t nwrite;
    for (i = 1; i < maxttl && done == 0; i++) {
        if (setsockopt(sendfd, IPPROTO_IP, IP_TTL, &i, sizeof(i)) < 0) {
            perror("setsockopt error!");
            exit(1);
        } 
        struct udpdata *data = (struct udpdata *)sendbuf;
        data->seq = nsent++;
        data->ttl = i;
        gettimeofday(&data->tv, NULL);
        printf("%d\t", i);
        for (int j = 0; j < nprobes; j++) {
            if ((nwrite = write(sendfd, sendbuf, sizeof(data))) != sizeof(data)) {
                perror("write error!");
                exit(1);
            }
            struct timeval tvsend, tvrecv;
            gettimeofday(&tvsend, NULL);
            int retcode = 0;
            /*
             * -1 代表超时
             * -2 代表收到的 icmp 包是错误的
             * -3 代表 TTL 减为 0 被路由器丢弃
             * -4 代表端口不可达
             */
            recvaddrlen = sizeof(recvaddr);
            if ((retcode = process_icmpv4_response((struct sockaddr *)&recvaddr, &recvaddrlen)) == -3) {
                int same = 0;
                int if_print = 0;
                char *router;
                if ((same = sockaddr_same(&lastaddr, &recvaddr)) != 1) {
                    if_print = 1;
                    router = inet_ntoa(recvaddr.sin_addr);
                    printf("%s\t", router);
                    memcpy(&lastaddr, &recvaddr, sizeof(struct sockaddr_in));
                }
                gettimeofday(&tvrecv, NULL);
                if ((tvrecv.tv_usec -= tvsend.tv_usec) < 0) {
                    tvrecv.tv_sec--;
                    tvrecv.tv_usec += 1000000;
                }        
                tvrecv.tv_sec -= tvsend.tv_sec;
                double costtime = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0;
                if (j == nprobes - 1) {
                    printf("%.3f ms\n", costtime);
                } else if (!same && !if_print) {
                    printf("%s %3f ms\n", router, costtime); 
                }  else {
                    printf("%.3f ms", costtime);
                }
            } else if (retcode == -4) {
                done = 1;
                break;
            } else {
                printf("%s", j == nprobes - 1 ? " *\n" : " * ");
            } 
        } 
    } 
}
























