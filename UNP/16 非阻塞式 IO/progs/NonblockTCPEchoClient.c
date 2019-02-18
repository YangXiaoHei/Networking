#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <stdarg.h>

char *tcpdump_timestamp(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        fprintf(stderr, "gettimeofday error!");
        exit(1);
    }
    static char str[30];
    /* Tue Feb 12 09:17:48 2019 */
    char *ptr = ctime(&tv.tv_sec);
    strcpy(str, ptr + 11);
    snprintf(str + 8, sizeof(str) - 8, ".%06ld", (long)tv.tv_usec);
    return str;
}

ssize_t logx(const char *fmt, ...) 
{
    size_t n = 0;
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    n = snprintf(buf, sizeof(buf), "%s: ", tcpdump_timestamp());
    vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
    va_end(ap);
    return fprintf(stderr, "%s\n", buf);
}

void nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int max(int a, int b) 
{
    return a > b ? a : b;
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *service = argv[2];

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        logx("getaddrinfo error! %s %s : %s", hostname, service, gai_strerror(error));
        exit(1);
    }

    int fd = -1;
    do {    
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket error!");
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("connect error!");
            close(fd);
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        logx("client start fail!");
        exit(1);
    }

    ressave = res;

    fd_set rset, wset;
    const int size = 5 << 10;
    char to[size], from[size];
    char *to_send, *to_finish, *from_send, *from_finish;
    to_send = to_finish = to;
    from_send = from_finish = from;
    int stdineof = 0;

    nonblock(fd);
    nonblock(STDOUT_FILENO);
    nonblock(STDIN_FILENO);

    int maxfd = max(max(fd, STDOUT_FILENO), STDIN_FILENO);

    ssize_t nwrite = 0, nread = 0, ntowrite = 0;
    for (;;) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);

        /* 标准输入还有内容需要读取，并且待发送缓冲区未填满，标准输入的可读事件 */
        if (stdineof == 0 && to_finish < &to[size])
            FD_SET(STDIN_FILENO, &rset);

        /* 缓冲区中还有待发送的数据，监听套接字的可写事件 */
        if (to_send < to_finish)
            FD_SET(fd, &wset);

        /* 接收缓冲区还有空余，监听套接字的可读事件 */
        if (from_finish < &from[size])
            FD_SET(fd, &rset);

        /* 接收缓冲区还有待读取的数据，监听标准输出的可写事件 */
        if (from_send < from_finish)
            FD_SET(STDOUT_FILENO, &wset);

        select_again:
        if (select(maxfd + 1, &rset, &wset, NULL, NULL) < 0) {
            if (errno == EINTR)
                goto select_again;
            perror("select error!");
            exit(1);
        }

        /* 从标准输入读到缓冲区 */
        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if ((nread = read(STDIN_FILENO, to_send, &to[size] - to_send)) < 0) {
                if (errno != EAGAIN) {
                    perror("read error!");
                    exit(1);
                } 
            /* 输入了 ctrl+d, 将标准输入关闭标志记上，但先不要 shutdown, 除非从标准输入读到的数据已经全部发完 */
            } else if (nread == 0) {
                logx("all data from client finished!");
                stdineof = 1;
                if (to_send == to_finish) {
                    shutdown(fd, SHUT_WR);
                    logx("send FIN to server");
                }
            } else {
                logx("read %zd bytes from stdin", nread);
                to_send += nread;
                FD_SET(fd, &wset);
            }
        }
        
        if (FD_ISSET(fd, &rset)) {
            if ((nread = read(fd, from_send, &from[size] - from_send)) < 0) {
                if (errno != EAGAIN) {
                    perror("read error!");
                    exit(1);
                }
            } else if (nread == 0) {
                if (stdineof) {
                    logx("server cut connection");
                } else {
                    logx("server terminated abnormally"); 
                }
                break;
            } else {
                logx("read %zd bytes from server", nread);
                from_send += nread;
                FD_SET(STDOUT_FILENO, &wset);
            }
        }

        if (FD_ISSET(STDOUT_FILENO, &wset) && (ntowrite = from_send - from_finish) > 0) {
            if ((nwrite = write(STDOUT_FILENO, from_finish, ntowrite)) < 0) {
                if (errno != EAGAIN) {
                    perror("write error!");
                    exit(1);
                }
            } else {
                logx("wrote %zd bytes to stdout", nwrite);
                from_finish += nwrite;
                if (from_finish == from_send) 
                    from_send = from_finish = from;
            }
        }

        if (FD_ISSET(fd, &wset) && (ntowrite = to_send - to_finish) > 0) {
            if ((nwrite = write(fd, to_finish, ntowrite)) < 0) {
                if (errno != EAGAIN) {
                    perror("write error!");
                    exit(1);
                }
            } else {
                logx("wrote %zd bytes to server", nwrite);
                to_finish += nwrite;
                if (to_send == to_finish) {
                    to_send = to_finish = to;
                    if (stdineof) {
                        shutdown(fd, SHUT_WR);
                        logx("halfclose, send FIN to server");
                    }
                }
            }
        }
    }
    return 0;
}
