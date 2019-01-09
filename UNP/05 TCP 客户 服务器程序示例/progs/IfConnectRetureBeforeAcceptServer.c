#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(fd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    sleep(100);

    int connfd = -1;
    if ((connfd = accept(fd, (struct sockaddr *)&svraddr, &clilen)) < 0) {
        perror("accept error!");
        exit(1);
    }
    printf("accept succ! %ld\n", time(NULL));

    sleep(100);

    return 0;
}