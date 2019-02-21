#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "../../tool/AddrTool.h"

typedef enum EN_ERROR {
    REQ_INCOMPLETE = 1,
    REQ_FORMAT_INVALID,
    REQ_FILE_ERROR,
    RSP_SEND_ERROR
} EN_REQ_ERROR;

int TCP_server(const char *service)
{
    const char *hostname = NULL;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int listenfd = -1;
    do {
        if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket create fail!");
            continue;
        }
        int on = 1;
        if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
            perror("setsockopt error!");
            goto err;
        }
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("bind error!");
            goto err;
        }
        if (listen(listenfd, 10000) < 0) {
            perror("listen error!");
            goto err;
        }
        break;
    err:
        close(listenfd);
        listenfd = -1;
    } while ((res = res->ai_next) != NULL);

    freeaddrinfo(ressave);

    if (res == NULL) {
        printf("TCP_server %s %s socket fd create fail!\n", hostname, service);
        return -1;
    }
    return listenfd;
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

void collect_child(int signo)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
    if (pid < 0 && errno != ECHILD)
        perror("waitpid error!");
    return;
}

int hanlde_client_req(int connfd)
{
    ssize_t nread = 0, nwrite = 0;
    char buf[8192];
    char *ptr = buf, *end = &buf[8192];
    while ((nread = read(connfd, ptr, end - ptr)) > 0) {
        ptr[nread] = 0;
        if (strstr(buf, "\r\n\r\n") != NULL) 
            break;
        ptr += nread;
    }
    // printf("req content = [%s]\n", buf);
    if (nread <= 0 && strstr(buf, "\r\n\r\n") == NULL) 
        return REQ_INCOMPLETE;

    char *pathbeg = NULL;
    if ((pathbeg = strstr(buf, "GET")) == NULL)
        return REQ_FORMAT_INVALID;
    pathbeg += 4;

    char *pathend = NULL;
    if ((pathend = strchr(pathbeg, ' ')) == NULL) 
        return REQ_FORMAT_INVALID;

    char path[256];
    strncpy(path, pathbeg, pathend - pathbeg);
    printf("client req [file=%s]\n", path);

    int fd = -1;
    if ((fd = open(path, O_RDONLY)) < 0)
        return REQ_FILE_ERROR;

    while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        if ((nwrite = write(connfd, buf, nread)) != nread) {
            perror("write error!");
            return RSP_SEND_ERROR;
        }
    }

    if (nread < 0) {
        perror("read error!");
        return REQ_FILE_ERROR;
    } else {
        printf("file finished! [file=%s]\n", path);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <service/port>\n", argv[0]);
        exit(1);
    }

    int listenfd = -1;
    if ((listenfd = TCP_server(argv[1])) < 0) {
        perror("TCP_server start error!");
        exit(1);
    }

    if (fuck(SIGCHLD, collect_child) == SIG_ERR) {
        perror("fuck error!");
        exit(1);
    }

    int connfd = -1;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    pid_t pid;
    for (;;) {
        socklen_t len = clilen;
    again:
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            if (errno == EINTR)
                goto again;
            perror("accept error!");
            continue;
        }

        char *cliInfo = getAddrInfo((struct sockaddr *)&cliaddr);
        printf("client connection from %s\n", cliInfo);
        free(cliInfo);

        if ((pid = fork()) < 0) {
            perror("fork error!");
            continue;
        } else if (pid == 0) {
            int retcode = hanlde_client_req(connfd);
            char result[1024];
            switch (retcode) {
                case 0 :
                    break;

                case REQ_FILE_ERROR :
                    snprintf(result, sizeof(result), "400 req file error! HTTP/1.1 \r\n\r\n");
                    break;

                case REQ_INCOMPLETE : 
                    snprintf(result, sizeof(result), "400 req incomplete! HTTP/1.1 \r\n\r\n");
                    break;

                case REQ_FORMAT_INVALID :
                    snprintf(result, sizeof(result), "400 req formate invalid! HTTP/1.1 \r\n\r\n");
                    break;

                case RSP_SEND_ERROR :
                    snprintf(result, sizeof(result), "500 rsp send error! HTTP/1.1 \r\n\r\n");
                    break;
                default :
                    snprintf(result, sizeof(result), "500 unknow error! HTTP/1.1 \r\n\r\n");
                    break;
            }
            if (retcode != 0)
                write(connfd, result, strlen(result));
            exit(0);
        }
        close(connfd);
    }

    return 0;
}
