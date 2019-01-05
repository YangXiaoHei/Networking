#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

ssize_t writen(int fd, const void *vptr, size_t n) 
{
    size_t nleft = n;
    char *ptr = vptr;
    ssize_t nwrite = 0;
    while (nleft > 0) {
        if((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return ptr - (char *)vptr;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}