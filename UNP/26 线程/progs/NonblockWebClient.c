#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../../tool/TimeTool.h"

#define F_READING       1
#define F_CONNECTING    2
#define F_DONE          3

#define GET_CMD "GET %s HTTP/1.1\r\n\r\n"

struct Session {
    int sockfd;
    int filefd;
    char *file;
    int flags;
    const char *hostname;
    const char *service;
};

static struct Session *sessions;
static fd_set rset, wset; 
static fd_set rallset, wallset;

static int maxfd;

void nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

void send_get_req(struct Session *session)
{
    session->flags = F_READING;
    char buf[256];
    snprintf(buf, sizeof(buf), GET_CMD, session->file);
    if (write(session->sockfd, buf, strlen(buf)) != strlen(buf)) {
        perror("write error!");
        exit(1);
    }
}

void start_conn(struct Session *session)
{
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    int error = 0;
    const char *hostname = session->hostname;
    const char *service = session->service;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    int fd = -1;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("socket fd create fail!");
        exit(1);
    }
    nonblock(fd);
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect error!");
            exit(1);
        }
        FD_SET(fd, &rallset);
        FD_SET(fd, &wallset);
        session->flags = F_CONNECTING;
        session->sockfd = fd;
    } else {
        FD_SET(fd, &rset);
        session->sockfd = fd;
        send_get_req(session);
    }
    if (fd > maxfd)
        maxfd = fd;
    
    freeaddrinfo(res);
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        printf("usage : %s <hostname/ip> <service/port> <maxconn> [file1] [file2]...\n", argv[0]);
        exit(1);
    } 
    const char *hostname = argv[1];
    const char *service = argv[2];
    int maxconn = atoi(argv[3]);
    int nfile = argc - 4;
    if ((sessions = malloc(nfile * sizeof(struct Session))) == NULL) {
        printf("malloc error!");
        exit(1);
    }
    for (int i = 0; i < nfile; i++) {
        sessions[i].sockfd = -1;
        sessions[i].filefd = -1;
        sessions[i].flags = 0;
        sessions[i].file = argv[i + 4];
        sessions[i].hostname = hostname;
        sessions[i].service = service;
        if (access(sessions[i].file, F_OK) == 0)
            remove(sessions[i].file);
        if ((sessions[i].filefd = open(sessions[i].file, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
            printf("open file %s fail!", sessions[i].file);
            exit(1);
        }
    }
    int nleft = nfile;
    int nconn = 0;
    char buf[8192];
    ssize_t nread, nwrite;
    ssize_t ntoconn = nfile;

    long beg = curtimeus();
    while (nleft > 0) {
        while (nconn < maxconn && ntoconn > 0) {
            int i;
            for (i = 0; i < nfile; i++) 
                if (sessions[i].flags == 0)
                    break;
            start_conn(&sessions[i]);
            nconn++;
            ntoconn--;
        } 
        
        rset = rallset;
        wset = wallset; 
        select(maxfd + 1, &rset, &wset, NULL, NULL);
        
        for (int i = 0; i < nfile; i++) {
            int fd = sessions[i].sockfd, flags = sessions[i].flags;
            char *file = sessions[i].file;
           
            if (flags == 0 || flags == F_DONE)
                continue;
            
            if (flags == F_CONNECTING  && (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))) {
                int error = 0;
                socklen_t len = sizeof(error);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    printf("connect to server for %s fail!, reason = %s", file, strerror(error));
                    exit(1);
                }
                printf("connection to %s %s for %s succ!\n", hostname, service, file);
                FD_CLR(fd, &wallset);
                send_get_req(&sessions[i]);
            } else if (flags == F_READING && FD_ISSET(fd, &rset)) {
                 if ((nread = read(fd, buf, sizeof(buf))) < 0) {
                     if (errno == EAGAIN) 
                        continue;
                    printf("file %s download fail!\n", file);
                    close(fd);
                    sessions[i].flags = 0;
                    FD_CLR(fd, &rallset); 
                    nconn--;
                } else if (nread == 0) {
                    sessions[i].flags = F_DONE;
                    FD_CLR(fd, &rallset);
                    printf("file %s donwload succ!\n", file);
                    nleft--;
                    nconn--;
                    close(fd);
                } else {
                    /*
                     * printf("file %s read %zd bytes from server\n", file, nread);
                     */
                    if (write(sessions[i].filefd, buf, nread) != nread) {
                        perror("write error!");
                        exit(1);
                    }
                }
            }
        }
    }
    printf("all download finished! total_cost = %.2f ms\n", (curtimeus() - beg) / 1000.0);
}
