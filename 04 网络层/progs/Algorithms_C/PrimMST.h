#ifndef _PRIMMST_H_
#define _PRIMMST_H_

#include "Graph.h"
#include "PriorityQueue.h"

struct PrimMST {
    struct edge_t *edgeTo;
    double *disTo;
    char *marked;
    double totalWeight;
    struct PriorityQueue *pq;
    struct G *g;
};

struct PrimMST *primMSTInit(struct G *g);
double primMSTGetWeight(struct PrimMST *mst);
void primMSTDisplayAllEdges(struct PrimMST *mst);
int  primMSTGetAllEdgesCount(struct PrimMST *mst);
void primMSTRelease(struct PrimMST **mst);

#endif