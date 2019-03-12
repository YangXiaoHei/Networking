#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "../tool/TimeTool.h"

int main(int argc, char *argv[])
{
    int fd = STDIN_FILENO;
    char buf[1024];
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    pid_t pid;
    int nready = 0;

    if ((pid = fork()) < 0) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        if ((nready = select(fd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            exit(1);
        }
        logx("child begin to read from stdin");
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            logx("child read succ from stdin");
            if (fputs(buf, stdout) == EOF) {
                if (ferror(stdout)) {
                    fprintf(stderr, "fputs error!");
                    exit(1);
                }
            }
        } 
        exit(0);
    }

    if ((nready = select(fd + 1, &rset, NULL, NULL, NULL)) < 0) {
        perror("select error!");
        exit(1);
    }

    logx("parent begin to read from stdin");
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        logx("parent read succ from stdin");
        if (fputs(buf, stdout) == EOF) {
            if (ferror(stdout)) {
                fprintf(stderr, "fputs error!");
                exit(1);
            }
        }
    } 

    int status;
    if (wait(&status) < 0) {
        perror("wait error!");
        exit(1);
    }
}

