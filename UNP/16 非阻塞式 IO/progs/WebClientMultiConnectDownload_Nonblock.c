#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "TimeTool.h"

#define GET_CMD "GET %s HTTP/1.1\r\n\r\n"

#define FLCTL_CONNECTING 1
#define FLCTL_READING 2
#define FLCTL_DONE 3

struct filectl {
    const char *file;
    int fd;
    int flags;
};

fd_set rset, wset;
int maxfd;

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

void block(int fd)
{
    int flags = fcntl(fd, F_SETFL);
    flags &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

void nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int write_get_cmd(struct filectl *ctl)
{
    if (ctl->fd <= 0)
        return -1;

    if (ctl->file == NULL || strlen(ctl->file) == 0)
        return -2;

    block(ctl->fd);

    ssize_t nwrite, nread;
    char reqbuf[1024];
    snprintf(reqbuf, sizeof(reqbuf), GET_CMD, ctl->file);
    if (write(ctl->fd, reqbuf, strlen(reqbuf)) != strlen(reqbuf)) {
        perror("write error! incomplete");
        return -3;
    }
    FD_CLR(ctl->fd, &wset);
    ctl->flags = FLCTL_READING;

    return 0;
}

int TCP_nonblock_connect(struct addrinfo *res, struct filectl *ctl)
{
    int fd = -1;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket fd create fail!");
        return -2;
    }

    nonblock(fd);

    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        if (errno != EINPROGRESS) 
            return -3;
        FD_SET(fd, &rset);
        FD_SET(fd, &wset);
        ctl->fd = fd;
        ctl->flags = FLCTL_CONNECTING;
        if (fd > maxfd)
            maxfd = fd;
    } else {
        return write_get_cmd(ctl);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc < 5) {
        printf("usage : %s <maxconn> <hostname/ip> <service/port> <file> ...\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *hostname = argv[2];
    const char *service = argv[3];
    int file_beg_idx = 4;
    int nfiles = argc - file_beg_idx;

    const char *download_dir = "download";
    if (mkdir(download_dir, 0777) < 0 && errno != EEXIST) {
        perror("mkdir error!");
        exit(1);
    }

    int fd = -1;
    int nconn = 0;
    int i = 0;
    struct filectl *ctls = NULL;
    char buf[1024];
    char filebuf[1024];
    int filefd = -1;

    int maxconn = 0;
    if ((maxconn = atoi(argv[1])) <= 0) {
        printf("maxconn must beyond zero!\n");
        exit(1);
    }

    struct addrinfo hints, *res = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        return -1;
    }

    if ((ctls = malloc(nfiles * sizeof(struct filectl))) == NULL) {
        printf("malloc error!\n");
        exit(1);
    }
    for (i = 0; i < nfiles; i++) {
        ctls[i].fd = -1;
        ctls[i].flags = 0;
        ctls[i].file = argv[file_beg_idx++];
    }

    long beg = curtimeus();
    fd_set rs, ws;
    int nlefttoread = nfiles;
    ssize_t nread, nwrite;

    while (nlefttoread > 0) {

        while (nconn < maxconn) {
            for (i = 0; i < nfiles; i++)
                if (ctls[i].flags == 0)
                    break;

            if (i == nfiles)
                break;

            if ((fd = TCP_nonblock_connect(res, &ctls[i])) < 0) {
                perror("TCP_connect to server error!");
                exit(1);
            }
            nconn++;
        }

        rs = rset;
        ws = wset;

        again:
        if (select(maxfd + 1, &rs, &ws, NULL, NULL) < 0) {
            if (errno == EINTR)
                goto again;
        }

        for (int j = 0; j < nfiles; j++) {

            int flags = ctls[j].flags;
            if (flags == 0 || flags == FLCTL_DONE)
                continue;

            if (flags == FLCTL_CONNECTING && (FD_ISSET(ctls[j].fd, &rs) || FD_ISSET(ctls[j].fd, &ws))) {
                int err = 0;
                socklen_t errlen = sizeof(err);
                if (getsockopt(ctls[j].fd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0) {
                    perror("getsockopt error!");
                    exit(1);
                } 
                if (err != 0) {
                    printf("connect error! %s\n", strerror(err));
                    exit(1);
                }
                FD_CLR(ctls[j].fd, &wset);
                write_get_cmd(&ctls[j]);
            } else if (flags == FLCTL_READING && FD_ISSET(ctls[j].fd, &rs)) {

                snprintf(filebuf, sizeof(filebuf), "%s/%s", download_dir, ctls[j].file);
            
                if ((filefd = open(filebuf, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
                    perror("open error!");
                    exit(1);
                }

                while ((nread = read(ctls[j].fd, buf, sizeof(buf))) > 0) {
                    if (write(filefd, buf, nread) != nread) {
                        perror("write error!");
                        exit(1);
                    }
                }
                ctls[j].flags = FLCTL_DONE;
                FD_CLR(ctls[j].fd, &rset);
                close(ctls[j].fd);
                nlefttoread--;
                nconn--;
                printf("j=%d download file [%s] succ! \n",j, ctls[j].file);
            }
        }
    }
    printf("total cost = %.2f ms\n", (curtimeus() - beg) / 1000.0);
    
    return 0;
}
