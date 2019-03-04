#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "FdTransmitTool.h"

#define UNIX_PATH "unix_hanson"

int main(int argc, char *argv[])
{
    int unixfd = -1;
    if ((unixfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    } 

    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sun_family = AF_LOCAL;
    strcpy(svraddr.sun_path, UNIX_PATH);
    unlink(UNIX_PATH); 
    if (bind(unixfd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(unixfd, 1000) < 0) {
        perror("listen error!");
        exit(1);
    }
    
    char buf[1024];
    ssize_t nwrite, nread;
    int connfd = -1;
    for (;;) {
        if ((connfd = accept(unixfd, NULL, NULL)) < 0) {
            perror("accept error!");
            exit(1);
        }
        printf("new unix connection req\n");
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        }
        buf[nread] = 0;
        printf("open %s req\n", buf);
        int filefd = -1;
        if ((filefd = open(buf, O_RDONLY)) < 0) {
            perror("open error!");
            exit(1);
        }
        if (write_fd(connfd, "", 1, filefd) < 0) {
            perror("write_fd error!");
            exit(1);
        }
    } 
}
