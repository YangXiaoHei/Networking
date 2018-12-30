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

    /* 指定了 dst_port 和 dst_IP，但没有指定 src_port 和 src_IP */
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }
    printf("connnect succ!\n");

    FILE *fp = NULL;
    if ((fp = fdopen(fd, "w")) == NULL)
    {
        printf("fdopen error\n");
        exit(1);
    }
    setbuf(fp, NULL);

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        if (fputs(buf, fp) == EOF)
        {
            printf("fputs error\n");
            exit(1);
        }
    }
    if (ferror(stdin))
    {
        printf("fgets error\n");
        exit(1);
    }
    close(fd);

    return 0;
}