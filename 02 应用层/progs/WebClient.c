#include <stdio.h>
#include <stdlib.h>
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

    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &svraddr.sin_addr) < 0)
    {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }
    printf("connnect succ!\n");

    ssize_t ntowrite = 0;

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        write(fd, buf, strlen(buf));
        printf("client begin to receive\n");
        ssize_t nread = read(fd, buf, sizeof(buf));
        printf("client receive data\n");
        buf[nread] = 0;
        if (fputs(buf, stdout) == EOF)
        {
            printf("fputs error!\n");
            exit(1);
        }
    }
    if (ferror(stdin))
    {
        printf("fgets error");
        exit(1);
    }
    return 0;
}