#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define DAYTIME_SVR_PORT (20001)

int main(int argc, char const *argv[])
{
    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in cliaddr, svraddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(DAYTIME_SVR_PORT);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(listenfd, 3) < 0) {
        perror("listen error!");
        exit(1);
    }

    char buf[2048];
    time_t curtime = time(NULL);
    int connfd = -1;
    socklen_t len = sizeof(cliaddr);
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            exit(1);
        }
        snprintf(buf, sizeof(buf), "%24s\r\n", ctime(&curtime));
        if (write(connfd, buf, strlen(buf)) != strlen(buf)) {
            perror("write error");
            exit(1);
        }

        close(connfd);
    }
    return 0;
}