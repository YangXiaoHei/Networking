#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/un.h>

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);

    int fd = -1;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    const char *pathname = "unix_hanson";

    struct sockaddr_un svraddr;
    bzero(&svraddr, sizeof(svraddr));
    strcpy(svraddr.sun_path, pathname);
    svraddr.sun_family = AF_LOCAL;
    if (connect(fd, (struct sockaddr *)&svraddr, SUN_LEN(&svraddr)) < 0) {
        perror("connect error!");
        exit(1);
    }
    
    sleep(5);

    char buf[1024];
    ssize_t nread = 0;
    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        buf[nread] = 0;
        printf("nread = %zd content = [%s]\n", nread, buf);
    }
    if (nread == 0) {
        printf("server cut connection\n");
    } else {
        perror("read error!");
    }

    return 0;
}