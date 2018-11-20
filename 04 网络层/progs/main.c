#include <stdio.h>
#include <stdlib.h>
#include "Dijkstra.h"

int main(int argc, char const *argv[])
{
    struct G *g = edgeWeightedGraphRandomInit(20, 50);
    printf("%s", edgeWeightedGraphToString(g));

    int src = 0;
    struct DijkstraSP *sp = dijkstraInitWithEdgeWeightedGraph(g, src);
    for (int i = 0; i < edgeWeightedGraphGetVertexCount(g); i++) {
        if (dijkstraHasPathTo(sp, i)) {
            struct Stack *s = dijkstraGetPathTo(sp, i);
            printf("%d to %d : [dis=%.2f]", src, i, dijkstraGetDistanceTo(sp, i));
            stackDisplay(s);
            stackRelease(&s);
        } else 
            printf("no path from %d to %d\n", src, i);
    }
    dijkstraRelease(&sp);
    edgeWeightedGraphRelease(&g);
    return 0;
}
