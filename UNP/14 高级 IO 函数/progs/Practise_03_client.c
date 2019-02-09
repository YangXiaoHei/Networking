#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "AddrTool.h"

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
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s :%s\n", hostname, service, gai_strerror(error));
        exit(1);
    }

    ressave = res;
    int fd = -1;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("socket fd create fail!");
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

    if (res == NULL) {
        printf("client connect to %s %s start fail!\n", hostname, service);
        exit(1);
    }

    char *dstInfo = getPeerInfo(fd);
    printf("connect to %s succ!\n", dstInfo);
    free(dstInfo);

    ssize_t nCanReadInRecvBuffer_MSG_PEEK = 0;
    ssize_t nCanReadInRecvBuffer_ioctl = 0;
    char buf[1024];
    ssize_t nread = 0;

    for (;;) {

        /* 来自 MSG_PEEK */
        if ((nCanReadInRecvBuffer_MSG_PEEK = recv(fd, buf, sizeof(buf), MSG_PEEK)) < 0) {
            perror("recv error!");
            break;
        }
        printf("there are [ %zd ] bytes in recv buffer -- by MSG_PEEK\n", nCanReadInRecvBuffer_MSG_PEEK);

        /* 来自 ioctl */
        if (ioctl(fd, FIONREAD, &nCanReadInRecvBuffer_ioctl) < 0) {
            perror("ioctl error!");
            break;
        }
        printf("there are [ %zd ] bytes in recv buffer -- by ioctl\n", nCanReadInRecvBuffer_ioctl);

        /* 不可能不等的，但是万一呢？一个强迫症患者的判断 
        【其实还真有可能不等，只要 recv 同时设置了 MSG_DONTWAIT】
        那么 recv 不会阻塞，会出现 recv 看了一眼，没数据，然后数据到了，ioctl 看一眼，发现哦哟有数据啊！
        ⚠️ 上述情况已经验证会出现
         */
        if (nCanReadInRecvBuffer_ioctl != nCanReadInRecvBuffer_MSG_PEEK) {
            printf("MSG_PEEK [%zd] != ioctl [ %zd ]\n", nCanReadInRecvBuffer_MSG_PEEK, nCanReadInRecvBuffer_ioctl);
            break;
        }
        if (nCanReadInRecvBuffer_ioctl == 0) {
            printf("server cut connection -- peek\n");
            break;
        }

        if ((nread = read(fd, buf, sizeof(buf))) < 0) {
            perror("read error!");
            exit(1);
        } else if (nread == 0) {
            printf("server cut connection -- read\n");
            break;
        }
        buf[nread] = 0;
        printf("read %zd bytes from recv buffer content = %s\n", nread, buf);

    }

    return 0;
}