#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[2]));
    svraddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    int totalPktToSend = 10000;
    int bytesPerPkt = 1500;

    char *buf = NULL;
    if ((buf = malloc(bytesPerPkt)) == NULL) {
        printf("malloc error!");
        exit(1);
    }

    while (totalPktToSend--) {
        if (sendto(fd, buf, bytesPerPkt, 0, NULL, 0) < 0) {
            perror("sendto error!");
            exit(1);
        }
        printf("there are [ %d ] packet to send\n", totalPktToSend);
    }
    printf("all send finished!");

    return 0;
}