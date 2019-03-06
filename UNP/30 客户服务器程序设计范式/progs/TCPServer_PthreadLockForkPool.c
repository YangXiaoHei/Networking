#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "TimeTool.h"
#include <errno.h>
#include "AddrTool.h"
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

#define MAXN (2 << 20)

static pthread_mutex_t *mtx;

static char response[MAXN];

typedef void(*sig_handler)(int);

ssize_t writen(int fd, char *buf, ssize_t n)
{
    ssize_t nleft = n;
    ssize_t nwrite;
    char *ptr = buf;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno != EAGAIN && errno != EINTR)
                break;
            nwrite = 0;
        }
        ptr += nwrite;
        nleft -= nwrite;
    }
    return n - nleft;
}

sig_handler fuck(int signo, sig_handler newhandler)
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = newhandler;

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

void lock_init(void)
{
    int fd = -1;
    if ((fd = open("/dev/zero", O_RDWR)) < 0) {
        perror("open error!");
        exit(1);
    }

    /* lock must be shared memory */
    mtx = mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    /* lock for mutli process */
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mtx, &mattr);
}

void lock(void)
{
    pthread_mutex_lock(mtx);
}

void unlock(void)
{
    pthread_mutex_unlock(mtx);
}

void sig_int(int signo)
{
    cpu_time();
}

void sig_child(int signo)
{
    pid_t pid;
    int status;
    while ((pid = wait(&status)) > 0)
        printf("collect %d\n", pid);
    if (errno != 0 && errno != ECHILD) {
        perror("waitpid error!");
        exit(1);
    }
}

void handle_req(int connfd)
{
    char buf[1024];
    ssize_t nread, nwrite;

    char *info = getPeerInfo(connfd);
    ssize_t nbytes = 0;
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            if (errno == EINTR)
                nread = 0;
            perror("read error!");
            break;
        } else if (nread == 0) 
            break;
        
        nbytes = atoi(buf);
        if (writen(connfd, response, nbytes) != nbytes) {
            perror("writen error!");
            break;
        }
        printf("send %zd bytes to client %s\n", nbytes, info);
    }
    free(info);
}

void child_main(int i, int fd)
{
    char buf[1024];
    ssize_t nread, nwrite;

    int connfd = -1;
    for (;;) {
        lock();
        if ((connfd = accept(fd, NULL, NULL)) < 0) { 
            if (errno == EINTR)
                continue;
            perror("accpet error!");
            exit(1);        
        }
        unlock();
        char *info = getPeerInfo(connfd);
        printf("child %d - %d accept new connection from %s\n", i, getpid(), info);
        handle_req(connfd);
        printf("child %d - %d cut connection to %s\n", i, getpid(), info);
        free(info);
        close(connfd);
    }
    exit(0);
}

int child_make(int i, int fd)
{
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid > 0)
        return pid;

    child_main(i, fd);
    return pid;
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <nchild>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    lock_init();

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (fuck(SIGINT, sig_int) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    if (fuck(SIGCHLD, sig_child) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd ,100000) < 0){
        perror("listen error!");
        exit(1);
    }

    int nchild = atoi(argv[2]);
    pid_t *childs = calloc(nchild, sizeof(pid_t));
    if (childs == NULL) {
        printf("calloc error!");
        exit(1);
    }

    for (int i = 0; i < nchild; i++)
        childs[i] = child_make(i, listenfd);

    for (;;)
        pause();
}
