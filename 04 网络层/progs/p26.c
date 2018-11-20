#include <stdio.h>
#include <stdlib.h>
#include "DijkstraSP/Dijkstra.h"

static unsigned char vertices[] = {
    'z', 
    'y', 
    'v', 
    't', 
    'x', 
    'w', 
    'u' 
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

    struct G *g = edgeWeightedGraphInit(7);
    edgeWeightedGraphAddEdge(g, indexOf('z'), indexOf('y'), 12);
    edgeWeightedGraphAddEdge(g, indexOf('z'), indexOf('x'), 8);
    edgeWeightedGraphAddEdge(g, indexOf('y'), indexOf('x'), 6);
    edgeWeightedGraphAddEdge(g, indexOf('y'), indexOf('t'), 7);
    edgeWeightedGraphAddEdge(g, indexOf('y'), indexOf('v'), 8);
    edgeWeightedGraphAddEdge(g, indexOf('v'), indexOf('t'), 4);
    edgeWeightedGraphAddEdge(g, indexOf('v'), indexOf('x'), 3);
    edgeWeightedGraphAddEdge(g, indexOf('v'), indexOf('u'), 3);
    edgeWeightedGraphAddEdge(g, indexOf('v'), indexOf('w'), 4);
    edgeWeightedGraphAddEdge(g, indexOf('x'), indexOf('w'), 6);
    edgeWeightedGraphAddEdge(g, indexOf('u'), indexOf('w'), 3);
    edgeWeightedGraphAddEdge(g, indexOf('u'), indexOf('t'), 2);
    printf("**************** print structure of graph ***************\n");
    printf("%s", edgeWeightedGraphToStringWithMapping(g, valueOf));
    printf("*********************************************************\n");

    int src = indexOf('x');
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
    edgeWeightedGraphRelease(&g);
    return 0;
}
