#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fds[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) < 0) {
        perror("socketpair error!");
        exit(1);
    }

    int svrfd = fds[0];
    int clifd = fds[1];

    char *msg = "hello world";

    ssize_t nwrite = 0, nread = 0;

    if ((nwrite = write(svrfd, msg, strlen(msg))) < 0) {
        perror("write error!");
        exit(1);
    }
    
    char buf[1024];
    if ((nread = read(clifd, buf, sizeof(buf))) < 0) {
        perror("read error!");
        exit(1);
    }
    buf[nread] = 0;
    printf("%s\n", buf);

    if ((nwrite = write(clifd, msg, strlen(msg))) < 0) {
        perror("write error!");
        exit(1);
    }

    if ((nread = read(svrfd, buf, sizeof(buf))) < 0) {
        perror("read error!");
        exit(1);
    }
    buf[nread] = 0;
    printf("%s\n", buf);

    return 0;
}