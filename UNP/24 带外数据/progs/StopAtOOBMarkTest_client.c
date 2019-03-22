#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../../tool/TimeTool.h"

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
    
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }
    
    char buf[3000];
    memset(buf, 'a', sizeof(buf));
    send(fd, buf, sizeof(buf), 0);
    
    char haha[4000];    
    memset(haha, 'x', sizeof(haha)); 
    send(fd, haha, sizeof(haha), MSG_OOB);
   
    memset(buf, 'c', sizeof(buf));
    send(fd, buf, sizeof(buf), 0);


    shutdown(fd, SHUT_WR);

    if ((read(fd, buf, sizeof(buf))) == 0) {
        logx("client closed connection");
    } else {
        logx("server what a fuck!");
    }
}
