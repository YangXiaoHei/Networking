#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/uio.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

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

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    static char *msg1 = "static memory msg 1 write by veason\n";

#define MSG2 ("heap memory msg 2 write by hanson\n") 
    char *msg2 = malloc(strlen(MSG2));
    memcpy(msg2, MSG2, strlen(MSG2));

    char msg3[] = "stack memory msg 3 write by laughing\n";

    struct iovec iov[3];
    iov[0].iov_base = msg1;
    iov[0].iov_len = strlen(msg1);

    iov[1].iov_base = msg2;
    iov[1].iov_len = strlen(MSG2);

    iov[2].iov_base = msg3;
    iov[2].iov_len = sizeof(msg3);

    ssize_t totalBytes = strlen(msg1) + strlen(MSG2) + sizeof(msg3);
    if (writev(fd, iov, 3) != totalBytes) {
        perror("writev error!");
        exit(1);
    }
    printf("writev succ!\n");
    return 0;
}