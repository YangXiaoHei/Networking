#include <stdio.h>
#include <stdlib.h>
#include "Algorithms/Dijkstra.h"
#include "Algorithms/BreadthFirstPath.h"

static unsigned char vertices[] = {
    [0] = 'A', 
    [1] = 'B', 
    [2] = 'C', 
    [3] = 'D', 
    [4] = 'E', 
    [5] = 'F', 
    [6] = 'G' 
};

#define GET_VERTEX_COUNT (sizeof(vertices) / sizeof(vertices[0]))

int indexOf(unsigned char c) 
{
    for (int i = 0; i < GET_VERTEX_COUNT; i++)
        if (vertices[i] == c)
            return i;
    LOG("what a fuck?!");
    exit(1);
}
unsigned char valueOf(int i) 
{
    if (i < 0 || i >= GET_VERTEX_COUNT) {
        LOG("what a fuck?1");
        exit(1);
    }
    return vertices[i];
}

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);

    int src = indexOf('C');

    struct G *g = edgeWeightedGraphInitWithMapping(GET_VERTEX_COUNT, indexOf);
    edgeWeightedGraphAddEdgeByMapping(g, 'A', 'B', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'A', 'C', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'B', 'C', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'C', 'F', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'C', 'E', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'F', 'E', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'B', 'D', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'E', 'D', 1);
    edgeWeightedGraphAddEdgeByMapping(g, 'D', 'G', 1);
    printf("**************** print structure of graph ***************\n");
    printf("%s", edgeWeightedGraphToStringWithMapping(g, valueOf));
    printf("*********************************************************\n");
    
    printf("-------------------- Dijkstra -------------------\n");
    struct DijkstraSP *sp = dijkstraInitWithEdgeWeightedGraph(g, src);
    for (int v = 0; v < edgeWeightedGraphGetVertexCount(g); v++) {
        if (dijkstraHasPathTo(sp, v)) {
            struct Stack *s = dijkstraGetPathTo(sp, v);
            printf("shortest path from %c to %c : [distance = %.0f]", valueOf(src), valueOf(v), dijkstraGetDistanceTo(sp, v));
            stackDisplayWithMapping(s, valueOf);
            stackRelease(&s);
        } else 
            printf("no path from %c to %c\n", valueOf(src), valueOf(v));
    }
    dijkstraRelease(&sp);
    printf("---------------------- BFS ---------------------\n");
    struct BreadthFirstPath *bfp = breadthFirstPathInit(g);
    for (int i = 0; i < GET_VERTEX_COUNT; i++) {
        if (breadthFirstPathHasPathBetween(bfp, src, i)) {
            struct Stack *s = breadthFirstPathGetPathBetween(bfp, src, i);
            printf("shortest path from %c to %c : ", valueOf(src), valueOf(i));
            stackDisplayWithMapping(s, valueOf);
            stackRelease(&s);
        } else
            printf("no path from %c to %c\n", valueOf(src), valueOf(i));
    }
    breadthFirstPathRelease(&bfp);
    edgeWeightedGraphRelease(&g); 
}
