#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "Stack.h"

struct Stack *stackInit()
{
    struct Stack *s = NULL;
    if ((s = malloc(sizeof(struct Stack))) == NULL) {
        LOG("stackInit fail: memory malloc error!");
        return NULL;
    }
    s->size = 0;
    s->first = NULL;
    return s;
}

void stackDisplay(struct Stack *s)
{
    for (struct stack_node_t *n = s->first; n != NULL; n = n->next) {
        printf(" { %d %d %.2f } ", n->edge.v, n->edge.w, n->edge.weight);
    }
    printf("\n");
}

int stackGetSize(struct Stack *s)
{
    if (s == NULL) {
        LOG("stackGetSize fail!: s == NULL!");
        return 0;
    }
    return s->size;
}
void stackPush(struct Stack *s, struct edge_t *e)
{
    if (s == NULL || e == NULL) {
        LOG("stackPush error!: invalid argument s or e equal to null!");
        return;
    }

    struct stack_node_t *node = NULL;
    if ((node = malloc(sizeof(struct stack_node_t))) == NULL) {
        LOG("newnode create fail!: memory malloc error!");
        return;
    }

    memcpy(&node->edge, e, sizeof(struct edge_t));
    node->next = s->first;
    s->first = node;
    s->size++;
}
void stackPop(struct Stack *s)
{
    if (stackIsEmpty(s)) {
        LOG("stackPop fail!: stack is empty!");
        return;
    }
    struct stack_node_t *node = s->first;
    s->first = s->first->next;
    s->size--;
    free(node);
}
struct edge_t *stackGetTop(struct Stack *s)
{
    if (stackIsEmpty(s)) {
        LOG("stackTop fail!: stack is empty!");
        return NULL;
    }
    return &s->first->edge;
}
int stackIsEmpty(struct Stack *s)
{
    if (s == NULL) {
        LOG("stackIsEmpty fail!: s == NULL!");
        return 1;
    }
    return s->size == 0;
}
void stackRelease(struct Stack **ss)
{
    if (ss == NULL || *ss == NULL) {
        LOG("stackRelease fail!: s or *s == NULL!");
        return;
    }
    struct Stack *s = *ss;
    while (!stackIsEmpty(s)) 
        stackPop(s);
    free(s);
    *ss = NULL;
}