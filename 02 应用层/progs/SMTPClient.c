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


#define MOCK 0                                      /* 是否用预先设置好的 SMTP 握手控制命令 */
#define __DEST_MAIL_ADDR__              "???????"   /* 这里填目的邮箱，比如 569712232@qq.com */
#define __FROM_MAIL_ADDR__              "???????"   /* 这里填源邮箱，比如 yangxiaohei321123@163.com */
#define __DEST_MAIL_ADDR__BASE64__      "???????"   /* 这里填源邮箱 base64 后的字符串 */
#define __AUTH_VERIFY_CODE__BASE64__    "???????"   /* 这里填源邮箱打开 SMTP 服务的授权码 base64 后的字符串 */

static unsigned short port;
static char ip[128];

static char *cmd[] = {

    /* HELO xxx */
    "helo xxx\r\n",

    /* AUTH LOGIN */
    "auth login\r\n",

    /* 邮箱地址 base64 编码后的字符串 */
    __DEST_MAIL_ADDR__BASE64__ "\r\n",

    /* 授权码 base64 编码后的字符串 */
    __AUTH_VERIFY_CODE__BASE64__ "\r\n",

    /* 邮件起始地 */
    "mail from <" __FROM_MAIL_ADDR__ ">\r\n",

    /* 邮件目的地 */
    "rcpt to <" __DEST_MAIL_ADDR__ ">\r\n",

    /* DATA */
    "data\r\n",

    /* 邮件内容 */
    "from: < " __FROM_MAIL_ADDR__ ">\r\n"  
    "to: <" __DEST_MAIL_ADDR__ ">\r\n"
    "subject: I love you\r\n"
    "Content-Type:text/plain\t\n"
    "\r\nI love computer network"
    "\r\n.\r\n"
};

int DNS_lookup(const char *domain, char *ipbuf, size_t buf_len)
{
    char script[1024];
    int len = snprintf(script, sizeof(script), "nslookup %s | grep Address | sed '1d' | sed -n 's/Address: //; 1p'", domain);
    script[len] = 0;
    FILE *fp;
    if ((fp = popen(script, "r")) == NULL)
    {
        LOG("popen error");
        return -1;
    }
    if (fgets(ipbuf, buf_len, fp) == NULL)
    {
        if (ferror(fp))
        {
            LOG("fgets error");
            return -2;
        }
    }
    len = strlen(ipbuf);
    if (ipbuf[len - 1] == '\n')
        ipbuf[len - 1] = 0;
    return 0;
}

int TCP_connect(const char *ip, unsigned short port)
{
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &svraddr.sin_addr) < 0)
    {
        ERRLOG("inet_pton error");
        return -1;
    }
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error");
        return -2;
    }    
    if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("connect error");
        return -3;
    }
    return fd;
}

int recv_rsp(int fd, char *rspbuf, size_t rspbuf_len)
{
    int nread;
    if ((nread = read(fd, rspbuf, rspbuf_len)) < 0)
    {
        ERRLOG("read error");
        return -1;
    }
    else if (nread == 0)
    {
        LOG("SMTP server [%s:%d] close connection!", ip, port);
        close(fd);
    }
    rspbuf[nread] = 0;
    return nread;
}

int read_data(int fd, char *databuf, size_t databuf_len, char *lastcmd, size_t lastcmd_len)
{
    int len = 0;
    while (fgets(databuf + len, databuf_len - len, stdin) != NULL)
    {
        if (strstr(databuf, "\n.\n") != NULL)
            break;
        printf(">");
        len = strlen(databuf);
    }
    bzero(lastcmd, lastcmd_len);
    if (ferror(stdin))
    {
        LOG("fgets error");
        return -1;
    }
    return len;
}

int read_cmd(int fd, char *cmdbuf, size_t cmdbuf_len, char *lastcmd, size_t lastcmd_len)
{
    if (fgets(cmdbuf, cmdbuf_len, stdin) == NULL && ferror(stdin))
    {
        LOG("fgets error");
        return -1;
    }
    int len = snprintf(lastcmd, lastcmd_len, "%s", cmdbuf);
    lastcmd[len] = 0;
    return strlen(cmdbuf);
}

int send_SMTP_msg(int fd, char *msg, size_t msglen)
{
    int nwrite;
    if ((nwrite = write(fd, msg, msglen)) != msglen)
    {
        ERRLOG("write error");
        return -1;
    }
    return nwrite;
}

int mock_SMTP_msg(int fd, int i)
{
    int len, nwrite;
    char tmpbuf[1024];
    const char *begin = "******************** cmd *****************\n";
    const char *end   = "******************************************\n";

    len = snprintf(tmpbuf, sizeof(tmpbuf), "%s%s%s", begin, cmd[i], end);
    tmpbuf[len] = 0;

    write(STDOUT_FILENO, tmpbuf, len);

    if ((nwrite = write(fd, cmd[i], strlen(cmd[i]))) != strlen(cmd[i]))
    {
        ERRLOG("write error");
        return -1;
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    char recvbuf[1024];
    char lastcmd[256];
    int nread;
    setbuf(stdout, NULL);

#if MOCK
    int i = 0;
    int size = sizeof(cmd) / sizeof(cmd[0]);
#endif

    if (argc != 3)
    {
        LOG("usage : %s <#domain> <#port>\n", argv[0]);
        exit(1);
    }

    if (DNS_lookup(argv[1], ip, sizeof(ip)) < 0)
    {
        LOG("DNS_lookup error!");
        exit(1);
    }
    LOG("DNS lookup result : %s", ip);

    port = atoi(argv[2]);
    int fd;
    if ((fd = TCP_connect(ip, port)) < 0)
    {
        LOG("TCP_connect error!");
        exit(1);
    }
    LOG("connect [%s:%d] succ!", ip, port);

    while (1)
    {
        if ((nread = recv_rsp(fd, recvbuf, sizeof(recvbuf))) <= 0)
            break;

        LOG("recv %d bytes response from [%s:%d]", nread, ip, port);

        if (fputs(recvbuf, stdout) == EOF)
        {
            LOG("fputs error!");
            exit(1);
        }

#if MOCK
        if (i == size)
            break;

        mock_SMTP_msg(fd, i++);
#else
        printf(">");

        if (strncmp(lastcmd, "data", 4) == 0)
        {
            if (read_data(fd, recvbuf, sizeof(recvbuf), lastcmd, sizeof(lastcmd)) < 0)
            {
                LOG("read_data error!");
                goto err;
            }
        }
        else
        {
            if (read_cmd(fd, recvbuf, sizeof(recvbuf), lastcmd, sizeof(lastcmd)) < 0)
            {
                LOG("read_data error!");
                goto err;
                
            }
        }
        if (send_SMTP_msg(fd, recvbuf, strlen(recvbuf)) < 0)
        {
            LOG("send_SMTP_msg error!");
            goto err;
        }
        continue;
err:
        close(fd);
        break;
#endif
    }

    return 0;
}