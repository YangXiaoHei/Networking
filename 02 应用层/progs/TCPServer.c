#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }
    
    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svraddr.sin_port = htons(5000);
    
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("bind error");
        exit(1);
    }
    
    if (listen(listenfd, 10) < 0)
    {
        perror("listen error");
        exit(1);
    }
    
    char buf[1 << 20];
    socklen_t len;
    int connfd;
    ssize_t rdlen;
    while (1)
    {
        len = sizeof(cliaddr);
        printf("wait to accept....\n");
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
        {
            perror("accpet error");
            exit(1);
        }
        const char *ip = inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf));
        unsigned short port = cliaddr.sin_port;
        printf("accpet succ! connnection from %s:%d\n", ip, port);
        
        while (1)
        {
            printf("begin to read data...\n");
            if ((rdlen = read(connfd, buf, sizeof(buf))) < 0)
            {
                printf("read error : already read %zd bytes, error reason: %s\n",rdlen, strerror(errno));
                exit(1);
            }
            else if (rdlen == 0)
            {
                printf("read finished!\n");
                close(connfd);
                break;
            }
            buf[rdlen] = 0; 
            printf("received data : %s", buf);
        }
    }
    
    return 0;
}
