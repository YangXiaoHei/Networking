#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

void display(const char *name, int option) 
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    long val = 0;
    socklen_t len = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, option, &val, &len) < 0) {
        perror("getsockopt error!");
        exit(1);
    }
    printf("%s : default = %ld\n", name, val);
    close(fd);
}


int main(int argc, char const *argv[])
{
    /*
     * SO_KEEPALIVE
     * SO_SNDBUF
     * SO_RCVBUF
     * SO_LINGER
     * SO_RCVLOWAT
     * SO_SNDLOWAT
     * SO_REUSEADDR
     * SO_REUSEPORT
    * SO_RCVTIMEO
    * SO_SNDTIMEO   
     */

    display("SO_KEEPALIVE", SO_KEEPALIVE);
    display("SO_SNDBUF", SO_SNDBUF);
    display("SO_RCVBUF", SO_RCVBUF);
    display("SO_LINGER", SO_LINGER);
    display("SO_RCVLOWAT", SO_RCVLOWAT);
    display("SO_SNDLOWAT", SO_SNDLOWAT);
    display("SO_REUSEADDR", SO_REUSEADDR);
    display("SO_REUSEPORT", SO_REUSEPORT);
    display("SO_SNDTIMEO", SO_SNDTIMEO);
    display("SO_RCVTIMEO", SO_RCVTIMEO);

    return 0;
}
