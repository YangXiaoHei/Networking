#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }    

    setbuf(stdout, NULL);

    const char *hostname = argv[1];
    const char *service = argv[2];

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error ! %s %s : %s\n", hostname, service, gai_strerror(error));
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

    ssize_t nread = 0;
    if (write(fd, "", 1) != 1) {
        perror("write error!");
        exit(1);
    }

    char buf[1024];
    if ((nread = read(fd, buf, sizeof(buf))) < 0) {
        perror("read error!");
        exit(1);
    }

    buf[nread] = 0;
    printf("%s\n", buf);

    return 0;
}