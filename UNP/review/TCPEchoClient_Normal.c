#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "../tool/ReadWriteTool.h"
#include "../tool/CommonTool.h"
#include "../tool/TimeTool.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        mlogx("usage : %s <ip/hostname> <port/service>", argv[0]);
        exit(1);
    }

    int fd = -1;
    if ((fd = tcp_connect_cb(argv[1], argv[2], NULL)) < 0) {
        mlogx("tcp_connect_cb error!", strerror(errno));
        exit(1);
    }

    char buf[4096];
    ssize_t nwrite, nread;
    ssize_t ntowrite;
    int finished = 0;
    for (;;) {
        /* 从标准输入读取，并发送到服务器 */
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (ferror(stdin)) 
                mlogx("fgets error!");
            else
                mlogx("client send data finish!");
            break;
        }

        /* 写到服务器 */
        ntowrite = strlen(buf);
        if (writen(fd, buf, ntowrite) != ntowrite) {
            mlogx("writen error! %s", strerror(errno));
            break;
        }
        mlogx("send %zd bytes to server", ntowrite);

        /* 从服务器读取回显响应 */
        if ((nread = readn(fd, buf, ntowrite)) <= 0) {
            if (nread < 0)
                mlogx("readn error! %s", strerror(errno));
            else
                mlogx("server close connection");
            break;
        }
        mlogx("recv %zd bytes from server", ntowrite);

        /* 将回显响应写到标准输出 */
        if (writen(STDOUT_FILENO, buf, ntowrite) != ntowrite) {
            mlogx("writen error! %s", strerror(errno));
            break;
        }
    }
    mlogx("client close connection");
    close(fd);
}
