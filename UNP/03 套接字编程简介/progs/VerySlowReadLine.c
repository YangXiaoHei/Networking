#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

ssize_t readline(int fd, void *vptr, size_t maxlen) 
{
    ssize_t rc = 0;
    char *ptr = vptr;
    char c = 0;
    for (size_t n = 1; n < maxlen; n++) {
    again:
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return n - 1;
        } else {
            if (errno == EINTR)
                goto again;
            return -1;
        }
    }
    *ptr = 0;
    return n;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}