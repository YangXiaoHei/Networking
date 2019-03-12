#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>

/*
 * @param fd  文件描述符
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @param timeout 超时时长
 * @return -1 可能出错的情况有: 1) 超时时间到达,
 *                              2) 阻塞等待可读事件发生时被信号中断,
 *                              3) 可读事件发生，然而数据被其他进程或线程读走
 *                              4) 其他
 */
ssize_t read_timeout(int fd, char *buf, size_t len, int timeout)
{
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    
    struct timeval *tv_ptr = NULL;
    struct timeval tv;
    if (timeout >= 0) {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        tv_ptr = &tv;
    }
    int nready = 0;
    if ((nready = select(fd + 1, &rset, NULL, NULL, tv_ptr)) == 0) {
        errno = ETIMEDOUT;
        return -1;
    } else if (nready < 0)
       return -1; 
    
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    /*
     * 防止数据被其他线程或进程抢先读走而造成阻塞，临时设置描述符为非阻塞
     * 其实也可以直接使用 recv(fd, buf, len, MSG_DONTWAIT)
     */
    ssize_t nread = read(fd, buf, len);

    fcntl(fd, F_SETFL, flags);
    return nread;
}

int main(int argc, char *argv[])
{
    int fd = -1;
    if ((fd = udp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        logx("udp_connect error! %s", strerror(errno));
        exit(1);
    }

    struct sockaddr_in checkaddr;
    char buf[8192];
    ssize_t nread, nwrite;
    for (;;) {
        if ((nread = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
            logx("read error! %s", strerror(errno));
            break;
        }
        if (write(fd, buf, nread) != nread) {
            logx("write error! %s", strerror(errno));
            break;
        }
        if ((nread = read_timeout(fd, buf, sizeof(buf), 5)) == 0) {
            if (errno != 0) 
                logx("read_timeout error! %s", strerror(errno));
            else
                logx("udp data no payload");
            continue;
        } else if (nread < 0) {
            logx("read_timeout error! %s", strerror(errno));
            break;
        }
        if (write(STDOUT_FILENO, buf, nread) != nread) {
            logx("write error! %s", strerror(errno));
            break;
        }
    }
}
