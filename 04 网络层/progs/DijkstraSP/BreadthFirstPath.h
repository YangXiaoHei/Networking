#ifndef _BREADTH_FIRST_PATH_H_
#define _BREADTH_FIRST_PATH_H_

#include "Graph.h"
#include "Stack.h"

struct BreadthFirstPath {
    struct edge_t **edgeTo;
    int **disTo;
    unsigned char **marked;
    struct G *g;
};

struct BreadthFirstPath *breadthFirstPathInit(struct G *g);
struct Stack *           breadthFirstPathGetPathBetween(struct BreadthFirstPath *bfp, int v, int w);
int                      breadthFirstPathHasPathBetween(struct BreadthFirstPath *bfp, int v, int w);
int                      breadthFirstPathGetDiameter(struct BreadthFirstPath *bfp);
struct Stack *           breadthFirstPathGetPathOfDimeter(struct BreadthFirstPath *bfp);
int                      breadthFirstPathGetDistanceBetween(struct BreadthFirstPath *bfp, int v, int w);
void                     breadthFirstPathRelease(struct BreadthFirstPath **bfp);

#endif