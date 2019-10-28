#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../tool/CommonTool.h"

int main(int argc, char *argv[])
{
    int fd = -1;
    if ((fd = udp_server(argv[1])) < 0) {
        perror("udp_server error!");
        exit(1);
    }
    char buf[1024];
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    ssize_t nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen);
    sendto(fd, buf, nread, 0, (struct sockaddr *)&cliaddr, clilen);
}
