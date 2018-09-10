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

ssize_t YHLog_err(int line, const char *fun, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[512];
    ssize_t len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[len] = 0;
    len = fprintf(stderr, "[%s:%s:%d] %s: %s\n", __FILE__, fun, line, buf, strerror(errno));
    va_end(ap);
    return len;
}

#define LOG(_format_, ...) YHLog(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)
#define ERRLOG(_format_, ...) YHLog_err(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)

#define MOCK 1

#define NETEASE_MOCK 1
#define QQ_MOCK 0

static char *cmd[] = {

#if NETEASE_MOCK
    "helo yanghan\r\n",
    "auth login\r\n",
    "eWFuZ3hpYW9oZWkzMjExMjNAMTYzLmNvbQ==\r\n",
    "eWFuZ2hhbjEyMw==\r\n",
    "mail from <yangxiaohei321123@163.com>\r\n",
    "rcpt to <569712232@qq.com>\r\n"
#elif QQ_MOCK
    "helo yanghan\r\n",
    "auth login\r\n",
    "NTY5NzEyMjMyQHFxLmNvbQ==\n",
    "c3Jid2VocWt4dGVuYmVhYg==\n",
    "mail from <569712232@qq.com>\r\n",
    "rcpt to <yangxiaohei321123@163.com>\r\n"
#endif

};

void automaticly_input(int fd, const char *ip, unsigned short port)
{
    ssize_t nread = 0, ntowrite = 0, nwrite = 0;
    char recvbuf[1024];
    int size = sizeof(cmd) / sizeof(cmd[0]);
    for (int i = 0; i < size; i++)
    {
        if ((nread = read(fd, recvbuf, sizeof(recvbuf))) < 0)
        {
            ERRLOG("read error");
            exit(1);
        }
        else if (nread == 0)
        {
            LOG("SMTP server [%s:%d] close connection!", ip, port);
            close(fd);
            break;
        }
        recvbuf[nread] = 0;
        LOG("recv %d bytes from [%s:%d]", nread, ip, port);

        /* echo received data */
        if (fputs(recvbuf, stdout) == EOF)
        {
            LOG("fputs error!");
            exit(1);
        }

        printf(">%s", cmd[i]);
        int nwrite;
        int ntowrite = strlen(cmd[i]);
        if ((nwrite = write(fd, cmd[i], ntowrite)) != ntowrite)
        {
            ERRLOG("write error");
            exit(1);
        }
    }
}

int main(int argc, char const *argv[])
{

    if (argc != 3)
    {
        LOG("usage : %s <#domain> <#port>\n", argv[0]);
        exit(1);
    }

    char script[1024];
    int len = snprintf(script, sizeof(script), "nslookup %s | grep Address | sed '1d' | head -n1 | sed 's/Address: //'", argv[1]);
    script[len] = 0;
    FILE *fp;
    if ((fp = popen(script, "r")) == NULL)
    {
        LOG("popen error");
        exit(1);
    }
    char ip[128];
    if (fgets(ip, sizeof(ip), fp) == NULL)
    {
        if (ferror(fp))
        {
            LOG("fgets error");
            exit(1);
        }
    }
    len = strlen(ip);
    if (ip[len - 1] == '\n')
        ip[len - 1] = 0;
    LOG("DNS lookup result : %s", ip);

    unsigned short port = atoi(argv[2]);
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &svraddr.sin_addr) < 0)
    {
        ERRLOG("inet_pton error");
        exit(1);
    }

    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error");
        exit(1);
    }    
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("connect error");
        exit(1);
    }
    LOG("connect [%s:%d] succ!", ip, port);

    ssize_t nread = 0, ntowrite = 0, nwrite = 0;
    char recvbuf[1024];
    setbuf(stdout, NULL);

#if MOCK
    automaticly_input(fd, ip, port);
#endif

    ssize_t stored_len = 0;
    char lastcmd[256];
    while (1)
    {
        /* --------------------------- recv ---------------------------- */

        if ((nread = read(fd, recvbuf, sizeof(recvbuf))) < 0)
        {
            ERRLOG("read error");
            exit(1);
        }
        else if (nread == 0)
        {
            LOG("SMTP server [%s:%d] close connection!", ip, port);
            close(fd);
            break;
        }
        recvbuf[nread] = 0;
        LOG("recv %d bytes from [%s:%d]", nread, ip, port);

        /* echo received data */
        if (fputs(recvbuf, stdout) == EOF)
        {
            LOG("fputs error!");
            exit(1);
        }

        /* --------------------------- send ---------------------------- */

        printf(">");
        if (strncmp(lastcmd, "data", 4) == 0)
        {
            int len = 0;
            while (fgets(recvbuf + len, sizeof(recvbuf) - len, stdin) != NULL)
            {
                if (strstr(recvbuf, "\n.\n") != NULL)
                    break;
                printf(">");
                len = strlen(recvbuf);
            }
        }
        else
        {
            if (fgets(recvbuf, sizeof(recvbuf), stdin) == NULL && ferror(stdin))
            {
                LOG("fgets error");
                exit(1);
            }
            int len = snprintf(lastcmd, sizeof(lastcmd), "%s", recvbuf);
            if (lastcmd[len - 1] == '\n')
                lastcmd[len - 1] = 0;
        }

        ntowrite = strlen(recvbuf);
        if ((nwrite = write(fd, recvbuf, ntowrite)) != ntowrite)
        {
            ERRLOG("write error");
            exit(1);
        }
    }

    return 0;
}