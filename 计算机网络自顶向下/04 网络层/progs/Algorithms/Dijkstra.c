
#include "Dijkstra.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

static inline int dijkstraIsValid(struct DijkstraSP *sp) 
{
    return sp != NULL && sp->disTo != NULL && sp->edgeTo != NULL && sp->g != NULL;
}

static void dijkstraDisplayDisTo(struct DijkstraSP *sp)
{
    for (int i = 0; i < sp->g->V; i++) {
        double disTo = sp->disTo[i];
        if (disTo == DBL_MAX) 
            printf("%-10s", "infinite");
        else 
            printf("%-10.2f", disTo);
    }
    printf("\n");
}

static void dijkstraDisplayEdgeTo(struct DijkstraSP *sp)
{
    for (int i = 0; i < sp->g->V; i++) {
        struct edge_t *e = &sp->edgeTo[i];
        if (weightedEdgeIsValid(e)) 
            printf(" { %d %d %.2f } ", e->v, e->w, e->weight);
         else 
            printf("%-10s", "null");
    }
    printf("\n");
}

static void dijkstraRelaxVertex(struct DijkstraSP *sp, int v) 
{
    struct G *g = sp->g;
    struct edge_t *e = NULL;
    EWG_FOREACH(g, e, v) {
        int w = weightedEdgeGetOther(e, v);
        if (sp->disTo[w] > sp->disTo[v] + e->weight) {
            sp->disTo[w] = sp->disTo[v] + e->weight;
            memcpy(&sp->edgeTo[w], e, sizeof(struct edge_t));
        }
    }
}

static int dijkstraFindNextVertexNeedToRelax(struct DijkstraSP *sp, struct Array *arr) 
{
    int minv = -1;
    double min = DBL_MAX;
    int V = edgeWeightedGraphGetVertexCount(sp->g);
    for (int i = 0; i < V; i++) {

        if (arrayContainsElement(arr, i))
            continue;

        if (sp->disTo[i] < min) {
            min = sp->disTo[i];
            minv = i;
        }
    }
    return minv;
}

struct DijkstraSP *dijkstraInitWithEdgeWeightedGraph(struct G *g, int s)
{
    struct DijkstraSP *sp = NULL;
    int *indexes = NULL;
    int V = 0;
    struct edge_t *e = NULL;
    struct Array *arr = NULL;
    int minv = -1;

    if (g == NULL || s < 0 || s > g->V) {
        LOG("dijkstraInitWithEdgeWeightedGraph fail! invalid argument g == NULL or s = %d\n", s);
        return NULL;
    }

    V = edgeWeightedGraphGetVertexCount(g);
    if ((sp = malloc(sizeof(struct DijkstraSP))) == NULL)
        goto err;
    if ((sp->edgeTo = malloc(sizeof(struct edge_t) * V)) == NULL)
        goto err_1;
    if ((sp->disTo = malloc(sizeof(double) * V)) == NULL)
        goto err_2;
    bzero(sp->disTo, sizeof(double) * V);

    sp->g = g;
    
    /* 将所有边初始化为无效状态 */    
    for (int i = 0; i < V; i++) 
        weightedEdgeInvalidate(&sp->edgeTo[i]);

    /* 将起点到所有顶点的距离初始化为无穷大 */
    for (int i = 0; i < V; i++) 
        sp->disTo[i] = DBL_MAX;

    /* dijkstra 起点距离为 0 */
    sp->disTo[s] = 0.0;
    
    /* 遍历起点的所有邻接点 */
    EWG_FOREACH(g, e, s) {

        /* 获取邻接点 */
        int w = weightedEdgeGetOther(e, s);

        /* 更新起点到所有邻接点的距离 */
        if (sp->disTo[w] > e->weight) {
            sp->disTo[w] = e->weight;
            memcpy(&sp->edgeTo[w], e, sizeof(struct edge_t));
        }
    }

    arr = arrayInit();
    while (arrayGetSize(arr) < V) {

        if ((minv = dijkstraFindNextVertexNeedToRelax(sp, arr)) < 0)
            break;

        arrayAddElement(arr, minv);
        dijkstraRelaxVertex(sp, minv);
    }
    arrayRelease(&arr);
    return sp;

err_2:
    free(sp->edgeTo);
err_1:
    free(sp);
err:
    return NULL;
}

int dijkstraHasPathTo(struct DijkstraSP *sp, int v)
{
    if (!dijkstraIsValid(sp)) {
       LOG("dijkstraGetPathTo fail!: dijkstraSP is invalid");
        return 0; 
    }
    return sp->disTo[v] < DBL_MAX;
}

void dijkstraRelease(struct DijkstraSP **ssp) 
{
    if (ssp == NULL) {
        LOG("dijkstraRelease fail!: ssp == NULL");
        return;
    }
    struct DijkstraSP *sp = *ssp;
    if (!dijkstraIsValid(sp)) {
        LOG("dijkstraRelease fail!: dijkstraSP is invalid");
        return; 
    }
    free(sp->disTo);
    free(sp->edgeTo);
    free(sp);
    *ssp = NULL;
}

struct Stack *dijkstraGetPathTo(struct DijkstraSP *sp, int v)
{
    if (!dijkstraIsValid(sp)) {
       LOG("dijkstraGetPathTo fail!: dijkstraSP is invalid");
        return NULL; 
    }

    if (!dijkstraHasPathTo(sp, v)) {
        LOG("dijkstraGetPathTo fail!: no path to %d", v);
        return NULL;
    }
    struct Stack *s = stackInit();
    for (struct edge_t *e = &sp->edgeTo[v]; weightedEdgeIsValid(e); v = weightedEdgeGetOther(e, v), e = &sp->edgeTo[v])
        stackPush(s, e);
    return s;
}

double dijkstraGetDistanceTo(struct DijkstraSP *sp, int v)
{
    if (!dijkstraIsValid(sp)) {
        LOG("invalid argument : sp is not invalid!");
        return DBL_MAX;
    }
    return sp->disTo[v];
}