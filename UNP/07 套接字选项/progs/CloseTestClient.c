#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "TimeTool.h"
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

#ifdef _USE_LINGER_
    struct linger l = { 1, 30 };
    socklen_t llen = sizeof(l);
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, llen) < 0) {
        perror("setsockopt error!");
        exit(1);
    }
#endif

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    char *buf = NULL;
    int size = 100 << 10;

    if ((buf = malloc(size)) == NULL) {
        printf("malloc error!");
        exit(1);
    }

    memset(buf, 'a', size);
    printf("write begin! %s\n", curTime());
    unsigned long begin = getCurTimeUs();
    if (write(fd, buf, size) != size) {
        perror("write error!");
        exit(1);
    }
    printf("write finished! %s total_cost=%ld\n", curTime(), getCurTimeUs() - begin);

    printf("close begin %s\n", curTime());
    begin = getCurTimeUs();
    close(fd);
    printf("close finished! %s total_cost=%ld\n", curTime(), getCurTimeUs() - begin);

    return 0;
}