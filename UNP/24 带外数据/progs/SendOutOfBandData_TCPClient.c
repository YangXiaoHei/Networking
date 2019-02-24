#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "../../tool/TimeTool.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(0);
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
    
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    write(fd, "123", 3);
    logx("wrote 3 bytes of normal data");
    sleep(1);    

    send(fd, "4", 1, MSG_OOB);
    logx("wrote 1 bytes of OOB data");
    sleep(1);
    
    write(fd, "56", 2);
    logx("wrote 2 bytes of normal data");
    sleep(1);
    
    send(fd, "7", 1, MSG_OOB);
    logx("wrote 1 bytes of OOB data");
    sleep(1);
    
    write(fd, "89", 2);
    logx("wrote 1 bytes of normal data"); 
    sleep(1);
}
