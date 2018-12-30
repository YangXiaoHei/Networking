#ifndef _PRIORITYQUEUE_H_
#define _PRIORITYQUEUE_H_

typedef double PriorityQueueKeyType;

struct PriorityQueue {
    int *pq;
    int *qp;
    PriorityQueueKeyType *keys;
    int size;
    int capacity;
};

struct PriorityQueue *priorityQueueInit(int capacity);
void priorityQueueRelease(struct PriorityQueue **pq);
void priorityQueueDisplay(struct PriorityQueue *pq);
int priorityQueueIsFull(struct PriorityQueue *pq);
int priorityQueueIsEmpty(struct PriorityQueue *pq);
int priorityQueueGetSize(struct PriorityQueue *pq);
void priorityQueueDequeue(struct PriorityQueue *pq);
PriorityQueueKeyType priorityQueueGetMinKey(struct PriorityQueue *pq);
int priorityQueueGetMinIndex(struct PriorityQueue *pq);
void priorityQueueInsertElementAtIndex(struct PriorityQueue  *pq, int i, PriorityQueueKeyType e);
PriorityQueueKeyType priorityQueueGetElementOfIndex(struct PriorityQueue *pq, int i);
int priorityQueueContainsElementOfIndex(struct PriorityQueue *pq, int i);
void priorityQueueRemoveElementOfIndex(struct PriorityQueue *pq, int i);
void priorityQueueChangePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e);
void priorityQueueDecreasePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e);
void priorityQueueIncreasePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e);

#endif