#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../tool/CommonTool.h"

int main(int argc, char *argv[])
{
    int fd = -1;
    if ((fd = udp_connect(argv[1], argv[2])) < 0) {
        perror("tcp_connect error!");
        exit(1);
    }
    char buf[1024];
    sendto(fd, "1234", 4, 0, NULL, 0);
    struct sockaddr_in svraddr;
    socklen_t clilen = sizeof(svraddr);
    recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&svraddr, &clilen); 

    printf("%s %d\n", inet_ntoa(svraddr.sin_addr), ntohs(svraddr.sin_port));
}
