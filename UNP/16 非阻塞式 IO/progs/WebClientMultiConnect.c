#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>


#define GET_CMD "GET %s HTTP/1.1\r\n\r\n"

int TCP_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        return -1;
    }
    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket create fail!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            fd = -1;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    freeaddrinfo(ressave);

    if (res == NULL) { /* 野指针，但是不用它, dont care */ 
        printf("connect to server %s %s error!", hostname, service);
        return -2;
    }
    return fd;
}

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        printf("usage : %s <hostname/ip> <service/port> <file>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *hostname = argv[1];
    const char *service = argv[2];
    const char *file = argv[3];

    int fd = -1;
    if ((fd = TCP_connect(hostname, service)) < 0) {
        printf("TCP_connect to server error!");
        exit(1);
    }

    char buf[1024];
    snprintf(buf, sizeof(buf), GET_CMD, file);
    ssize_t nwrite = 0, nread = 0;
    if ((nwrite = write(fd, buf, strlen(buf))) != strlen(buf)) {
        perror("write error!");
        exit(1);
    }

    const char *download_dir = "download";
    if (mkdir(download_dir, 0777) < 0 && errno != EEXIST) {
        perror("mkdir error!");
        exit(1);
    }

    char filebuf[1024];
    snprintf(filebuf, sizeof(filebuf), "%s/%s", download_dir, file);

    int filefd = -1;
    if ((filefd = open(filebuf, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
        perror("open error!");
        exit(1);
    }

    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        if (write(filefd, buf, nread) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    printf("download file succ!\n");

    return 0;
}