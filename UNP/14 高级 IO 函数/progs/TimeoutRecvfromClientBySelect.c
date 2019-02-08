#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

int readable_timeout(int fd, int sec)
{
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return select(fd + 1, &rset, NULL, NULL, &tv);
}

int main(int argc, char const *argv[])
{
    if (argc != 3 && argc != 4) {
        printf("usage : %s <ip/hostname> <service/port> [recv timeout]\n", argv[0]);
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
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int udpfd = -1;
    do {
        if ((udpfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create error!");
            continue;
        }
        if (connect(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(udpfd);
            udpfd = -1;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("udp client start fail!\n");
        exit(1);
    }

    char c = 1;
    if (write(udpfd, &c, 1) < 0) {
        perror("write error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0;
    int retcode = 0;
    if ((retcode = readable_timeout(udpfd, recv_timeout)) <= 0) {
        if (retcode == 0)
            errno = ETIMEDOUT;
        perror("recvfrom error!");
        exit(1);
    } else { 
        if ((nread = read(udpfd, buf, sizeof(buf))) <= 0) {
            perror("read error!");
            exit(1);
        } 
        buf[nread] = 0;
        printf("%s\n", buf);
    }

    return 0;
}