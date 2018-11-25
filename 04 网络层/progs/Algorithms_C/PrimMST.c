
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "PrimMST.h"

static void primMSTVisit(struct PrimMST *mst, int s)
{
    struct edge_t *e = NULL;
    mst->disTo[s] = 0.0;
    priorityQueueInsertElementAtIndex(mst->pq, s, mst->disTo[s]);
    while (!priorityQueueIsEmpty(mst->pq)) {
        int v = priorityQueueGetMinIndex(mst->pq);
        priorityQueueDequeue(mst->pq);

        mst->marked[v] = 1;
        EWG_FOREACH(mst->g, e, v) {
            int w = weightedEdgeGetOther(e, v);
            if (mst->marked[w]) continue;
            if (weightedEdgeGetWeight(e) < mst->disTo[w]) {
                mst->disTo[w] = weightedEdgeGetWeight(e);
                memcpy(&mst->edgeTo[w], e, sizeof(struct edge_t));
                if (priorityQueueContainsElementOfIndex(mst->pq, w))
                    priorityQueueDecreasePriorityOfIndex(mst->pq, w, mst->disTo[w]);
                else
                    priorityQueueInsertElementAtIndex(mst->pq, w, mst->disTo[w]);
            }
        }
    }
}

struct PrimMST *primMSTInit(struct G *g)
{
    struct PrimMST *mst = NULL;
    int V = edgeWeightedGraphGetVertexCount(g);
    if ((mst = malloc(sizeof(struct PrimMST))) == NULL)
        goto err;
    if ((mst->edgeTo = malloc(sizeof(struct edge_t) * V)) == NULL)
        goto err_1;
    if ((mst->disTo = malloc(sizeof(double) * V)) == NULL)
        goto err_2;
    if ((mst->marked = malloc(sizeof(char) * V)) == NULL)
        goto err_3;
    if ((mst->pq = priorityQueueInit(V)) == NULL)
        goto err_4;
    mst->totalWeight = 0.0;
    mst->g = g;
    for (int i = 0; i < V; i++) {
        weightedEdgeInvalidate(&mst->edgeTo[i]);
        mst->disTo[i] = DBL_MAX;
        mst->marked[i] = 0;
    }
    for (int i = 0; i < V; i++)
        if (!mst->marked[i])
            primMSTVisit(mst, i);

    return mst;

err_4:
    free(mst->marked);
err_3:
    free(mst->disTo);
err_2:
    free(mst->edgeTo);
err_1:
    free(mst);
err:
    return NULL;
}
void primMSTDisplayAllEdges(struct PrimMST *mst)
{
    printf("All edges in minimum spaning tree : [weight=%5.2f] [edgeCount=%d]\n", 
        primMSTGetWeight(mst), primMSTGetAllEdgesCount(mst));
    int V = edgeWeightedGraphGetVertexCount(mst->g);
    for (int i = 0; i < V; i++) {
        if (weightedEdgeIsValid(&mst->edgeTo[i])) {
            int v = weightedEdgeGetEither(&mst->edgeTo[i]);
            int w = weightedEdgeGetOther(&mst->edgeTo[i], v);
            double weight = weightedEdgeGetWeight(&mst->edgeTo[i]);
            printf("{ %d %d %5.2f}\n", v, w, weight);
        }
    }
}

void primMSTDisplayAllEdgesByMapping(struct PrimMST *mst, unsigned char(*mapper)(int))
{
    printf("All edges in minimum spaning tree : [weight=%5.2f] [edgeCount=%d]\n", 
        primMSTGetWeight(mst), primMSTGetAllEdgesCount(mst));
    int V = edgeWeightedGraphGetVertexCount(mst->g);
    for (int i = 0; i < V; i++) {
        if (weightedEdgeIsValid(&mst->edgeTo[i])) {
            int v = weightedEdgeGetEither(&mst->edgeTo[i]);
            int w = weightedEdgeGetOther(&mst->edgeTo[i], v);
            double weight = weightedEdgeGetWeight(&mst->edgeTo[i]);
            printf("{ %c %c %5.2f}\n", mapper(v), mapper(w), weight);
        }
    }
}
int  primMSTGetAllEdgesCount(struct PrimMST *mst)
{
    int V = edgeWeightedGraphGetVertexCount(mst->g);
    int n = 0;
    for (int i = 0; i < V; i++) 
        if (weightedEdgeIsValid(&mst->edgeTo[i]))
            n++;
    return n;
}
double primMSTGetWeight(struct PrimMST *mst)
{
    int V = edgeWeightedGraphGetVertexCount(mst->g);
    int weight = 0;
    for (int i = 0; i < V; i++) 
        if (weightedEdgeIsValid(&mst->edgeTo[i]))
            weight += weightedEdgeGetWeight(&mst->edgeTo[i]);
    return weight;
}

void primMSTRelease(struct PrimMST **mmst)
{
    struct PrimMST *mst = *mmst;
    free(mst->edgeTo);
    free(mst->disTo);
    free(mst->marked);
    priorityQueueRelease(&mst->pq);
    free(mst);
    *mmst = NULL;
}