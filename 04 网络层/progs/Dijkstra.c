
#include "Dijkstra.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

/*
 * struct DijkstraSP {
    double *disTo;
    struct edge_t *edgeTo;
    struct G *g;
};

struct edge_t {
    struct edge_t *next;
    double weight;
    int v, w;
};

struct *DijkstraInit(struct G *g);
int hasPathTo(struct DijkstraSP *sp, int v);
struct Stack *pathTo(struct DijkstraSP *sp, int v);
double disTo(struct DijkstraSP *sp, int v);
 */

static inline int isDijkstraValid(struct DijkstraSP *sp) 
{
    return sp->disTo != NULL && sp->edgeTo != NULL && sp->g != NULL;
}

static void relax(struct DijkstraSP *sp, int v)
{
    struct G *g = sp->g;
    for (struct edge_t *e = g->adjs[v].first; e != NULL; e = e->next) {
        int w = other(e, v);
        if (sp->disTo[w] > sp->disTo[v] + e->weight) {
            sp->disTo[w] = sp->disTo[v] + e->weight;
            sp->edgeTo[w].next = NULL;
            sp->edgeTo[w].weight = e->weight;
            sp->edgeTo[w].v = e->v;
            sp->edgeTo[w].w = e->w;
        }
    }
}

struct *DijkstraInit(struct G *g, int s)
{
    struct DijkstraSP *sp = NULL;
    if ((sp = malloc(sizeof(struct DijkstraSP))) == NULL)
        goto err;
    if ((sp->edgeTo = malloc(sizeof(struct edge_t) * g->V)) == NULL)
        goto err_1;
    if ((sp->disTo = malloc(sizeof(double) * g->V)) == NULL)
        goto err_2;
    sp->g = g;

    int *indexes = NULL;
    if ((indexes = malloc(sizeof(int) * g->V)) == NULL)
        goto err_3;

    bzero(indexes, sizeof(int) * g->V);
    bzero(sp->edgeTo, sizeof(struct edge_t) * g->V);

    sp->disTo[s] = 0.0;
    indexes[s] = 1;
    for (struct edge_t *e = g->adjs[s].first; e != NULL; e = e->next) {
        int w = other(e, s);
        if (sp->disTo[w] > e->weight) {
            sp->disTo[w] = e->weight;
            sp->edgeTo[w].next = NULL;
            sp->edgeTo[w].weight = e->weight;
            sp->edgeTo[w].v = s;
            sp->edgeTo[w].w = w;
        }
        indexes[w] = 1;
    }
    
    for (int i = 0; i < g->V; i++) 
        if (indexes[i] != 1) 
            sp->disTo[i] = DBL_MAX;
        
    


}
int hasPathTo(struct DijkstraSP *sp, int v)
{
    
}

struct Stack *pathTo(struct DijkstraSP *sp, int v)
{

}

double disTo(struct DijkstraSP *sp, int v)
{
    if (!isDijkstraValid(sp)) {
        LOG("invalid argument : sp is not invalid!");
        return -1;
    }
    return sp->disTo[v];
}