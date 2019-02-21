#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int TCP_connect(const char *hostname, const char *service)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        return -1;
    }
    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket create fail!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            fd = -1;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    freeaddrinfo(ressave);

    if (res == NULL) { /* 野指针，但是不用它, dont care */ 
        printf("connect to server %s %s error!", hostname, service);
        return -2;
    }
    return fd;
}

// #define writen(fd, buf, len) send(fd, buf, len, MSG_WAITALL)

ssize_t writen(int fd, void *ptr, ssize_t n)
{
    ssize_t nwrite = 0, ntowrite = n;
    char *vptr = ptr;
    while (ntowrite > 0) {
        if ((nwrite = write(fd, vptr, ntowrite)) < 0) {
            if (errno != EINTR && errno != EAGAIN)
                break;
            nwrite = 0;
        } 
        if (nwrite != 0)
            printf("--- nwrite = %ld\n", nwrite);
        vptr += nwrite;
        ntowrite -= nwrite;
    }
    return n - ntowrite;
}

typedef void(*sig_handler)(int);

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

void write_brokenfd(int signo) 
{
    printf("write broken pipe\n");
}
void write_interrupt(int signo) {}
void collect_child(int signo) 
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("collect child %d\n", pid);
    }
    if (pid < 0 && errno != ECHILD) {
        perror("waitpid");
        exit(1);
    }
}

void nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <hostname/ip> <service/port>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *hostname = argv[1];
    const char *service = argv[2];

    if (fuck(SIGINT, write_interrupt) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }
    if (fuck(SIGCHLD, collect_child) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    if (fuck(SIGPIPE, write_brokenfd) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    pid_t pid = -1;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        usleep(1000 * 200);
        for (;;) {
            kill(getppid(), SIGINT);
            usleep(1000 * 10);
        }
        exit(0);
    }

    int sockfd = -1;
    if ((sockfd = TCP_connect(hostname, service)) < 0) {
        perror("TCP_connect error!");
        exit(1);
    }

    // nonblock(sockfd);

    ssize_t len = 10 << 20;
    char *buf = malloc(len);
    memset(buf, 'a', len);

    ssize_t nwrite = writen(sockfd, buf, len);
    printf("all finished! total_need_write=%ld nwrite = %ld\n", len, nwrite);
    if (len != nwrite) {
        perror("writen error!");
        exit(1);
    }

    printf("send FIN\n");
    shutdown(sockfd, SHUT_WR);
    char c = 0;
    while (read(sockfd, &c, 1) != 0); 
    printf("peer all received\n");

    if (kill(pid, SIGTERM) < 0) {
        perror("kill child fail!");
        exit(1);
    }

    sleep(1); // 为了回收子进程
    return 0;
}