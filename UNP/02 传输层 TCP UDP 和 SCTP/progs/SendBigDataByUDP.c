#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

long curtimeus(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);

    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &svraddr.sin_addr) < 0) {
        perror("ip format error");
        exit(1);
    }

    int sendbuff = 0;
    socklen_t len = sizeof(sendbuff);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }
    printf("send buffer size = %d\n", sendbuff);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    printf("input ? 100B you want to send\n");
    int wantToSend = 0;
    scanf("%d", &wantToSend);

    const int bufsize = wantToSend * 100;
    char *buf = malloc(bufsize);
    memset(buf, 'a', bufsize);

    long beg = curtimeus();
    if (sendto(fd, buf, bufsize, 0, NULL, 0) < 0) {
        perror("sendto error!");
        exit(1);
    }
    printf("sendto succ! %.2f ms\n", (curtimeus() - beg) / 1000.0);

    // 即使服务器端口未监听，sendto 还是能发送成功，但如果接下来 recvfrom 的话，会返回 Connection refused 错误
    // char c = 0;
    // if (recvfrom(fd, &c, 1, 0, NULL, NULL) < 0) {
    //     perror("recvfrom error!");
    //     exit(1);
    // }

    close(fd);

    return 0;
}
