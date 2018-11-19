#ifndef _GRAPH_H_
#define _GRAPH_H_

struct vertex_t {
    struct vertex_t *next;
    double weight;
    int v, w;
};

struct adj_list_t {
    int size;
    struct vertex_t *first; 
};

struct G {
    struct adj_list_t *adjs;
    int V;
    int E;
};

struct G* createGraph(int V);
struct G* createRandomGraph(int V, int E);
void destroyGraph(struct G** g); 
void addEdge(struct G *g, int v, int w, double weight);
const char *toString(struct G *g);

#endif