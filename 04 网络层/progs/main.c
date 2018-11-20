#include <stdio.h>
#include <stdlib.h>
#include "Dijkstra.h"

int main(int argc, char const *argv[])
{
    /*
     * V = 5
        E = 10
        0 : { 4 0 78 } { 0 3 62 } { 1 0 58 } { 0 0 69 } { 0 0 69 } { 0 1 21 } { 4 0 65 }
        1 : { 1 3 2 } { 1 2 31 } { 1 0 58 } { 0 1 21 }
        2 : { 4 2 81 } { 1 2 31 }
        3 : { 1 3 2 } { 0 3 62 }
        4 : { 4 4 57 } { 4 4 57 } { 4 2 81 } { 4 0 78 } { 4 0 65 }
     */ 
    struct G *g = edgeWeightedGraphInit(5);
    edgeWeightedGraphAddEdge(g, 4, 0, 65);
    edgeWeightedGraphAddEdge(g, 0, 1, 21);
    edgeWeightedGraphAddEdge(g, 0, 0, 69);
    edgeWeightedGraphAddEdge(g, 1, 0, 58);
    edgeWeightedGraphAddEdge(g, 0, 3, 62);
    edgeWeightedGraphAddEdge(g, 4, 0, 78); 
    edgeWeightedGraphAddEdge(g, 1, 2, 31);
    edgeWeightedGraphAddEdge(g, 1, 3,  2);  
    edgeWeightedGraphAddEdge(g, 4, 2, 81);  
    edgeWeightedGraphAddEdge(g, 4, 4, 57);
    printf("%s", edgeWeightedGraphToString(g));

    struct DijkstraSP *sp = dijkstraInitWithEdgeWeightedGraph(g, 0);
    for (int i = 0; i < edgeWeightedGraphGetVertexCount(g); i++) {
        printf("i = %d\n", i);
        if (dijkstraHasPathTo(sp, i)) {
            struct Stack *s = dijkstraGetPathTo(sp, i);
            printf("get path succ! i = %d\n", i);
            stackDisplay(s);
            stackRelease(&s);
        }
    }
    dijkstraRelease(&sp);
    edgeWeightedGraphRelease(&g);
    return 0;
}
