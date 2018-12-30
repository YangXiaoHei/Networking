
#include "PriorityQueue.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PQ (pq->pq)
#define QP (pq->qp)
#define SIZE (pq->size)
#define KEYS (pq->keys)
#define CAP (pq->capacity)

static void priorityQueueSwim(struct PriorityQueue *pq, int k)
{
    int toSwimIndex = PQ[k];
    while (k > 1 && KEYS[toSwimIndex] < KEYS[PQ[k >> 1]]) {
        PQ[k] = PQ[k >> 1];
        QP[PQ[k]] = k;
        k >>= 1;
    }
    PQ[k] = toSwimIndex;
    QP[PQ[k]] = k;
}

static void priorityQueueSink(struct PriorityQueue *pq, int k)
{
    int toSinkIndex = PQ[k];
    while ((k << 1) <= SIZE) {
        int j = k << 1;
        if (KEYS[PQ[j]] > KEYS[PQ[j + 1]]) j++;
        if (KEYS[toSinkIndex] <= KEYS[PQ[j]]) break;
        PQ[k] = PQ[j];
        QP[PQ[k]] = k;
        k = j;
    }
    PQ[k] = toSinkIndex;
    QP[PQ[k]] = k;
}

struct PriorityQueue *priorityQueueInit(int capacity)
{
    struct PriorityQueue *pq = NULL;
    if ((pq = malloc(sizeof(struct PriorityQueue))) == NULL)
        goto err;
    if ((KEYS = malloc(sizeof(double) * capacity)) == NULL)
        goto err_1;
    if ((PQ = malloc(sizeof(int) * (capacity + 1))) == NULL)
        goto err_2;
    if ((QP = malloc(sizeof(int) * (capacity + 1))) == NULL)
        goto err_3;
    SIZE = 0;
    CAP = capacity;
    memset(KEYS, 0, sizeof(PriorityQueueKeyType) * capacity);
    memset(PQ, -1, sizeof(int) * (capacity + 1));
    memset(QP, -1, sizeof(int) * (capacity + 1));
    return pq;

err_3:
    free(PQ);
err_2:
    free(KEYS);
err_1:
    free(pq);
err:
    return NULL;
}

void priorityQueueRelease(struct PriorityQueue **ppq)
{
    struct PriorityQueue *pq = *ppq;
    free(PQ);
    free(QP);
    free(KEYS);
    free(pq);
    *ppq = NULL;
}

void priorityQueueDisplay(struct PriorityQueue *pq)
{
    printf("%-7s", " ");
    for (int i = 0; i <= CAP; i++) {
        printf("%-7d", i);
    }
    printf("\n");
    printf("%-7s", "keys");
    for (int i = 0; i < CAP; i++) {
        printf("%-7.1f", KEYS[i]);
    }
    printf("\n");
    printf("%-7s", "pq");
    for (int i = 0; i <= CAP; i++) {
        printf("%-7d", PQ[i]);
    }
    printf("\n");
    printf("%-7s", "qp");
    for (int i = 0; i <= CAP; i++) {
        printf("%-7d", QP[i]);
    }
    printf("\n");
}

int priorityQueueIsFull(struct PriorityQueue *pq)
{
    return SIZE == CAP;
}
int priorityQueueIsEmpty(struct PriorityQueue *pq)
{
    return SIZE == 0;
}

int priorityQueueGetSize(struct PriorityQueue *pq)
{
    return SIZE;
}

PriorityQueueKeyType priorityQueueDequeueElement(struct PriorityQueue *pq)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueDequeueElement fail: pq is empty!");
        return -1;
    }
    int min = PQ[1];
    PriorityQueueKeyType minKey = KEYS[min];
    PQ[1] = PQ[SIZE--];
    QP[PQ[1]] = 1;
    priorityQueueSink(pq, 1);
    KEYS[min] = 0;
    PQ[SIZE + 1] = -1;
    QP[min] = -1;

    return minKey;
}

PriorityQueueKeyType priorityQueueGetMinKey(struct PriorityQueue *pq)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueGetMinKey fail : pq is empty!");
        return 0;
    }
    return KEYS[PQ[1]];
}

int priorityQueueGetMinIndex(struct PriorityQueue *pq)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueGetElementOfIndex fail : pq is empty!");
        return 0;
    }
    return PQ[1];
}

void priorityQueueDequeue(struct PriorityQueue *pq)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueDequeueIndex fail: pq is empty!");
        return;
    }
    int min = PQ[1];
    PQ[1] = PQ[SIZE--];
    QP[PQ[1]] = 1;
    priorityQueueSink(pq, 1);
    KEYS[min] = 0;
    PQ[SIZE + 1] = -1;
    QP[min] = -1;
}

void priorityQueueInsertElementAtIndex(struct PriorityQueue  *pq, int i, PriorityQueueKeyType e)
{
    if (priorityQueueIsFull(pq)) {
        LOG("priorityQueueInsertElementAtIndex fail : pq is full!");
        return;
    }
    if (priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueInsertElementAtIndex fail : already exist!");
        return;
    }
    KEYS[i] = e;
    ++SIZE;
    PQ[SIZE] = i;
    QP[i] = SIZE;
    priorityQueueSwim(pq, SIZE); 
}

PriorityQueueKeyType priorityQueueGetElementOfIndex(struct PriorityQueue *pq, int i)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueGetElementOfIndex fail : pq is empty!");
        return 0;
    }
    if (!priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueGetElementOfIndex fail : %d not exist in pq!", i);
        return 0;
    }
    return KEYS[i];
}

int priorityQueueContainsElementOfIndex(struct PriorityQueue *pq, int i)
{
    return !priorityQueueIsEmpty(pq) && QP[i] != -1;
}

void priorityQueueRemoveElementOfIndex(struct PriorityQueue *pq, int i)
{
    if (priorityQueueIsEmpty(pq)) {
        LOG("priorityQueueRemoveElementOfIndex fail : pq is empty!");
        return;
    }
    if (!priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueRemoveElementOfIndex fail : %d not exist in pq!", i);
        return;
    }
    int indexOfKeyIndex = QP[i];
    PQ[indexOfKeyIndex] = PQ[SIZE--];
    QP[PQ[indexOfKeyIndex]] = indexOfKeyIndex;
    priorityQueueSwim(pq, indexOfKeyIndex);
    priorityQueueSink(pq, indexOfKeyIndex);
    KEYS[i] = 0.0;
    PQ[SIZE + 1] = -1;
    QP[i] = -1; 
}

void priorityQueueChangePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e)
{
    if (!priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueChangePriorityOfIndex fail : %d not exist in pq!", i);
        return;
    }
    KEYS[i] = e;
    priorityQueueSwim(pq, QP[i]);
    priorityQueueSink(pq, QP[i]);
}

void priorityQueueDecreasePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e)
{
    if (!priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueDecreasePriorityOfIndex fail : %d not exist in pq!", i);
        return;
    }
    KEYS[i] = e;
    priorityQueueSwim(pq, QP[i]);
}

void priorityQueueIncreasePriorityOfIndex(struct PriorityQueue *pq, int i, PriorityQueueKeyType e)
{
    if (!priorityQueueContainsElementOfIndex(pq, i)) {
        LOG("priorityQueueIncreasePriorityOfIndex fail : %d not exist in pq!", i);
        return;
    }
    KEYS[i] = e;
    priorityQueueSink(pq, QP[i]);
}