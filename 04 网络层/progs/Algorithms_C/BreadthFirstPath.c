
#include "BreadthFirstPath.h"
#include <string.h>
#include <stdarg.h>
#include "log.h"
#include "Array.h"

static int checkVertexIsValid(struct BreadthFirstPath *bfp, int count, ...) 
{
    int V = edgeWeightedGraphGetVertexCount(bfp->g);
    va_list vl;
    va_start(vl, count);
    while (count--) {
        int v = va_arg(vl, int);
        if (v < 0 || v >= V) {
            va_end(vl);
            return 0;
        }
    }
    va_end(vl);
    return 1;
}

static int breadthFirstPathIsValid(struct BreadthFirstPath *bfp) 
{
    if (bfp == NULL || bfp->g == NULL || bfp->edgeTo == NULL || bfp->disTo == NULL) return 0;
    for (int i = 0; i < bfp->g->V; i++) 
        if (bfp->edgeTo[i] == NULL || bfp->disTo[i] == NULL || bfp->marked[i] == NULL)
            return 0;
    return 1;
}

static int breadthFirstPathGetDiameterInfo(struct BreadthFirstPath *bfp, int *src, int *dst) 
{
    int max = 0;
    int V = edgeWeightedGraphGetVertexCount(bfp->g);
    for (int v = 0; v < V; v++) {
        for (int w = 0; w < V; w++) {
            if (bfp->disTo[v][w] > max) {
                max = bfp->disTo[v][w];
                if (src) *src = v;
                if (dst) *dst = w;
            }
        }
    }
    return max;
}

static void bfs(struct BreadthFirstPath *bfp, int v) 
{
    struct G *g = bfp->g;
    struct edge_t *e = NULL;
    struct Array *queue = NULL;

    if ((queue = arrayInit()) == NULL) {
        LOG("bfs fail: arrayInit error!");
        return;
    }
    int s = v;
    bfp->marked[s][v] = 1;
    arrayAddElement(queue, v);
    while (!arrayIsEmpty(queue)) {
        v = arrayRemoveFirstElement(queue);
        EWG_FOREACH(g, e, v) {
            int w = weightedEdgeGetOther(e, v);
            if (!bfp->marked[s][w]) {
                bfp->marked[s][w] = 1;
                bfp->disTo[s][w] = bfp->disTo[s][v] + 1;
                memcpy(&bfp->edgeTo[s][w], e, sizeof(struct edge_t));
                arrayAddElement(queue, w);
            } 
        }
    }
    arrayRelease(&queue);
}

struct BreadthFirstPath *breadthFirstPathInit(struct G *g)
{
    struct BreadthFirstPath *bfp = NULL;
    int V = edgeWeightedGraphGetVertexCount(g);

    if ((bfp = malloc(sizeof(struct BreadthFirstPath))) == NULL)
        goto err;

    if ((bfp->edgeTo = malloc(sizeof(struct edge_t *) * V)) == NULL)
        goto err_1;

    for (int i = 0; i < V; i++) {
        if ((bfp->edgeTo[i] = malloc(sizeof(struct edge_t) * V)) == NULL)
            goto err_2;
        for (int j = 0; j < V; j++)
            weightedEdgeInvalidate(&bfp->edgeTo[i][j]);
    }

    if ((bfp->disTo = malloc(sizeof(int *) * V)) == NULL)
        goto err_3;

    for (int i = 0; i < V; i++) {
        if ((bfp->disTo[i] = malloc(sizeof(int) * V)) == NULL)
            goto err_4;
        bzero(bfp->disTo[i], sizeof(int) * V);
    }

    if ((bfp->marked = malloc(sizeof(char *) * V)) == NULL)
        goto err_5;

    for (int i = 0; i < V; i++) {
        if ((bfp->marked[i] = malloc(sizeof(char) * V)) == NULL)
            goto err_6;
        bzero(bfp->marked[i], sizeof(char) * V);
    }

    bfp->g = g;

    for (int v = 0; v < V; v++) 
        bfs(bfp, v);
    return bfp;

err_6:
    for (int i = 0; i < V; i++) 
        if (bfp->marked[i] != NULL) 
            free(bfp->marked[i]);
    free(bfp->marked);
err_5:
err_4:
    for (int i = 0; i < V; i++)
        if (bfp->disTo[i] != NULL)
            free(bfp->disTo[i]);
    free(bfp->disTo);
err_3:
err_2:
    for (int i = 0; i < V; i++)
        if (bfp->edgeTo[i] != NULL)
            free(bfp->edgeTo[i]);
    free(bfp->edgeTo);
err_1:
    free(bfp);
err:
    return NULL;
}

struct Stack *breadthFirstPathGetPathBetween(struct BreadthFirstPath *bfp, int v, int w)
{
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathGetPathBetween fail: bfp is invalid!");
        return NULL;
    }
    if (!checkVertexIsValid(bfp, 2, v, w)) {
        LOG("breadFirstPathGetPathBetween fail: v=%d or w=%d invalid!", v, w);
        return NULL;
    }

    struct Stack *s = NULL;
    if ((s = stackInit()) == NULL) {
        LOG("breadFirstPathGetPathBetween fail: stack init error!");
        return NULL;
    }
    struct edge_t *e = NULL;
    for (e = &bfp->edgeTo[v][w]; weightedEdgeIsValid(e) && weightedEdgeGetOther(e, w) != v; w = weightedEdgeGetOther(e, w), e = &bfp->edgeTo[v][w]) {
        stackPush(s, e);
    }
    if (weightedEdgeIsValid(e))
        stackPush(s, e);
    return s;
}
int breadthFirstPathHasPathBetween(struct BreadthFirstPath *bfp, int v, int w)
{
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathHasPathBetween fail: bfp is invalid!");
        return 0;
    }
    if (!checkVertexIsValid(bfp, 2, v, w)) {
        LOG("breadFirstPathGetPathBetween fail: v=%d or w=%d invalid!", v, w);
        return 0;
    }
    return !!bfp->marked[v][w];
}
int breadthFirstPathGetDistanceBetween(struct BreadthFirstPath *bfp, int v, int w)
{
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathGetDistanceBetween fail: bfp is invalid!");
        return -1;
    }
    if (!checkVertexIsValid(bfp, 2, v, w)) {
        LOG("breadFirstPathGetPathBetween fail: v=%d or w=%d invalid!", v, w);
        return -1;
    }
    return bfp->disTo[v][w];
}

struct Stack *breadthFirstPathGetPathOfDimeter(struct BreadthFirstPath *bfp)
{
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathGetPathOfDimeter fail: bfp is invalid!");
        return NULL;
    }
    int src = -1, dst = -1;
    breadthFirstPathGetDiameterInfo(bfp, &src, &dst);
    return breadthFirstPathGetPathBetween(bfp, src, dst);
}

int breadthFirstPathGetDiameter(struct BreadthFirstPath *bfp)
{
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathGetDiameter fail: bfp is invalid!");
        return -1;
    }
    return breadthFirstPathGetDiameterInfo(bfp, NULL, NULL);
}
void breadthFirstPathRelease(struct BreadthFirstPath **bbfp)
{
    struct BreadthFirstPath *bfp = *bbfp;
    if (!breadthFirstPathIsValid(bfp)) {
        LOG("breadFirstPathRelease fail: bfp is invalid!");
        return;
    }
    int V = edgeWeightedGraphGetVertexCount(bfp->g);
    for (int i = 0; i < V; i++) {
        free(bfp->disTo[i]);
        free(bfp->edgeTo[i]);
        free(bfp->marked[i]);
    }
    free(bfp->disTo);
    free(bfp->edgeTo);
    free(bfp->marked);
    *bbfp = NULL;
}