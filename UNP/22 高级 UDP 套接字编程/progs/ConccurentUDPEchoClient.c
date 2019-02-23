#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <inttypes.h>
#define SVR_PORT 50001

int udp_connect(const char *ip, unsigned short port)
{
    /*
     * 类似与 TCP，先发送一个 syn 给服务器，当服务器回应ack 时，说明
     * 服务器已经创建好了与该客户端通信的特定套接字
     */
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        return -1;
    }
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        return -2;
    }
    
    const char *syn = "syn";
    if (sendto(fd, syn, 3, 0, NULL, 0) < 0) {
        perror("sendto error!");
        return -3;
    }
    /*
     * 从服务器接收 ack 标记，如果正确收到，那么同时也获取到了新的通信套接字地址
     */
    ssize_t nread = 0;
    struct sockaddr_in connaddr;
    socklen_t connlen = sizeof(connaddr);
    char buf[10];
    if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&connaddr, &connlen)) < 0) {
        perror("recvfrom error!");
        return -4;
    }
    buf[nread] = 0;
    if (strncmp(buf, "ack", 3) != 0)
        return -5;
    
    char *spe = NULL;
    if ((spe = strchr(buf, ':')) == NULL) {
        printf("server internal error!, %s\n", buf);
        return -7;
    }   
    spe++; 
    unsigned short p = strtoll(spe, NULL, 10); 
    connaddr.sin_port = htons(p);
    printf("connect to new port : %d\n", p); 
    if (connect(fd, (struct sockaddr *)&connaddr, sizeof(connaddr)) < 0) {
        perror("connect error!");
        return -6;
    }
    return fd; 
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("usage : %s <ip>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = udp_connect(argv[1], SVR_PORT)) < 0) {
        printf("udp_connect error! %d\n", fd);
        exit(1);
    }
    
    char buf[1024];
    ssize_t nread, nwrite;    
    for (;;) {
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (ferror(stdin)) {
                printf("stdin error!");
                exit(1);
            } else {
                printf("all data finsihed!");
                break;
            }
        }

        if ((nwrite = sendto(fd, buf, strlen(buf), 0, NULL, 0)) != strlen(buf)) {
            perror("sendto error!");
            continue;
        }
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
            perror("recvfrom error!");
            continue;
        }
        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF) {
            if (ferror(stdout)) {
                printf("stdout error!");
                break;
            }
        }
    }
}
