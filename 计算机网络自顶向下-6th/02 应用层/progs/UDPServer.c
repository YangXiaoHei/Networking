#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("usage : %s <#port>\n", argv[0]);
        exit(1);
    }

    unsigned short port = atoi(argv[1]);

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("bind error");
        exit(1);
    }

    char buf[1 << 20];
    char ipbuf[128];
    ssize_t rdlen = 0;
    while (1)
    {
        printf("wait to read data...\n");
        socklen_t clilen = sizeof(cliaddr);
        if ((rdlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror("recvfrom error");
            exit(1);
        }
        buf[rdlen] = 0;
        const char *ip;
        unsigned short port = ntohs(cliaddr.sin_port);
        if ((ip = inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf))) == NULL)
        {
            perror("inet_ntop error!");
            exit(1);
        }
        printf("receive data from [%s:%d]\n-------------------------------\n", ip, port);
        printf("%s", buf);
        printf("-------------------------------\n");
    }

    return 0;
}