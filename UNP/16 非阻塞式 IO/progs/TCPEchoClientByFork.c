#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

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
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! : %s %s %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket error!");
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
        printf("client start error!\n");
        exit(1);
    }

    char buf[1024];
    ssize_t nwrite = 0, nread = 0;
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        while ((nread = read(fd, buf, sizeof(buf))) > 0) {
            buf[nread] = 0;
            printf("%s", buf);
        }
        if (nread == 0) {
            printf("server cut connection\n");
        } else {
            printf("server terminated abnormal\n");
        }
        kill(getppid(), SIGTERM);
        exit(0);
    }

    while ((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        buf[nread] = 0;
        if ((nwrite = write(fd, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    shutdown(fd, SHUT_WR);
    pause();

    return 0;
}