#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_

#include "Graph.h"
#include "Stack.h"

struct DijkstraSP {
    double *disTo;
    struct edge_t *edgeTo;
    struct G *g;
};

struct *DijkstraInit(struct G *g);
int hasPathTo(struct DijkstraSP *sp, int v);
struct Stack *pathTo(struct DijkstraSP *sp, int v);
double disTo(struct DijkstraSP *sp, int v);


#endif 