#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int tcp_server(unsigned short port)
{
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }
    if (listen(listenfd, 10) < 0) {
        perror("listen error!");
        exit(1);
    }
    return listenfd;
}

ssize_t get_cur_time(char *buf, ssize_t buflen)
{
    time_t now = time((time_t *)NULL);
    const char *timestr = ctime(&now);
    return snprintf(buf, buflen, "%s", timestr);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = tcp_server(atoi(argv[1]));
    int connfd = -1;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buf[1024];
    ssize_t ntowrite = 0;
    for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;
            perror("accept error!");
            break;
        }

        const char *ip = inet_ntoa(cliaddr.sin_addr);
        unsigned short port = ntohs(cliaddr.sin_port);
        printf("TCP connection from client [ip:port=%s:%d][connfd=%d]\n", ip, port, connfd);

        ntowrite = get_cur_time(buf, sizeof(buf));
        if (write(connfd, buf, ntowrite) != ntowrite) {
            perror("write error!");
            exit(1);
        }
        printf("close connection\n");
        close(connfd);
        connfd = -1;
    }
    printf("time server terminated abnormally\n");
}
