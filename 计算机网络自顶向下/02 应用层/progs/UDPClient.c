#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("usage : %s <#ip> <#port>\n", argv[0]);
        exit(1);
    }

    const char *ip = argv[1];
    unsigned short port = atoi(argv[2]);

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &svraddr.sin_addr) < 0)
    {
        perror("inet_pton error");
        exit(1);
    }
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        perror("socket error");
        exit(1);
    }

    /********** 为 udp 套接字绑定一个端口，而不是让操作系统随便选 *********/
    // struct sockaddr_in tmpaddr;
    // memcpy(&tmpaddr, &svraddr, sizeof(svraddr));
    // tmpaddr.sin_addr.s_addr = 0;
    // tmpaddr.sin_port = htons(5432);
    // if (bind(fd, (struct sockaddr *)&tmpaddr, sizeof(tmpaddr)) < 0)
    // {
    //     perror("bind error");
    //     exit(1);
    // }
    /*************************************************************/

    char buf[1 << 20];
    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        size_t len = strlen(buf);
        ssize_t nsend = 0;
        if ((nsend = sendto(fd, buf, len, 0, (struct sockaddr *)&svraddr, sizeof(svraddr))) < 0)
        {
            perror("sendto error");
            exit(1);
        }
        printf("send msg to [%s:%d] succ!\n", ip, port);
    }
    if (ferror(stdin))
    {
        printf("fgets error\n");
        exit(1);
    }
    close(fd);

    return 0;
}