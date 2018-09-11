#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>

#define _DEBUG_ 1

struct task_t {
    int seq;
    unsigned long begtime;
    unsigned long endtime;
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
static struct work_queue *task_queue;

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static int global_value = 1;

int get_value()
{
    pthread_mutex_lock(&global_lock);
    int val = global_value++;
    pthread_mutex_unlock(&global_lock);
    return val;
}

void queue_display(struct work_queue *queue, const char *format, ...)
{
    pthread_mutex_lock(&queue->lock);
    struct work_queue_node *cur;
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("%d {", queue->size);
    for (cur = queue->header->next; cur != queue->tailer; cur = cur->next)
    {
        if (cur->next == queue->tailer)
            printf(" %d", cur->task->seq);
        else
            printf(" %d,", cur->task->seq);
    }
    printf(" }\n");
    pthread_mutex_unlock(&queue->lock);
}

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

    task->seq = get_value();
    node->task = task;
    node->prev = queue->tailer->prev;
    node->next = queue->tailer;
    queue->tailer->prev->next = node;
    queue->tailer->prev = node;

#if _DEBUG_
    queue_display(queue, "producer put %d in queue  \t", task->seq);
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
    queue_display(queue, "consumer get %d from queue\t", (*task)->seq);
#endif

    if (queue->size == queue->capacity - 1)
        pthread_cond_broadcast(&queue->full);

    pthread_mutex_unlock(&queue->lock);
    return 0;
}

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
    t->seq = 0;
    t->begtime = curtime_us();
    t->endtime = 0;
    sleep(rand() % 3 + 1);
    return t;
}

void run_task(struct task_t *t)
{
    sleep(rand() % 3 + 1);
    free(t);
}

void *produce(void *arg)
{   
    int index = *(int *)arg;
    while (1)
    {
        enqueue(task_queue, new_task());
    }
}

void *consume(void *arg)
{
    int index = *(int *)arg;
    while (1)
    {
        struct task_t *t;
        dequeue(task_queue, &t);
        run_task(t);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        printf("usage : %s <#producer> <#consumer> <#queue_capacity>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    task_queue = queue_init(atoi(argv[3]));

    int n_pro = atoi(argv[1]);
    int n_con = atoi(argv[2]);

    pthread_t tid_consumer[n_con], tid_producer[n_pro];
    int cons[n_con], pros[n_pro];
    for (int i = 0; i < n_pro; i++)
    {
        pros[i] = i;
        pthread_create(tid_producer + i, NULL, produce, pros + i);
    }
    for (int i = 0; i < n_con; i++)
    {
        cons[i] = i;
        pthread_create(tid_consumer + i, NULL, consume, cons + i);
    }
    while (1);

    return 0;
}