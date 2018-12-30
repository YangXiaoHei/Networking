#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_

#include "Graph.h"
#include "Stack.h"
#include "tool.h"
#include "log.h"
#include "Array.h"

struct DijkstraSP {
    double *disTo;
    struct edge_t *edgeTo;
    struct G *g;
};

struct DijkstraSP *dijkstraInitWithEdgeWeightedGraph(struct G *g, int s);
void               dijkstraRelease(struct DijkstraSP **ssp);
int                dijkstraHasPathTo(struct DijkstraSP *sp, int v);
struct Stack *     dijkstraGetPathTo(struct DijkstraSP *sp, int v);
double             dijkstraGetDistanceTo(struct DijkstraSP *sp, int v);

#endif 