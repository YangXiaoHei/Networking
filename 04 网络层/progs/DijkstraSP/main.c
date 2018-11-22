#include <stdio.h>
#include <stdlib.h>
#include "Dijkstra.h"
#include "BreadthFirstPath.h"

void test()
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
}

void test2()
{
    struct G *g = edgeWeightedGraphRandomInit(10, 20);
    printf("%s", edgeWeightedGraphToString(g));

    struct BreadthFirstPath *bfp = breadthFirstPathInit(g);
    int V = edgeWeightedGraphGetVertexCount(g);
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (breadthFirstPathHasPathBetween(bfp, i, j)) {
                struct Stack *s = breadthFirstPathGetPathBetween(bfp, i, j);
                printf("%d to %d : [hops=%d]", i, j, breadthFirstPathGetDistanceBetween(bfp, i, j));
                stackDisplay(s);
                stackRelease(&s);
            } else
                printf("no path from %d to %d\n", i, j);
        }
    }

    printf("the diameter of graph is %d\n", breadthFirstPathGetDiameter(bfp));
    struct Stack *s = breadthFirstPathGetPathOfDimeter(bfp);
    printf("diameter of graph :");
    stackDisplay(s);
    stackRelease(&s);

    breadthFirstPathRelease(&bfp);
    edgeWeightedGraphRelease(&g);
}

int main(int argc, char const *argv[])
{
    test2();
    return 0;
}
