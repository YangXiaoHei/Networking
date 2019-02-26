#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>

int tcp_connect(const char *ip, unsigned short port)
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_port = htons(port);
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
        return -2;
    return fd;
}

void *recvTextFromServer(void *arg)
{
    int fd = (int)arg;
    char buf[1024];
    ssize_t nwrite, nread;
    for (;;) {
        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            perror("server terminated abnormal");
            exit(1);
        } else if (nread == 0) {
            printf("server cut connection\n");
            exit(1);
        }
        buf[nread] = 0;
        printf("%s", buf);
    }
    return (void *)NULL;
}

void sendTextToServer(int fd)
{
    char buf[1024];
    ssize_t nread, nwrite;

    int error = 0;
    pthread_t tid;
    if ((error = pthread_create(&tid, NULL, recvTextFromServer, (void *)fd)) != 0) {
        perror("pthread_Create fail!");
        exit(1);
    }

    while ((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if ((nwrite = write(fd, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread == 0) {
        shutdown(fd, SHUT_WR);
        exit(0);
    } else {
        perror("read error!");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = tcp_connect(argv[1],atoi(argv[2]))) < 0) {
        perror("tcp connect error!");
        exit(1);
    }
    sendTextToServer(fd);
}
