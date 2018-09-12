#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

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

#define LOG(_format_, ...)      YHLog(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)
#define ERRLOG(_format_, ...)   YHLog_err(__LINE__, __FUNCTION__, _format_, ##__VA_ARGS__)

#define _DEBUG_ 1
#define MAX_BUFSZ (1 << 10)

enum {
    WEB_PROXY_SVR__PROCEDURE__FORWARD_TO_ORG,
    WEB_PROXY_SVR__PROCEDURE__SENDBACK_TO_CLIENT
};

struct session_t {
    unsigned long proc_begtime;
    unsigned long proc_endtime;

    int procedure_cmd;
    int seqno;

    int connfd;
    int listenfd;

    int svr_fd;

    const char *cli_ip;
    unsigned short cli_port;

    char req_buf[MAX_BUFSZ];
    char req_buf_len;
    char rsp_buf[MAX_BUFSZ];
    char rsp_buf_len;

    int is_using;
};

struct task_t {
    struct session_t session;
};

struct work_queue_node {
    struct task_t *task;
    struct work_queue_node *next;
    struct work_queue_node *prev;
};
struct work_queue {
    struct work_queue_node *header;
    struct work_queue_node *tailer;
    int size;
    int capacity;
    pthread_cond_t empty;
    pthread_cond_t full;
    pthread_mutex_t lock;
};

/* 工作队列 */
static struct work_queue *task_queue;
static struct work_queue *idle_pool;

static pthread_t query_net_io;
static pthread_t search_net_io;
static pthread_t work_process[10];

/* 监听可读事件用的数组 */
pthread_rwlock_t global_fds_lock = PTHREAD_RWLOCK_INITIALIZER;
static struct task_t *check_readable_fds[1024];

/* 监听的端口 */
unsigned short port;

/* 初始服务器 ip 和 port */
const char *trans_ip;
unsigned short trans_port;

/* 全局递增序列号 */
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static int global_value = 1;
int get_value()
{
    pthread_mutex_lock(&global_lock);
    int val = global_value++;
    pthread_mutex_unlock(&global_lock);
    return val;
}

/* 获取当前时间 us */
unsigned long curtime_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct task_t* new_task()
{
    struct task_t* t;
    if ((t = malloc(sizeof(struct task_t))) == NULL)
        return NULL;
    return t;
}

void reset_task(struct task_t *task)
{
    bzero(task, sizeof(struct task_t));
}

/* 设置非阻塞文件描述符 */
void setnonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

/* 清除文件描述符非阻塞标志 */
void clrnonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

// void queue_display(struct work_queue *queue, const char *format, ...)
// {
//     pthread_mutex_lock(&queue->lock);
//     struct work_queue_node *cur;
//     va_list ap;
//     va_start(ap, format);
//     vprintf(format, ap);
//     va_end(ap);
//     printf("%d {", queue->size);
//     for (cur = queue->header->next; cur != queue->tailer; cur = cur->next)
//     {
//         if (cur->next == queue->tailer)
//             printf(" %d", cur->task->seq);
//         else
//             printf(" %d,", cur->task->seq);
//     }
//     printf(" }\n");
//     pthread_mutex_unlock(&queue->lock);
// }

struct work_queue *queue_init(int capacity)
{
    capacity = capacity < 0 ? 0 : capacity;

    struct work_queue *queue;

    if ((queue = malloc(sizeof(struct work_queue))) == NULL)
        goto err_1;

    if ((queue->header = malloc(sizeof(struct work_queue_node))) == NULL ||
        (queue->tailer = malloc(sizeof(struct work_queue_node))) == NULL)
        goto err_2;

    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0)
        goto err_2;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
        goto err_3;
    if (pthread_mutex_init(&queue->lock, &attr) != 0)
        goto err_3;
    if (pthread_cond_init(&queue->empty, NULL) != 0)
        goto err_4;
    if (pthread_cond_init(&queue->full, NULL) != 0)
        goto err_5;

    queue->size = 0;
    queue->capacity = capacity;
    queue->header->prev = NULL;
    queue->tailer->next = NULL;
    queue->header->next = queue->tailer;
    queue->tailer->prev = queue->header;

    return queue;

err_5:
    pthread_cond_destroy(&queue->empty);
err_4:
    pthread_mutex_destroy(&queue->lock);
err_3:
    pthread_mutexattr_destroy(&attr);
err_2:
    if (queue->tailer)
        free(queue->tailer);
    if (queue->header)
        free(queue->header);
    free(queue);
err_1:
    return NULL;
}

int queue_size(struct work_queue *queue)
{
    if (queue == NULL)
        return -1;

    pthread_mutex_lock(&queue->lock);
    int size = queue->size;
    pthread_mutex_unlock(&queue->lock);
    return size;
}

int queue_capacity(struct work_queue *queue)
{
    if (queue == NULL)
        return -1;

    pthread_mutex_lock(&queue->lock);
    int capacity = queue->capacity;
    pthread_mutex_unlock(&queue->lock);
    return capacity;
}

int enqueue(struct work_queue *queue, struct task_t *task)
{
    if (queue == NULL || task == NULL)
        return -1;

    pthread_mutex_lock(&queue->lock);

    while (queue->size == queue->capacity)
        pthread_cond_wait(&queue->full, &queue->lock);

    queue->size++;
    struct work_queue_node *node;
    if ((node = malloc(sizeof(struct work_queue_node))) == NULL)
        goto err;

    node->task = task;
    node->prev = queue->tailer->prev;
    node->next = queue->tailer;
    queue->tailer->prev->next = node;
    queue->tailer->prev = node;

// #if _DEBUG_
//     queue_display(queue, "producer put %d in queue  \t", task->seq);
// #endif

    if (queue->size == 1)
        pthread_cond_broadcast(&queue->empty);

    pthread_mutex_unlock(&queue->lock);
    return 0;
err:
    pthread_mutex_unlock(&queue->lock);
    return -1;
}

int dequeue(struct work_queue *queue, struct task_t **task)
{
    if (queue == NULL || task == NULL)
        return -1;

    pthread_mutex_lock(&queue->lock);

    while (queue->size == 0)
        pthread_cond_wait(&queue->empty, &queue->lock);

    queue->size--;
    struct work_queue_node *node = queue->header->next;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    *task = node->task;
    free(node);

// #if _DEBUG_
//     queue_display(queue, "consumer get %d from queue\t", (*task)->seq);
// #endif

    if (queue->size == queue->capacity - 1)
        pthread_cond_broadcast(&queue->full);

    pthread_mutex_unlock(&queue->lock);
    return 0;
}

int prepare_service(unsigned short port)
{
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error!");
        return -1;
    }
    LOG("create listen socket succ [listenfd=%d]", listenfd);

    struct sockaddr_in svraddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("bind error");
        return -2;
    }
    LOG("listen socket bind succ");
    if (listen(listenfd, 10000) < 0)
    {
        ERRLOG("listen error");
        return -3;
    }
    LOG("listen succ, prepare_service finished!");
    return listenfd;
}

int TCP_connect(const char *ip, unsigned short port)
{
    struct sockaddr_in svraddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &svraddr.sin_addr) < 0)
    {
        ERRLOG("inet_pton error");
        return -1;
    }
    int svr_fd;
    if ((svr_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRLOG("socket error");
        return -2;
    }
    if (bind(svr_fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0)
    {
        ERRLOG("bind error");
        return -3;
    }
    struct sockaddr_in peeraddr;
    if (connect(svr_fd, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) < 0)
    {
        ERRLOG("connect to [%s:%d] error!", ip, port);
        return -4;
    }
    setnonblock(svr_fd);
    LOG("connect to original svr [%s:%d] succ!", trans_ip, trans_port);
    return svr_fd;
}

void *query_routine(void *arg)
{
    int listenfd, connfd;

    if ((listenfd = prepare_service(port)) < 0)
    {
        LOG("prepare_service error!");
        exit(1);
    }
    while (1)
    {
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
        {
            ERRLOG("accept error");
            exit(1);
        }
        char ipbuf[128];
        const char *peer_ip = inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf));
        unsigned short peer_port = ntohs(cliaddr.sin_port);
        LOG("new connection establish from [%s:%d]", peer_ip, peer_port);

        struct task_t *available;
        if (dequeue(idle_pool, &available) < 0)
        {
            LOG("fetch from idle_pool error!");
            exit(1);
        }
        LOG("fetch new session from idle_pool succ!");

        ssize_t nread;
        if ((nread = read(connfd, available->session.req_buf, MAX_BUFSZ)) < 0)
        {
            ERRLOG("read from client error!");
            exit(1);
        }
        available->session.seqno = get_value();
        available->session.proc_begtime = curtime_us();
        available->session.connfd = connfd;
        available->session.listenfd = listenfd;
        available->session.cli_ip = peer_ip;
        available->session.cli_port = peer_port;
        available->session.req_buf[nread] = 0;
        available->session.req_buf_len = nread;
        available->session.procedure_cmd = WEB_PROXY_SVR__PROCEDURE__FORWARD_TO_ORG;
        available->session.is_using = 1;

        if (enqueue(task_queue, available) < 0)
        {
            LOG("push to work_queue error!");
            exit(1);
        }
    }
}

void *search_routine(void *arg)
{
    while (1)
    {
        pthread_rwlock_rdlock(&global_fds_lock);
        for (int i = 0; i < 1024; i++)
        {
            if (check_readable_fds[i])
            {   
                int nread;
                if ((nread = read(i, check_readable_fds[i]->session.rsp_buf, MAX_BUFSZ)) < 0)
                {
                    LOG("fd %d no readable event occur! [seqno=%d]", 
                        i, check_readable_fds[i]->session.seqno);
                }
                else if (nread == 0)
                {
                    LOG("fd %d read 0 bytes?? [seqno=%d]", 
                        i, check_readable_fds[i]->session.seqno);
                }
                else
                {
                    check_readable_fds[i]->session.rsp_buf_len = nread;
                    check_readable_fds[i]->session.procedure_cmd = WEB_PROXY_SVR__PROCEDURE__SENDBACK_TO_CLIENT;
                    LOG("successfully read %d bytes from fd %d [%s:%d] [seqno=%d]", 
                        nread, i, trans_ip, trans_port, check_readable_fds[i]->session.seqno);
                    if (enqueue(task_queue, check_readable_fds[i]) < 0)
                    {
                        LOG("put session to work_queue fail!");
                        exit(1);
                    }
                    break;
                }
            }
        }
        pthread_rwlock_unlock(&global_fds_lock);
    }
}

void *work_process_routine(void *arg)
{
    while (1)
    {
        struct task_t *available;
        if (dequeue(task_queue, &available) < 0)
        {
            LOG("fetch session from idle_pool fail!");
            exit(1);
        }
        LOG("fetch session from idle_pool succ! [procedure=%d][seqno=%d]",
            available->session.procedure_cmd, available->session.seqno);

        if (available->session.is_using == 0)
        {
            LOG("fetch a unusing session, release it\n");
            continue;
        }
        LOG("session is using [procedure=%d][seqno=%d]", 
            available->session.procedure_cmd, available->session.seqno);

        switch (available->session.procedure_cmd)
        {
            case WEB_PROXY_SVR__PROCEDURE__FORWARD_TO_ORG :
            {
                if (available->session.svr_fd == 0)
                {
                    if ((available->session.svr_fd = TCP_connect(trans_ip, trans_port)) < 0)
                    {
                        LOG("TCP_connect to org_svr error");
                        exit(1);
                    }
                    LOG("first time establish connect to original svr [%s:%d] succ! [seqno=%d]", 
                        trans_ip, trans_port, available->session.seqno);
                }
                
                ssize_t nwrite;
                const char *msg = "hello world";
                if ((nwrite = write(available->session.svr_fd, msg, sizeof(msg))) < 0)
                {
                    ERRLOG("write error!");
                    exit(1);
                }
                LOG("forward request to original svr [%s:%d] succ! [seqno=%d]", 
                    trans_ip, trans_port, available->session.seqno);

                pthread_rwlock_wrlock(&global_fds_lock);
                check_readable_fds[available->session.svr_fd] = available;
                pthread_rwlock_unlock(&global_fds_lock);

            } break;
            case WEB_PROXY_SVR__PROCEDURE__SENDBACK_TO_CLIENT :
            {
                if (available->session.connfd == 0)
                {
                    LOG("fatal error! client connfd is zero [seqno=%d]", available->session.seqno);
                    exit(1);
                }
                ssize_t nwrite;
                const char *msg = "handle finished!";
                if ((nwrite = write(available->session.connfd, msg, sizeof(msg))) < 0)
                {
                    ERRLOG("write error!");
                    exit(1);
                }

                close(available->session.connfd);
                close(available->session.svr_fd);

                pthread_rwlock_wrlock(&global_fds_lock);
                check_readable_fds[available->session.svr_fd] = NULL;
                pthread_rwlock_unlock(&global_fds_lock);

                reset_task(available);

                if (enqueue(idle_pool, available) < 0)
                {
                    LOG("release session to idle_pool fail! [seqno=%d]", available->session.seqno);
                    exit(1);
                }
            } break;
            default :
            {
                LOG ("fatal error! unexpected procedure [seqno=%d]", available->session.seqno);
                exit(1);
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    port = 5000;

    trans_ip = "";
    trans_port = 6000;

    setbuf(stdout, NULL);

    /* 初始 session 池 */
    idle_pool = queue_init(100);

    /* 初始化工作队列 */
    task_queue = queue_init(10);

    /* 充满空闲 session 池 */
    for (int i = 0; i < 100; i++)
        enqueue(idle_pool, new_task());

    /* 启动上下游线程 */
    pthread_create(&query_net_io, NULL, query_routine, NULL);
    pthread_create(&search_net_io, NULL, search_routine, NULL);

    /* 启动工作线程 */
    for (int i = 0; i < 10; i++)
        pthread_create(work_process + i, NULL, work_process_routine, NULL);

    while (1)
        sleep(1);

    return 0;
}