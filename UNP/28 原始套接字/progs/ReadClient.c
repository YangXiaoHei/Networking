#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "../../tool/PathTool.h"
#include "FdTransmitTool.h"

#define UNIX_PATH "unix_hanson"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <path>\n", argv[0]);
        exit(1);
    }

    int unixfd = -1;
    if ((unixfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    strcpy(svraddr.sun_path, getAbsolutePath(UNIX_PATH));
    svraddr.sun_family = AF_LOCAL;
   
    if (connect(unixfd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    } 

    if (write(unixfd, argv[1], strlen(argv[1])) != strlen(argv[1])) {
        perror("write error!");
        exit(1);
    }

    int filefd = -1;
    char c;
    int recvfd = -1;
    ssize_t nread, nwrite;
    char buf[1024];
    if ((nread = read_fd(unixfd, &c, 1, &recvfd)) < 0) {
        perror("read_fd error!");
        exit(1);
    }
    
    while ((nread = read(recvfd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, nread) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread < 0) {
        printf("read error!");
        exit(1);
    } else {
        printf("read EOF\n");
    }
}
