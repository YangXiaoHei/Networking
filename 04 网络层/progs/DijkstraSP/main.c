#include <stdio.h>
#include <stdlib.h>
#include "Dijkstra.h"
#include "PriorityQueue.h"
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

void test3() 
{
    struct PriorityQueue *pq = priorityQueueInit(9);

#define INSERT_ALL  \
    do {            \
        priorityQueueInsertElementAtIndex(pq, 3, 3.4);  \
        priorityQueueInsertElementAtIndex(pq, 1, 7.5);  \
        priorityQueueInsertElementAtIndex(pq, 0, 2.1);  \
        priorityQueueInsertElementAtIndex(pq, 5, 9.0);  \
        priorityQueueInsertElementAtIndex(pq, 4, 3.4);  \
        priorityQueueInsertElementAtIndex(pq, 7, 2.7);  \
        priorityQueueInsertElementAtIndex(pq, 8, 1.1);  \
        priorityQueueInsertElementAtIndex(pq, 6, 5.2);  \
        priorityQueueInsertElementAtIndex(pq, 2, 8.6);  \
    } while(0);

#define POP_ALL     \
    do {    \
        while (!priorityQueueIsEmpty(pq)) { \
            printf("最小值 %d : %5.1f\n",priorityQueueGetMinIndex(pq),     \
                                        priorityQueueGetMinKey(pq));    \
            priorityQueueDequeue(pq);   \
        }   \
        printf("---------------\n"); \
    } while (0);


    INSERT_ALL
    priorityQueueDisplay(pq);
    POP_ALL

    INSERT_ALL
    priorityQueueRemoveElementOfIndex(pq, 3);
    priorityQueueRemoveElementOfIndex(pq, 4);
    priorityQueueRemoveElementOfIndex(pq, 6);
    priorityQueueRemoveElementOfIndex(pq, 7);
    priorityQueueRemoveElementOfIndex(pq, 0);
    POP_ALL

    INSERT_ALL
    priorityQueueChangePriorityOfIndex(pq, 8, 6.6);
    priorityQueueChangePriorityOfIndex(pq, 0, 3.9);
    priorityQueueChangePriorityOfIndex(pq, 2, 0.5);
    priorityQueueChangePriorityOfIndex(pq, 6, 1.7);
    POP_ALL

    INSERT_ALL
    priorityQueueDecreasePriorityOfIndex(pq, 5, 5.5);
    priorityQueueDecreasePriorityOfIndex(pq, 2, 3.2);
    priorityQueueDecreasePriorityOfIndex(pq, 7, 0.1);
    priorityQueueDecreasePriorityOfIndex(pq, 1, 2.9);
    priorityQueueDecreasePriorityOfIndex(pq, 8, 0.5);
    priorityQueueDecreasePriorityOfIndex(pq, 6, 4.4);
    POP_ALL

    priorityQueueRelease(&pq);
}

int main(int argc, char const *argv[])
{
    test3();
    return 0;
}
