#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

struct RLine {
    char buf[8192];
    char *buf_ptr;
    ssize_t read_cnt;
};

pthread_once_t rl_once = PTHREAD_ONCE_INIT;
pthread_key_t rl_key;

ssize_t __read(int fd, struct RLine *tsd, char *c)
{
    if (tsd->read_cnt <= 0) {
    again:
        if ((tsd->read_cnt = read(fd, tsd->buf, sizeof(tsd->buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return -1;
        } else if (tsd->read_cnt == 0) {
           return 0; 
        }
        tsd->buf_ptr = tsd->buf;
    }
    *c = *tsd->buf_ptr++;
    tsd->read_cnt--;
    return 1;
}

void readline_free(void *ptr)
{
    free(ptr);
}

void readline_once(void)
{
    pthread_key_create(&rl_key, readline_free);
}

ssize_t readline(int fd, void *vptr, size_t len)
{
    pthread_once(&rl_once, readline_once);
    struct RLine *tsd = NULL;
    if ((tsd = pthread_getspecific(rl_key)) == NULL) {
        tsd = calloc(1, sizeof(struct RLine));
        pthread_setspecific(rl_key, tsd);
    }   
    size_t n, rc;
    char c;
    char *ptr = vptr;
    for (n = 1; n < len; n++) {
        if ((rc = __read(fd, tsd, &c)) == 1) {
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

