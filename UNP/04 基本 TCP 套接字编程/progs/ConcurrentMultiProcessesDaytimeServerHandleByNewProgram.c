#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

ssize_t writen(int fd, void *vptr, size_t n) 
{
    ssize_t nwrite = 0;
    char *ptr = vptr;
    ssize_t nleft = n;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
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
    if (argc != 2) {
        printf("usage : %s <bind_port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }

    struct sockaddr_in svraddr, cliaddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(atoi(argv[1]));
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        perror("listen error!");
        exit(1);
    }

    char ipbuf[128];
    char rspbuf[1024];
    int connfd = -1;
    socklen_t len = sizeof(cliaddr);

    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error!");
            exit(1);
        }

        pid_t child_pid = -1;
        if ((child_pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (child_pid == 0) {

            printf("child process %d handle the TCP connection from %s:%d\n",
                getpid(), 
                inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf)),
                ntohs(cliaddr.sin_port));

            close(listenfd);

            int retcode = -1;
            char arg1[64];
            snprintf(arg1, sizeof(arg1), "%d", connfd);
            if ((retcode = execl("./RunByExecFunctionServer", 
                "./RunByExecFunctionServer", arg1, (char *)0)) < 0) {
                printf("run fail!\n");
                exit(retcode);
            }
            exit(0);
        }
        int status = 0;
        if (waitpid(child_pid, &status, 0) < 0) {
            perror("waitpid error!");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("child %d exit with code : %d\n", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("child %d terminated by signal : %d\n", child_pid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("child %d stopped by signal : %d\n", child_pid, WSTOPSIG(status));
        }

        close(connfd);
    }
    return 0;
}