#ifndef _DIJKSTRAX_H_
#define _DIJKSTRAX_H_

struct DijkstraSPX {
    double *disTo;
    struct edge_t *edgeTo;
    struct G *g;
};

struct DijkstraSPX *dijkstraInitWithEdgeWeightedGraph(struct G *g, int s);
void               dijkstraRelease(struct DijkstraSPX **ssp);
int                dijkstraHasPathTo(struct DijkstraSPX *sp, int v);
struct Stack *     dijkstraGetPathTo(struct DijkstraSPX *sp, int v);
double             dijkstraGetDistanceTo(struct DijkstraSPX *sp, int v);

#endif