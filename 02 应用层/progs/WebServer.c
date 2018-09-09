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
#include <time.h>

enum {
    WEB_SVR_RET_CODE__NOT_FOUND         = 100,
    WEB_SVR_RET_CODE__BAD_REQUEST       = 101,
    WEB_SVR_RET_CODE__NOT_SUPPORTED     = 102,
    WEB_SVR_RET_CODE__INTERNAL          = 103,
    WEB_SVR_RET_CODE__INTERNAL_PERM     = 104, 
    WEB_SVR_RET_CODE__INTERNAL_BADF     = 105,
    WEB_SVR_RET_CODE__INTERNAL_RD       = 106,
    WEB_SVR_RET_CODE__INTERNAL_BUFSZ    = 107
};

#define PATH_PREFIX "/Users/bot/Desktop/Networking/Networking/02 应用层/resource/"

ssize_t YHLog(int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s\n", __FILE__, fun, line, buf);
    va_end(ap);
    return len;
}

#define LOG(_format_, ...) YHLog(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)

int parse_http_protocol(const char *protocol_str, char *method, size_t mlen, char *url, size_t ulen)
{
    size_t len = strlen(protocol_str);

    char *p = (char *)protocol_str, *q;
    
    /* Get HTTP method field */   
    if ((q = strchr(protocol_str, ' ')) == NULL)
        return WEB_SVR_RET_CODE__BAD_REQUEST;

    if (q - p + 1 > mlen)
        return WEB_SVR_RET_CODE__INTERNAL_BUFSZ;

    memcpy(method, protocol_str, q - p);
    method[q - p] = 0;

    /* Get HTTP url field */
    p = ++q;
    if ((q = strchr(q, ' ')) == NULL)
        return WEB_SVR_RET_CODE__BAD_REQUEST;

    if (q - p + 1 > ulen)
        return WEB_SVR_RET_CODE__INTERNAL_BUFSZ;

    memcpy(url, p, q - p);
    url[q - p] = 0;
    return 0;
}

int do_method(const char *method, const char *url, char *content, size_t *clen, struct stat *st)
{
    int retcode = 0;

    /* Just implement GET method yet */
    if (strcmp(method, "GET") != 0)
    {
        retcode = WEB_SVR_RET_CODE__NOT_SUPPORTED;
        goto err;
    }

    if (mkdir(PATH_PREFIX, 777) != 0 && errno != EEXIST)
    {
        retcode = WEB_SVR_RET_CODE__NOT_FOUND;
        goto err;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s%s", PATH_PREFIX, url);

    int fd;
    if ((fd = open(path, O_RDONLY)) < 0)
    {
        switch(errno)
        {
            case ENOENT:
                retcode = WEB_SVR_RET_CODE__NOT_FOUND;
                break;
            case EPERM:
                retcode = WEB_SVR_RET_CODE__INTERNAL_PERM;
                break;
            case EBADF:
                retcode = WEB_SVR_RET_CODE__INTERNAL_BADF;
                break;
            default:
                retcode = WEB_SVR_RET_CODE__INTERNAL;
        }
        goto err;
    }
    if (lstat(path, st) < 0)
    {
        switch(errno)
        {
            case ENOENT:
                retcode = WEB_SVR_RET_CODE__NOT_FOUND;
                break;
            case EPERM:
                retcode = WEB_SVR_RET_CODE__INTERNAL_PERM;
                break;
            case EBADF:
                retcode = WEB_SVR_RET_CODE__INTERNAL_BADF;
                break;
            default:
                retcode = WEB_SVR_RET_CODE__INTERNAL;
        }
        goto err;
    }
    ssize_t nread;
    if ((nread = read(fd, content, *clen - 1)) < 0)
    {
        retcode = WEB_SVR_RET_CODE__INTERNAL_RD;
        goto err;
    }
    content[nread] = 0;
    *clen = nread;
    return 0;
err:
    /* clear response buf */
    bzero(content, *clen);
    *clen = 0;
    return retcode;
}

int packbuf(int retcode, struct stat *st, char *dst, size_t dst_len, const char *data, size_t data_len)
{
    int len;
    int status_code;
    const char *phrase;
    switch(retcode)
    {
        /* OK */
        case 0 :
            status_code = 200;
            phrase = "OK";
            break;

        /* Not Found */
        case WEB_SVR_RET_CODE__NOT_FOUND :
            status_code = 404;
            phrase = "Not Found";
            break;

        /* Parse Fail */
        case WEB_SVR_RET_CODE__BAD_REQUEST :
            status_code = 400;
            phrase = "Bad Request";
            break;

        /* Internal error */
        default:
            status_code = 500;
            phrase = "Internal Error";
    }

    /* status header line */
    len = snprintf(dst, dst_len, "%s %d %s\r\n", "HTTP/1.1", status_code, phrase);

    /* response header line */
    len += snprintf(dst + len, dst_len - len, "Content-Type:text/html\r\nContent-Length:%zd\r\n", data_len);
    len += snprintf(dst + len, dst_len - len, "Server:YangXiaoHei_WebServer_v1.0\r\n");
    struct tm *tmbuf = localtime(&st->st_mtimespec.tv_sec);
    len += snprintf(dst + len, dst_len - len, "Last-Modified:%04d/%02d/%02d [%0d:%0d:%0d]\r\n", 
                    tmbuf->tm_year + 1900, 
                    tmbuf->tm_mon + 1, 
                    tmbuf->tm_mday, 
                    tmbuf->tm_hour, 
                    tmbuf->tm_min,
                    tmbuf->tm_sec);
    len += snprintf(dst + len, dst_len - len, "Connection:keep-alive\r\n");
    len += snprintf(dst + len, dst_len - len, "\r\n");
    memcpy(dst + len, data, data_len);
    len += data_len;
    return len;
}

int sendbackrsp(int sockfd, const char *rsp, size_t rsp_len)
{   
    ssize_t nwrite;
    if ((nwrite = write(sockfd, rsp, rsp_len)) < 0)
    {
        if (errno != EINTR)
        {
            perror("sendback rsp error");
            return -1;
        }
    }
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
        perror("setsockopet error");
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
        LOG("usage : %s <#port>", argv[0]);
        exit(1);
    }

    unsigned short port = atoi(argv[1]);
    int listenfd;
    if ((listenfd = start_service(port)) < 0)
    {
        LOG("start_service error!");
        exit(1);
    }

    int connfd, nread;
    struct sockaddr_in cliaddr;
    char ipbuf[128], reqbuf[1 << 10];
    socklen_t len = sizeof(cliaddr);

    char method[32], url[512];
    char databuf[1 << 20], rspbuf[1 << 20];
    size_t databuf_len, rspbuf_len;
    while (1)
    {
        LOG("wait to accept...");
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
        {
            perror("accept error");
            exit(1);
        }
        const char *ip = inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf));
        unsigned short port = ntohs(cliaddr.sin_port);
        LOG("connection received from [%s:%d]", ip, port);

        while (1)
        {
            databuf_len = sizeof(databuf);
            rspbuf_len = sizeof(rspbuf);
            LOG("wait to received data from [%s:%d]...", ip, port);
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
                LOG("peer closed socket, read EOF");
                goto handle_http_err;
            }
            reqbuf[nread] = 0;

            LOG("received data content:");
            LOG("*****************************************");
            LOG("%s", reqbuf);
            LOG("*****************************************");
            LOG("begin to parse HTTP protocol");

            int retcode = 0;
            if ((retcode = parse_http_protocol(reqbuf, method, sizeof(method), url, sizeof(url))) < 0)
            {
                LOG("parse HTTP protocol fail!, close peer socket");
                goto handle_http_err;
            }

            struct stat st;
            if ((retcode = do_method(method, url, databuf, &databuf_len, &st)) < 0)
            {
                LOG("do HTTP method fail!, close peer socket");
                goto handle_http_err;
            }

            rspbuf_len = packbuf(retcode, &st, rspbuf, rspbuf_len, databuf, databuf_len);

            if (sendbackrsp(connfd, rspbuf, rspbuf_len) < 0)
            {
                LOG("send back rsp fail!, close peer socket");
                goto handle_http_err;
            }
            LOG("sendback succ!");

            continue;

handle_http_err:
            close(connfd);
            break;
        }
    }
    return 0;
}