#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <bindpath>\n", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("unix fd create fail!");
        exit(1);
    }

    /* 域套接字绑定的地址不能存在，否则将 bind 失败 */
    unlink(argv[1]);

    struct sockaddr_un addr1, addr2;
    bzero(&addr1, sizeof(addr1));
    addr1.sun_family = AF_LOCAL;
    strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1);

    if (bind(fd, (struct sockaddr *)&addr1, SUN_LEN(&addr1)) < 0) {
        perror("bind error!");
        exit(1);
    }
    printf("SUN_LEN = %lu\n", SUN_LEN(&addr1));

    socklen_t len = sizeof(addr2);
    if (getsockname(fd, (struct sockaddr *)&addr2, &len) < 0) {
        perror("getsockname error!");
        exit(1);
    }

    printf("bound name = %s returned len = %d\n", addr2.sun_path, len);

    return 0;
}