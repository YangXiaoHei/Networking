#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

static int port;

int tcp_connect_cb(const char *hostname, const char *service, int(*cb)(int))
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    int saveerrno = 0;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }

        if (cb && cb(fd) < 0) {
            saveerrno = errno;
            continue;
        }

        printf("prepare to connect %d\n", getpid());
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}


int bind_local_port(int fd)
{
    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(port);
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        perror("bind error!");
        return -1;
    }

    struct linger l;
    l.l_onoff = 1;
    l.l_linger = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        perror("setsockopt error!");
        return -2;
    }
    sleep(1);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage : %s <ip> <port> <bindport>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[3]);
    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], bind_local_port)) < 0) {
        perror("tcp_connect error!");
        exit(1);
    }

   for (;;)
        pause();
}
