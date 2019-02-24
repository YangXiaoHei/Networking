#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);
    
    ssize_t sndbuf = 2 << 10;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
        perror("setsockopt error!");
        exit(1);
    }   
 
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }
    
    ssize_t len = 5000; 
    char buf[len];
    /*
     * 发送普通数据
     */
    if (send(fd, buf, len, 0) < 0) {
        perror("send error!");
        exit(1);
    }    
    /*
     * 发送紧急数据
     */
    char c = 'a';
    if (send(fd, &c, 1, MSG_OOB) < 0) {
        perror("send OOB data fail!");
        exit(1);
    } 
    /*
     * 发送紧急数据后面的普通数据 
     */
    if (send(fd, buf, len, 0) < 0) {
        perror("send error!");
        exit(1);
    }    
    printf("all data finished!");
}
