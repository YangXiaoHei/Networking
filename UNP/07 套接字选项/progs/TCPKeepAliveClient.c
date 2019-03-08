#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../tool/CommonTool.h"

int main(int argc, char *argv[])
{
    int fd = -1;
    if ((fd = tcp_connect(argv[1], argv[2])) < 0) {
        perror("tcp_connect error!");
        exit(1);
    }

    char buf[1024];
    ssize_t nwrite, nread;
    while ((fgets(buf, sizeof(buf), stdin)) != NULL) {
        if (write(fd, buf, strlen(buf)) != strlen(buf)) {
            perror("write error!");
            exit(1);
        }
    }
}
