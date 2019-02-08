#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    if (argc != 3 && argc != 4) {
        printf("usage : %s <ip/hostname> <port/service> [recv timeout]\n", argv[0]);
        exit(1);
    }
    setbuf(stdout, NULL);
    const char *hostname = argv[1];
    const char *service = argv[2];
    int recv_timeout = 5;

    if (argc == 4)
        recv_timeout = atoi(argv[3]);

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;

    int udpfd = -1;
    do {
        if ((udpfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket error!");
            continue;
        }

        struct timeval tv;
        socklen_t len = sizeof(tv);
        if (getsockopt(udpfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &len) < 0) {
            perror("getsockopt error!");
        }
        printf("default recv timeout = %ld\n", tv.tv_sec);

        tv.tv_sec = recv_timeout;
        tv.tv_usec = 0;

        if (setsockopt(udpfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("socket fd set socket opt error!");
            goto udp_fd_err;
        }
        if (connect(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            goto udp_fd_err;
        }
        break;
    udp_fd_err:
        close(udpfd);
        udpfd = -1;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("can't assign a socket address\n");
        exit(1);
    }

    char c = 1;
    if (write(udpfd, &c, 1) != 1) {
        perror("write error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0;
    if ((nread = read(udpfd, buf, sizeof(buf))) <= 0) {
        if (errno == EWOULDBLOCK)
            errno = ETIMEDOUT;
        perror("read error!");
        exit(1);
    } 

    buf[nread] = 0;
    printf("%s\n", buf);

    return 0;
}