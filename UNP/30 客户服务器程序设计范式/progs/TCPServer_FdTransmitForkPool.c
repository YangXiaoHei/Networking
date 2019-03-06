#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "AddrTool.h"
#include "FdTransmitTool.h"
#include <errno.h>

#define MAXN (2 << 20)

static char response[MAXN];

struct Child {
    int status;
    int pipefd;
    pid_t pid;
};

int max(int a, int b) { return a > b ? a : b;  }

static struct Child *childs;

ssize_t writen(int fd, char *buf, ssize_t n)
{
    ssize_t nleft = n;
    ssize_t nwrite;
    char *ptr = buf;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno != EINTR && errno != EAGAIN)
                break;
            nwrite = 0;
        }
        ptr += nwrite;
        nleft -= nwrite;
    }
    return n - nleft;
}

void handle_req(int connfd)
{
    char buf[1024];
    ssize_t nread, nwrite;
    for (;;) {
        if ((nread = read(connfd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            break;
        } else if (nread == 0) 
            break;
        ssize_t nbytes = atoi(buf);
        if (writen(connfd, response, nbytes) != nbytes) {
            perror("writen error!");
            exit(1);
        }
    }
}

void child_main(int i)
{
    int connfd = -1;
    char c;
   
    for (;;) {
        if ((read_fd(STDERR_FILENO, &c, 1, &connfd)) < 0) {
            perror("read_fd error!");
            exit(1);
        }
        char *info = getPeerInfo(connfd);
        handle_req(connfd);
        close(connfd);
        printf("cut connection from %s\n", info);
        free(info);

        if (write(STDERR_FILENO, "1", 1) != 1) {
            perror("write error!");
            exit(1);
        }
    }
}

void child_make(int i, int listenfd)
{
    int fds[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) < 0) {
        perror("socket error!");
        exit(1);
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid > 0) {
        close(fds[0]);
        childs[i].pipefd = fds[1];
        childs[i].pid = pid;
        childs[i].status = 0;
        return;
    }
    if (dup2(fds[0], STDERR_FILENO) < 0) {
        perror("dup2 error!");
        exit(1);
    }
    close(fds[0]);
    close(fds[1]);
    close(listenfd);
    child_main(i);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <nchild>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int nchild = atoi(argv[2]);

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd error!");
        exit(1);
    }

    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100000) < 0) {
        perror("listen error!");
        exit(1);
    }

    if ((childs = calloc(nchild, sizeof(struct Child))) == NULL) {
        printf("calloc error1");
        exit(1);
    }

    int maxfd = listenfd;    
    fd_set allset, rset;
    FD_ZERO(&allset);
    FD_ZERO(&rset);
    FD_SET(listenfd, &allset);
    for (int i = 0; i < nchild; i++) {
        child_make(i, listenfd);
        FD_SET(childs[i].pipefd, &allset);
        maxfd = max(childs[i].pipefd, maxfd);
    }

    int navali = nchild;
    int nready = 0;
    char c;
    ssize_t nread;
    int connfd = -1;
    for (;;) {
        rset = allset;

        /* main process accept only if there are avaliable child processes */
        if (navali <= 0)
            FD_CLR(listenfd, &rset);

        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
                continue;
            perror("select error!");
            exit(1);
        }

        if (FD_ISSET(listenfd, &rset)) {
            int i = 0;
            for (; i < nchild; i++) 
                if (childs[i].status == 0)
                    break;

            if (i == nchild) {
                printf("unexpected! no avaliable child\n");
                exit(1);
            }

            if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
                perror("accept error!");
                --nready;
                continue;
            }

            childs[i].status = 1;
            navali--;
            char *info = getPeerInfo(connfd);
            printf("tcp connection from %s\n", info);
            free(info);

            if (write_fd(childs[i].pipefd, "", 1, connfd) < 0) {
                perror("write_fd error!");
                --nready;
                continue;
            }
            close(connfd);
        }

        for (int i = 0; i < nchild; i++) {
            if (FD_ISSET(childs[i].pipefd, &rset)) {
                if ((nread = read(childs[i].pipefd, &c, 1)) <= 0) {
                    perror("child process terminated\n");
                    FD_CLR(childs[i].pipefd, &allset);
                    close(childs[i].pipefd);
                    --nready;
                    continue;
                }
                childs[i].status = 0;
                ++navali;

                if (--nready <= 0)
                    continue;
            }
        }
    } 
}



















