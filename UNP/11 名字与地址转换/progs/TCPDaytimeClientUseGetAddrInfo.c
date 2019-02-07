#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *service = argv[2];

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
            continue;
        }

        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            continue;
        }

        break;

    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("connect to %s %s fail!\n", hostname, service);
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        if (buf[nread] != '\n')
            buf[nread] = '\n';
        buf[nread + 1] = 0;
        if (fputs(buf, stdout) == EOF) {
            printf("fputs error!");
            exit(1);
        }
    }

    freeaddrinfo(ressave);

    return 0;
}