#ifndef _GRAPH_H_
#define _GRAPH_H_

struct edge_t {
    struct edge_t *next;
    double weight;
    int v, w;
};

struct adj_list_t {
    int size;
    struct edge_t *first; 
};

struct G {
    struct adj_list_t *adjs;
    int V;
    int E;
    int(*mapper)(unsigned char);
};

#define EWG_FOREACH(_G_, _var_, _vertex_) \
for ((_var_) = (_G_)->adjs[(_vertex_)].first; \
     (_var_) != NULL;                       \
     (_var_) = (_var_)->next)               

struct G*   edgeWeightedGraphInit(int V);
struct G*   edgeWeightedGraphInitWithMapping(int V, int(*mapper)(unsigned char));
struct G*   edgeWeightedGraphRandomInit(int V, int E);
void        edgeWeightedGraphRelease(struct G** g); 
int         edgeWeightedGraphGetVertexCount(struct G *g);
int         edgeWeightedGraphGetEdgeCount(struct G *g);
void        edgeWeightedGraphAddEdge(struct G *g, int v, int w, double weight);
void        edgeWeightedGraphAddEdgeByMapping(struct G *g, unsigned char v, unsigned char w, double weight);
const char *edgeWeightedGraphToString(struct G *g);
const char *edgeWeightedGraphToStringWithMapping(struct G *g, unsigned char(*mapper)(int v));

int     weightedEdgeGetEither(struct edge_t *e);
int     weightedEdgeGetOther(struct edge_t *e, int v);
double  weightedEdgeGetWeight(struct edge_t *e);
int     weightedEdgeIsValid(struct edge_t *e);
void    weightedEdgeInvalidate(struct edge_t *e);
void    weightedEdgeSet(struct edge_t *e, int v, int w, double weight, struct edge_t *next);

#endif