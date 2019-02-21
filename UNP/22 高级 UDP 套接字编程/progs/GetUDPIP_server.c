#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

struct pkt_info {
    struct in_addr udp_ip;
    int    udp_ifindex;
};

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int recvbuf = 100 << 10;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &recvbuf, sizeof(recvbuf)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    int on = 1;
#ifdef IP_RECVORIGDSTADDR
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVORIGDSTADDR, &on, sizeof(on)) < 0) {
#elif defined(IP_RECVDSTADDR) 
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on)) < 0) {   
#endif
        perror("setsockopt error!");
        exit(1);
    }

#ifdef IF_RECVIF
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVIF, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }
#endif

    struct msghdr msg;

    union {
        struct msghdr msg;
        char control[CMSG_SPACE(sizeof(struct sockaddr_in)) + CMSG_SPACE(sizeof(struct pkt_info))]; 
    } control;

    char buf[1024];
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    struct pkt_info info;
    bzero(&info, sizeof(info));

    for (;;) {

        bzero(&msg, sizeof(msg));
        bzero(&control, sizeof(control));
        bzero(&info, sizeof(info));

        msg.msg_control = control.control;
        msg.msg_controllen = sizeof(control);
        msg.msg_flags = 0;

        msg.msg_name = &cliaddr;
        msg.msg_namelen = clilen;

        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        ssize_t nread = 0;
        if ((nread = recvmsg(sockfd, &msg, 0)) < 0) {
            perror("recvmsg error!");
            exit(1);
        }
        buf[nread] = 0;
        if (msg.msg_controllen < sizeof(struct cmsghdr) || (msg.msg_flags & MSG_CTRUNC)) {
            printf("recvbuf len too less\n");
            exit(1);
        }

        struct cmsghdr *ptr = NULL;
        for (ptr = CMSG_FIRSTHDR(&msg); ptr != NULL; ptr = CMSG_NXTHDR(&msg, ptr)) {
#ifdef IP_RECVORIGDSTADDR
            if (ptr->cmsg_level == IPPROTO_IP && ptr->cmsg_type == IP_RECVORIGDSTADDR) {
#elif defined(IP_RECVDSTADDR)
            if (ptr->cmsg_level == IPPROTO_IP && ptr->cmsg_type == IP_RECVDSTADDR) {
#endif
                info.udp_ip = ((struct sockaddr_in *)CMSG_DATA(ptr))->sin_addr;

            }

#ifdef IF_RECVIF
            if (ptr->cmsg_level == IPPROTO_IP && ptr->cmsg_type == IP_RECVIF) {
                struct sockaddr_dl *dl_addr = (struct sockaddr_dl *)CMSG_DATA(ptr);
                info.udp_ifindex = dl_addr->sdl_index;
            }
#endif
        }

        char addr[1024];
        snprintf(addr, sizeof(addr), "[%s:%d]", inet_ntoa(info.udp_ip), ntohs(cliaddr.sin_port));
        printf("udp req from %s, recv %ld bytes\n", addr, (long)nread);

        ssize_t nwrite = 0;
        if ((nwrite = sendto(sockfd, buf, nread, 0, (struct sockaddr *)&cliaddr, msg.msg_namelen)) < 0) {
            perror("sendto error!");
            continue;
        }
    }
    return 0;
}