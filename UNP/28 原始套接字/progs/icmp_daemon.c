#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include "comm.h"
#include <sys/un.h>
#include "FdTransmitTool.h"
#include "../../tool/AddrTool.h"
#include <netinet/udp.h>
#include "../../tool/TimeTool.h"

struct client {
    int connfd;
    int family;
    int lport;
} client[FD_SETSIZE];

int fd4, listenfd, maxi, maxfd, nready;
fd_set rset, allset;
struct sockaddr_un cliaddr;

int max(int a, int b) {  return a > b ? a : b; }

unsigned short bind_wild(int fd)
{
   struct sockaddr_in bindaddr;
   bzero(&bindaddr, sizeof(bindaddr));
   bindaddr.sin_port = htons(0);
   bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) 
       return -1;
   socklen_t len = sizeof(bindaddr);
   if (getsockname(fd, (struct sockaddr *)&bindaddr, &len) < 0) 
       return -1;
   return bindaddr.sin_port;
}

int readable_listen()
{
    int connfd = -1;
    socklen_t len = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("accept error!");
        return --nready;
    }
    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (client[i].connfd < 0) {
            client[i].connfd = connfd;
            break;
        }
    }
    if (i == FD_SETSIZE) {
        logx("too many connections");
        close(connfd);
        return --nready;
    }
    logx("new connection, i = %d, connfd = %d", i, connfd);
    
    FD_SET(connfd, &allset);

    if (connfd > maxfd)
        maxfd = connfd;
    if (i > maxi)
        maxi = i;
    
    return --nready;
}

int readable_conn(int i)
{
    ssize_t nread;
    char c;

    int recvfd;
    /* 接收客户端传过来的 udp 套接字描述符 */
    if ((nread = read_fd(client[i].connfd, &c, 1, &recvfd)) == 0) {
       logx("client %d terminated, recvfd = %d", i, recvfd);
       goto clientdone;
    } 
    if (recvfd < 0) {
       logx("read_fd did not return fd");
       goto clienterr;
    }     

    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    
    logx("succ recv transmitted fd = %d", recvfd);

    /* 获取客户端 udp 描述符绑定的端口 */
    if (getsockname(recvfd, (struct sockaddr *)&ss, &len) < 0) {
       perror("getsockname error!");
       goto clienterr;
    }

    /* 如果客户端端未曾绑定一个临时端口，那么在 icmpd 守护进程中帮他绑定一个 */
    client[i].family = ss.ss_family;
    if ((client[i].lport = ((struct sockaddr_in *)&ss)->sin_port) == 0) {
       client[i].lport = bind_wild(client[i].connfd); 
       if (client[i].lport < 0) {
           logx("bind wild port error!");
           goto clienterr;
       }
    }
    close(recvfd);
    return --nready;
clienterr:
clientdone:
    close(client[i].connfd);
    if (recvfd >= 0)
        close(recvfd);
    FD_CLR(client[i].connfd, &allset);
    client[i].connfd = -1;
    return --nready;
}

int readable_v4(void)
{
    struct sockaddr_in from;
    socklen_t len = sizeof(from);
    
    char buf[1024];
    ssize_t n = 0;
    /*
     * 接收一个 icmpv4 分组
     */
    if ((n = recvfrom(fd4, buf, sizeof(buf), 0, (struct sockaddr *)&from, &len)) < 0) {
        perror("recvfrom error!");
        exit(1);
    }
    logx("%zd bytes ICMPv4 from %s", n, getAddrInfo((struct sockaddr *)&from));

    struct ip *ip, *hip;
    struct icmp *icmp;
    struct udphdr *udp;
    int hlen1, hlen2;
    int icmplen;
    ip = (struct ip *)buf;
    hlen1 = ip->ip_hl << 2; /* ip 首部长度 */
    unsigned short sport; 

    if ((icmplen = n - hlen1) < 8) {
        logx("icmp %d < 8", icmplen);
        exit(1);
    } 
    icmp = (struct icmp *)(buf + hlen1);
    logx(" type = %d, code = %d", icmp->icmp_type, icmp->icmp_code);

    /* 只接收不可达，超时，源熄灭的 ICMP 分组 */
    if (icmp->icmp_type == ICMP_UNREACH || icmp->icmp_type == ICMP_TIMXCEED || icmp->icmp_type == ICMP_SOURCEQUENCH) {
        if (icmplen < 8 + 20 + 8) {
            logx("icmplen %d < 8 + 20 + 8", icmplen);
            exit(1);
        }

        /* 找出 icmp 分组中容纳的 ip 首部 */
        hip = (struct ip *)(buf + hlen1 + 8);
        hlen2 = hip->ip_hl << 2;
        logx("srcip = %s, dstip = %s, proto = %d", strdup(inet_ntoa(hip->ip_src)), strdup(inet_ntoa(hip->ip_dst)), hip->ip_p);

        /* 如果 ip 负荷中容纳的是 udp 分组 */
        if (hip->ip_p == IPPROTO_UDP) {
            udp = (struct udphdr *)(buf + hlen1 + 8 + hlen2);
            sport = udp->uh_sport;

            logx("udp ----- xixixixixi");
            for (int i = 0; i <= maxi; i++) {

                /*
                 * 遍历所有连接该域套接字的客户端，找出与接收到 ICMPv4 分组的
                 * udp 负荷中的源端口号一致的
                 */
                logx("i = %d connfd=%d family=%d, port=%d sport=%d", i, client[i].connfd, client[i].family, client[i].lport, sport);
                if (client[i].connfd >= 0 && client[i].family == AF_INET && client[i].lport == sport) {
                    logx("hahahahahahahaha");

                    struct sockaddr_in dst;
                    bzero(&dst, sizeof(dst));

                    dst.sin_family = AF_INET;
                    dst.sin_len = sizeof(dst);
                    memcpy(&dst.sin_addr, &hip->ip_dst, sizeof(hip->ip_dst));
                    dst.sin_port = udp->uh_dport;

                    struct icmpd_err icmpd_err;
                    icmpd_err.icmpd_type = icmp->icmp_type;
                    icmpd_err.icmpd_code = icmp->icmp_code;
                    icmpd_err.icmpd_len = sizeof(struct sockaddr_in);
                    memcpy(&icmpd_err.icmpd_dst, &dst, sizeof(dst));
                    
                    icmpd_err.icmpd_errno = EHOSTUNREACH;
                    if (icmp->icmp_type == ICMP_UNREACH) {
                        if (icmp->icmp_code == ICMP_UNREACH_PORT) 
                            icmpd_err.icmpd_errno = ECONNREFUSED;
                        else if (icmp->icmp_code == ICMP_UNREACH_NEEDFRAG)
                            icmpd_err.icmpd_errno = EMSGSIZE;
                    }

                    /* 把该 ICMPv4 错误告知客户端 */
                    if (write(client[i].connfd, &icmpd_err, sizeof(icmpd_err)) != sizeof(icmpd_err)) {
                        perror("write error!");
                        exit(1);
                    }
                }
            }
        }
    }
    return --nready;
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    maxi = -1;
    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i].connfd = -1;
    } 
    FD_ZERO(&allset);
    
    if ((fd4 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket create fail!");
        exit(1);
    }

    FD_SET(fd4, &allset);
    maxfd = fd4;

    if ((listenfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket create fail!");
        exit(1);
    }
    struct sockaddr_un sun;
    sun.sun_family = AF_LOCAL;
    strcpy(sun.sun_path, "/tmp/icmp_daemon");
    unlink("/tmp/icmp_daemon");
    if (bind(listenfd, (struct sockaddr *)&sun, SUN_LEN(&sun)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }
    FD_SET(listenfd, &allset);
    maxfd = max(maxfd, listenfd);
    
    for (;;) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            continue;
        }
        if (FD_ISSET(listenfd, &rset)) {
            if (readable_listen() <= 0)
                continue;
        } 
        if (FD_ISSET(fd4, &rset)) {
            if (readable_v4() <= 0)
               continue;
        }
        for (int i = 0; i <= maxi; i++) {
            if (client[i].connfd < 0)
                continue;

            if (FD_ISSET(client[i].connfd, &rset)) {
                if (readable_conn(i) <= 0)
                    break;
            }
        }

    }
}
