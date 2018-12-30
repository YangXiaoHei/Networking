#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        printf("usage : %s <#ip> <#port> <#file_name>\n", argv[0]);
        exit(1);
    }

    const char *file = argv[3];

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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
    if (connect(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }
    printf("connnect succ!\n");

    int flags = fcntl(sockfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    struct st stbuf;
    if (lstat(file, &stbuf) < 0)
    {
        perror("lstat error!");
        exit(1);
    }
    int filefd;
    if ((filefd = open(file, O_RDONLY)) < 0)
    {
        perror("open error!");
        exit(1);
    }

    char *filebuf;
    if ((filebuf = malloc(stbuf.st_size)) == NULL)
    {
        printf("malloc error!");
        exit(1);
    }
    if (read(filefd, filebuf, stbuf.st_size) <= 0)
    {
        perror("read error!");
        exit(1);
    }

    ssize_t ntowrite = stbuf.st_size;
    ssize_t nwritten = 0, curwritten = 0;
    while (nwritten < ntowrite)
    {
        if ((curwritten = write(sockfd, filebuf + nwritten, ntowrite - nwritten)) <= 0)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                perror("write error!")
                exit(1);
            }
            curwritten = 0;
        }
        nwritten += curwritten;
    }
    printf("%d bytes file data transmit finished!", stbuf. st_size);
    free(filebuf);
    close(sockfd);

    return 0;
}