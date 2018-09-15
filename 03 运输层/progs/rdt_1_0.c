/*
 * 
 * 1, 最简单情况，底层信道是完全可靠的
 * 
 * 在底层信道是完全可靠的假设下，发送端仅有一个状态，发送。接收端仅有一个状态，接收
 */
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

#define _DEBUG_ 0
#define MAX_BUFSZ (1 << 10)

struct packet_t {

    /*
     * 模拟头部信息
     */
    short header_xxx;
    short header_yyy;
    short header_zzz;

    /*
     * 数据域
     */
    char data[MAX_BUFSZ];  // 一定要以 '\0' 结尾
    int  data_len;
};

struct task_t {
    struct packet_t packet;
    int (*display_value_for_queue_iterate)(struct task_t *);
};
struct work_queue_node {
    struct task_t *task;
    struct work_queue_node *next;
    struct work_queue_node *prev;
};
struct work_queue {
    const char *name;
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

/* 发送端 */
static pthread_t sender;

/* 接收端 */
static pthread_t receiver;

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static int global_value = 1;
int get_value()
{
    pthread_mutex_lock(&global_lock);
    int val = global_value++;
    pthread_mutex_unlock(&global_lock);
    return val;
}

unsigned long curtime_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

char * display(struct task_t *task)
{
    return task->packet.data;
}

struct task_t* new_task()
{
    struct task_t* t;
    if ((t = malloc(sizeof(struct task_t))) == NULL)
        return NULL;
    t->display_value_for_queue_iterate = display;
    return t;
}

void reset_task(struct task_t *task)
{
    bzero(task, sizeof(struct task_t));
}

void queue_display(struct work_queue *queue, const char *format, ...)
{
    pthread_mutex_lock(&queue->lock);
    struct work_queue_node *cur;
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("%s %d {", queue->name, queue->size);
    for (cur = queue->header->next; cur != queue->tailer; cur = cur->next)
    {
        if (cur->next == queue->tailer)
            printf(" %s", cur->task->display_value_for_queue_iterate(cur->task));
        else
            printf(" %s,", cur->task->display_value_for_queue_iterate(cur->task));
    }
    printf(" }\n");
    pthread_mutex_unlock(&queue->lock);
}

struct work_queue *queue_init(const char *name, int capacity)
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

    queue->name = name;
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

#if _DEBUG_
    queue_display(queue, "producer put %d in queue  \t", task->display_value_for_queue_iterate(task));
#endif

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

#if _DEBUG_
    queue_display(queue, "consumer get %d from queue\t", (*task)->display_value_for_queue_iterate(*task));
#endif

    if (queue->size == queue->capacity - 1)
        pthread_cond_broadcast(&queue->full);

    pthread_mutex_unlock(&queue->lock);
    return 0;
}

int rdt_send(char *data);
struct packet_t make_pkt(char *data);
int udt_send(struct packet_t *packet);
int rdt_rcv(struct packet_t *packet);
int extract(const struct packet_t *packet, char *data);
int deliver_data(char *data);

/*
 * 发送端：可靠发
 */
int rdt_send(char *data)
{
    udt_send(make_pkt(data));
}

/*
 * 发送端：拼包
 */
struct packet_t make_pkt(char *data)
{
    struct packet_t 
}

/*
 * 发送端：不可靠发送
 */
int udt_send(struct packet_t *packet)
{

}

/*
 * 接收端：可靠的收
 */
int rdt_rcv(struct packet_t *packet)
{

}

/*
 * 提取包内数据
 */
int extract(const struct packet_t *packet, char *data)
{

}

/*
 * 接收端：交付数据
 */
int deliver_data(char *data)
{

}

int main(int argc, char const *argv[])
{
    
    return 0;
}