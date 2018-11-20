#ifndef _STACK_H_
#define _STACK_H_

#include "Graph.h"

struct stack_node_t {
    struct stack_node_t *next;
    struct edge_t edge;
};

struct Stack {
    int size;
    struct stack_node_t *first;
};

struct Stack *  stackInit();
void            stackRelease(struct Stack **s);
int             stackGetSize(struct Stack *s);
struct edge_t * stackGetTop(struct Stack *s);
void            stackPush(struct Stack *s, struct edge_t *e);
void            stackPop(struct Stack *s);
int             stackIsEmpty(struct Stack *s);
void            stackDisplay(struct Stack *s);

#endif