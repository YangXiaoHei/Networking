#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#define PATH_PREFIX "/Users/bot/Desktop/Networking/Networking/resource/"

ssize_t YHLog(int line, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    if (errno != 0)
        len += snprintf(buf + len, sizeof(buf) - len, "【%s】", strerror(errno));
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d]:%s\n", __FILE__, __FUNCTION__, line, buf);
    va_end(ap);
    return len;
}

#define LOG(_format_, ...) YHLog(__LINE__, _format_, ##__VA_ARGS__)

int parse_http_protocol(const char *protocol_str, 
                        char *method, 
                        size_t mlen, 
                        char *url, 
                        size_t ulen)
{
    size_t len = strlen(protocol_str);

    char *p = (char *)protocol_str, *q;
   
    if ((q = strchr(protocol_str, ' ')) == NULL)
        return -1;

    if (q - p + 1 > mlen)
        return -2;

    memcpy(method, protocol_str, q - p);
    method[q - p] = 0;

    p = ++q;
    if ((q = strchr(q, ' ')) == NULL)
        return -3;

    if (q - p + 1 > ulen)
        return -4;

    memcpy(url, p, q - p);
    url[q - p] = 0;
    return 0;
}

int do_method(const char *method, const char *url, char *content, size_t *clen)
{
    /* no supported yet */
    if (strcmp(method, "GET") != 0)
    {
        bzero(content, *clen);
        *clen = 0;
        return -1; 
    }

    if (mkdir(PATH_PREFIX, 777) != 0 && errno != EEXIST)
    {
        perror("mkdir error");
        bzero(content, *clen);
        *clen = 0;
        return -2;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s%s", PATH_PREFIX, url);

    int fd;
    if ((fd = open(path, O_RDONLY)) < 0)
    {
        perror("open error");
        exit(1);
    }
    ssize_t nread;
    if ((nread = read(fd, content, *clen - 1)) < 0)
    {
        LOG("%s read error", path);
        bzero(content, *clen);
        *clen = 0;
        return -3;
    }
    content[nread] = 0;
    *clen = nread;

    return 0;
}

int packbuf(char *dst, size_t dst_len, const char *data, size_t data_len)
{
    int status_code = data_len > 0 ? 200 : 404;
    const char *phrase = status_code == 200 ? "OK" : "Not found";
    int len = snprintf(dst, dst_len, "%s %d %s\r\n", "HTTP/1.1", status_code, phrase);
    len += snprintf(dst + len, dst_len - len, "Content-Type:text/html\r\nContent-Length:%zd\r\n", data_len);
    len += snprintf(dst + len, dst_len - len, "\r\n");
    memcpy(dst + len, data, data_len);
    len += data_len;
    return len;
}

int sendbackrsp(int sockfd, const char *rsp, size_t rsp_len)
{   
    LOG("sendbackrsp begin");
    ssize_t nwrite;
    if ((nwrite = write(sockfd, rsp, rsp_len)) < 0)
    {
        if (errno != EINTR)
        {
            perror("write error");
            return -1;
        }
    }
    LOG("sendbackrsp end");
    return nwrite;
}

int start_service(unsigned short port)
{
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        goto err;
    }
    int reuse = 0;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopet error\n");
        goto err;
    }
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        perror("bind error");
        goto err;
        
    }
    if (listen(listenfd, 1) < 0)
    {
        perror("listen error");
        goto err;
    }
    return listenfd;
err:
    return -1;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("usage : %s <#port>\n", argv[0]);
        exit(1);
    }

    unsigned short port = atoi(argv[1]);
    int listenfd;
    if ((listenfd = start_service(port)) < 0)
    {
        printf("start_service error!\n");
        exit(1);
    }

    int connfd, nread;
    struct sockaddr_in cliaddr;
    char ipbuf[128], reqbuf[1 << 10];
    socklen_t len = sizeof(cliaddr);

    char method[32], url[512];
    char databuf[1 << 20], rspbuf[1 << 20];
    size_t databuf_len = sizeof(databuf), rspbuf_len = sizeof(rspbuf);
    while (1)
    {
        printf("wait to accept...\n");
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
        {
            perror("accept error");
            exit(1);
        }
        const char *ip = inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf));
        unsigned short port = ntohs(cliaddr.sin_port);
        printf("connection received from [%s:%d]\n", ip, port);

        while (1)
        {
            printf("wait to received data from [%s:%d]...\n", ip, port);
            if ((nread = read(connfd, reqbuf, sizeof(reqbuf))) < 0)
            {
                if (errno != EINTR)
                {
                    perror("read error occur");
                    goto handle_http_err;
                }
            }
            else if (nread == 0)
            {
                printf("peer closed socket, read EOF\n");
                goto handle_http_err;
            }
            reqbuf[nread] = 0;

            printf("received data content:\n");
            printf("*****************************************\n");
            printf("%s", reqbuf);
            printf("*****************************************\n");
            printf("begin to parse HTTP protocol\n");

            if (parse_http_protocol(reqbuf, method, sizeof(method), url, sizeof(url)) < 0)
            {
                printf("parse HTTP protocol fail!, close peer socket\n");
                goto handle_http_err;
            }

            if (do_method(method, url, databuf, &databuf_len) < 0)
            {
                printf("do HTTP method fail!, close peer socket\n");
                goto handle_http_err;
            }

            rspbuf_len = packbuf(rspbuf, rspbuf_len, databuf, databuf_len);

            printf("packbuf end\n");

            if (sendbackrsp(connfd, rspbuf, rspbuf_len) < 0)
            {
                printf("send back rsp fail!, close peer socket\n");
                goto handle_http_err;
            }

            continue;

handle_http_err:
            close(connfd);
            break;
        }
    }
    return 0;
}