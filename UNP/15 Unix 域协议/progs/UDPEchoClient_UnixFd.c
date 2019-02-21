#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include "../../tool/PathTool.h"

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_un svraddr, cliaddr;
    bzero(&cliaddr, sizeof(cliaddr));
    bzero(&svraddr, sizeof(svraddr));

    const char *svrPath = getAbsolutePath("unix_hanson");
    svraddr.sun_family = AF_LOCAL;
    strcpy(svraddr.sun_path, svrPath);

    const char *localPath = getAbsolutePath("unix_hanson_tmp");
    unlink(localPath);
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, localPath);

    if (bind(fd, (struct sockaddr *)&cliaddr, SUN_LEN(&cliaddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nread = 0, nwrite = 0;
    struct sockaddr_un checkaddr;
    socklen_t checklen = sizeof(checkaddr);
    for (;;) {
        socklen_t len = checklen;
        if ((nread = read(fileno(stdin), buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        }
        if ((nwrite = sendto(fd, buf, nread, 0, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr))) < 0) {
            perror("sendto error!");
            exit(1);
        }
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&checkaddr, &len)) < 0) {
            perror("recvfrom error!");
            exit(1);
        }

        if (len != SUN_LEN(&svraddr) || strcmp(checkaddr.sun_path, svraddr.sun_path) != 0) {
            printf("not expected echo data from svr (ignored) %s : %s\n", checkaddr.sun_path, svraddr.sun_path);
            continue;
        }

        if ((nwrite = write(fileno(stdout), buf, nread)) < 0) {
            perror("write error!");
            exit(1);
        }
    }
    return 0;
}
