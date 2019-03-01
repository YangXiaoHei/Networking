#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>

static pthread_t tid;
static ssize_t total_read;

int tcp_connect(const char *ip, unsigned short port)
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_port = htons(port);
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
        return -2;
    return fd;
}

void *recvTextFromServer(void *arg)
{
    int fd = (int)arg;
    char buf[1024];
    ssize_t nwrite, nread;
    /* setbuf(stdout, NULL); */
    for (;;) {
        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            perror("server terminated abnormal");
            break;
        } else if (nread == 0) {
            fprintf(stderr, "server cut connection, total_read=%zd\n", total_read);
            break;
        }
        total_read += nread;
        /* fprintf(stderr, "recv %zd bytes from server, total_read=%zd\n", nread, total_read); */
        /* buf[nread] = 0; */
        if ((nwrite = write(STDOUT_FILENO, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
        /*
         * printf("%s", buf);
         */
        /*
         * if (fputs(buf, stdout) == EOF) {
         *     if (ferror(stdout)) {
         *         printf("stdout error!");
         *         exit(1);
         *     }
         * }
         */
    }
    return (void *)NULL;
}

void sendTextToServer(int fd)
{
    char buf[1024];
    ssize_t nread, nwrite;

    int error = 0;
    if ((error = pthread_create(&tid, NULL, recvTextFromServer, (void *)fd)) != 0) {
        perror("pthread_Create fail!");
        exit(1);
    }

    while ((nread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if ((nwrite = write(fd, buf, nread)) != nread) {
            perror("write error!");
            exit(1);
        }
    }
    if (nread == 0) {
        shutdown(fd, SHUT_WR);
        printf("client no data send\n");
    } else {
        perror("read error!");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    /* setbuf(stdout, NULL); */
    int fd = -1;
    if ((fd = tcp_connect(argv[1],atoi(argv[2]))) < 0) {
        perror("tcp connect error!");
        exit(1);
    }
    sendTextToServer(fd);
    
    int n = 0;
    if ((n = pthread_join(tid, NULL)) != 0) {
        fprintf(stderr, "pthread_join fail! : %s\n", strerror(n));
        exit(0);
    }   
 
}
