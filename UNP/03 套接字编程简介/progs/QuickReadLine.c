#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static ssize_t read_cnt;
static char read_buf[8192]; /* 10k */
static char *read_ptr;

ssize_t __internal_read(int fd, void *vptr) 
{
    char *ptr = vptr;
    if (read_cnt <= 0) {
    again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            else
                return -1;
        } else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) 
{
    ssize_t rc = 0;
    char c = 0;
    char *ptr = vptr;
    for (size_t n = 1; n < maxlen; n++) {
        if ((rc = __internal_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return n - 1;
        } else 
            return -1;
    }
    *ptr = 0;
    return n;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}