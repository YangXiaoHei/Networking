#include<stdio.h>
#include "Graph.h"
#include "log.h"
#include <time.h>
#include <stdlib.h>

#define MAX_BUF (1 << 10)
static char _internal_buffer[MAX_BUF];

static int randomBetween(int v, int w) 
{
    if (v > w) {
        LOG("invalid argument : v=%d w=%d", v, w);
        return -1;
    }
    return rand() % (w - v) + v;
}

struct G* createGraph(int V) 
{
    if (V <= 0) {
        LOG("invalid argument : V=%d", V);
        return NULL;
    }

    struct G *g = NULL;
    if ((g = malloc(sizeof(struct G))) == NULL)
        goto err;

    g->V = V;
    g->E = 0;
    if ((g->adjs = malloc(sizeof(struct adj_list_t) * V)) == NULL)
        goto err_1;

    for (int i = 0; i < V; i++) {
        g->adjs[i].size = 0;
        g->adjs[i].first = NULL;
    }

    return g;

err_1:
    free(g);
err:
    return NULL;
}

struct G* createRandomGraph(int V, int E)
{
    if (V <= 0 || E < 0) {
        LOG("invalid argument : V=%d E=%d", V, E);
        return NULL;
    }

    srand((unsigned)time(NULL));

    struct G* g = NULL;
    if ((g = createGraph(V)) == NULL)
        return NULL;

    while (E--) 
        addEdge(g, randomBetween(0, V), 
                   randomBetween(0, V), 
                   randomBetween(1, 100));
    return g;
}

void addEdge(struct G *g, int v, int w, double weight)
{
    if (g == NULL || v < 0 || v >= g->V || w < 0 || w >= g->V) {
        LOG("invalid parameter: ");
        return;
    }
    struct vertex_t *vertexv = NULL, *vertexw = NULL;
    if ((vertexv = malloc(sizeof(struct vertex_t))) == NULL) {
        LOG("malloc error for new vertex : {%d %d %.2f}", v, w, weight);
        return;
    }
    if ((vertexw = malloc(sizeof(struct vertex_t))) == NULL) {
        LOG("malloc error for new vertex : {%d %d %.2f}", v, w, weight);
        return;
    }

    vertexv->v = v;
    vertexv->w = w;
    vertexv->weight = weight;

    vertexw->v = v;
    vertexw->w = w;
    vertexw->weight = weight;

    g->E++;
    struct adj_list_t *adjv = &g->adjs[v];
    struct adj_list_t *adjw = &g->adjs[w];
    vertexv->next = adjv->first;
    adjv->first = vertexv;
    vertexw->next = adjw->first;
    adjw->first = vertexw;
}

void destroyGraph(struct G** gg) 
{
    if (gg == NULL || *gg == NULL) {
        LOG("invalid parameter : g == NULL!");
        return;
    }
    struct G *g = *gg;
    for (int i = 0; i < g->V; i++) {
        for (struct vertex_t *cur = g->adjs[i].first; cur != NULL; ) {
            struct vertex_t *tmp = cur;
            g->adjs[i].first = cur->next;
            cur = cur->next;
            free(tmp);
        }
    }
    free(g->adjs);
    free(g);
    *gg = NULL;
}

const char *toString(struct G *g) 
{
    if (g == NULL) {
        _internal_buffer[0] = 0;
        return _internal_buffer;
    }

#define APPEND(_format_, ...) (len += snprintf(_internal_buffer + len, MAX_BUF - len, _format_, ##__VA_ARGS__));
    ssize_t len = 0;
    APPEND("V = %d\n", g->V);
    APPEND("E = %d\n", g->E);
    for (int i = 0; i < g->V; i++) {
        APPEND("%d :", i);
        for (struct vertex_t *cur = g->adjs[i].first; cur != NULL; cur = cur->next) 
            APPEND(" { %d %d %.0f }", cur->v, cur->w, cur->weight);
        APPEND("\n");
    }
    _internal_buffer[len] = 0;
    return _internal_buffer;
#undef APPEND
}
