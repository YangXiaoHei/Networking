#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <errno.h>

ssize_t writen(int fd, void *vptr, size_t n) 
{
    ssize_t nleft = n;
    char *ptr = vptr;
    ssize_t nwrite = -1;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return ptr - (char *)vptr;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <connfd>\n", argv[0]);
        exit(99);
    }

    int connfd = atoi(argv[1]);

    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    bzero(&peeraddr, sizeof(peeraddr));
    if (getpeername(connfd, (struct sockaddr *)&peeraddr, &len) < 0) {
        perror("getpeername error!");
        exit(100);
    }

    char peer_ip[128];
    unsigned short peer_port = ntohs(peeraddr.sin_port);
    printf("process %d handle the response for client 【%s:%d】\n", getpid(),
        inet_ntop(AF_INET, &peeraddr.sin_addr, peer_ip, sizeof(peer_ip)),
        peer_port);

    char rspbuf[1024];
    time_t ticks = time(NULL);
    snprintf(rspbuf, sizeof(rspbuf), "%s\n", ctime(&ticks));
    if (writen(connfd, rspbuf, strlen(rspbuf)) < 0) {
        perror("writen error!");
        exit(101);
    }

    close(connfd);

    return 0;
}