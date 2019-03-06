#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

typedef void(*sig_fun)(int);

sig_fun fuck(int signo, sig_fun f)
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = f;

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        newact.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        newact.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &newact, &oldact) < 0)
        return SIG_ERR;
    return oldact.sa_handler;
}

int filefd = -1;

void unlock(int fd)
{
    struct flock l;
    l.l_whence = SEEK_SET;
    l.l_start = 0;
    l.l_len = 0;
    l.l_type = F_UNLCK;

    printf("%d release lock\n", getpid());
    if (fcntl(fd, F_SETLKW, &l) < 0) {
        perror("fcntl error!");
    }
    printf("%d release lock succ!\n", getpid());
}

void lock(int fd)
{
    struct flock l;
    l.l_whence = SEEK_SET;
    l.l_start = 0;
    l.l_len = 0;
    l.l_type = F_WRLCK;
    l.l_pid = getpid();

    printf("%d get lock\n", getpid());
    if (fcntl(fd, F_SETLKW, &l) < 0) {
        perror("fcntl error!");
    }
    printf("%d get lock succ!\n", getpid());
}

void sig_int(int signo)
{
    if (filefd < 0)
        return;
    unlock(filefd);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage : %s <pathname>\n", argv[0]);
        exit(1);
    }
    const char *pname = argv[1];
    int fd = open(pname, O_RDWR);
    if (fd < 0) {
        perror("open error!");
        exit(1);
    }
    if (fuck(SIGINT, sig_int) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    filefd = fd;

    for (;;) {
        lock(fd);
        pause();
    }
}
