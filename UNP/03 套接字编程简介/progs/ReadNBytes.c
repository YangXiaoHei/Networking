#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

ssize_t readn(int fd, void *vptr, size_t n) 
{
    char *ptr = vptr;
    size_t nleft = n;
    ssize_t nread = 0;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno = EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    } 
    return n - nleft;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}