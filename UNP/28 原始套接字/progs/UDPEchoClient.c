#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>
#include  "FdTransmitTool.h"
#include "comm.h"

struct sockaddr_in svraddr;

struct icmpd_err icmpd_err;

int udp_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket error!");
        exit(1);
    }

    memcpy(&svraddr, res->ai_addr, sizeof(struct sockaddr_in));

    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_port = htons(0);
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    return fd;
}

int max(int a, int b) { return a > b ? a : b; }

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <hostname/ip> <service/port>\n", argv[0]);
        exit(1);
    }
    int udpfd = udp_connect(argv[1], argv[2]);    
    int icmpfd = -1;
    if ((icmpfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    } 

    struct sockaddr_un sun;
    bzero(&sun, sizeof(sun));
    strcpy(sun.sun_path, "/tmp/icmp_daemon");
    if (connect(icmpfd, (struct sockaddr *)&sun, SUN_LEN(&sun)) < 0) {
        perror("connect error!");
        exit(1);
    }

    if (write_fd(icmpfd, "", 1, udpfd) < 0) {
        perror("write_fd error!");
        exit(1);
    }

    ssize_t nread;
    fd_set rset;
    FD_ZERO(&rset);
    int maxfd = max(icmpfd, udpfd);

    int nready = 0;
    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {

        if (sendto(udpfd, buf, strlen(buf), 0, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
            perror("sendto error!");
            exit(1);
        }

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_SET(udpfd, &rset);
        FD_SET(icmpfd, &rset);

        if ((nready = select(maxfd + 1, &rset, NULL, NULL, &tv)) == 0) {
            printf("socket timeout\n");
            continue;
        }
        printf("%d readable\n", nready);

        if (FD_ISSET(udpfd, &rset)) {
            ssize_t nread;
            if ((nread = recvfrom(udpfd, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
                perror("recvfrom error!");
                exit(1);
            }
            buf[nread] = 0;
            if (write(STDOUT_FILENO, buf, strlen(buf)) != strlen(buf)) {
                perror("write error!");
                exit(1);
            }
        }

        if (FD_ISSET(icmpfd, &rset)) {
            printf("icmpfd readable\n");
            if ((nread = read(icmpfd, &icmpd_err, sizeof(icmpd_err))) == 0) {
                printf("icmpd daemon terminated\n");
                exit(1);
            } else if (nread != sizeof(icmpd_err)) {
                printf("nread = %zd bytes, expected %zd\n", nread, sizeof(icmpd_err));
                exit(1);
            } else {

                printf("ICMP error: %s, type = %d, code = %d\n", 
                        strerror(icmpd_err.icmpd_errno), icmpd_err.icmpd_type, icmpd_err.icmpd_code);
            }
        }
    }
}
