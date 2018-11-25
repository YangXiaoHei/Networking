#include <stdio.h>
#include <stdlib.h>
#include "Algorithms/PrimMST.h"

static unsigned char vertices[] = {
    [0] = 'z', 
    [1] = 'y', 
    [2] = 'v', 
    [3] = 't', 
    [4] = 'x', 
    [5] = 'w', 
    [6] = 'u' 
};

#define GET_VERTEX_COUNT (sizeof(vertices) / sizeof(vertices[0]))

int indexOf(unsigned char c) 
{
    for (int i = 0; i < GET_VERTEX_COUNT; i++)
        if (vertices[i] == c)
            return i;
    LOG("what a fuck?!");
    exit(1);
}
unsigned char valueOf(int i) 
{
    if (i < 0 || i >= GET_VERTEX_COUNT) {
        LOG("what a fuck?1");
        exit(1);
    }
    return vertices[i];
}

int main(int argc, char const *argv[])
{
    setbuf(stdout, NULL);

    struct G *g = edgeWeightedGraphInitWithMapping(GET_VERTEX_COUNT, indexOf);
    edgeWeightedGraphAddEdgeByMapping(g, 'z', 'y', 12);
    edgeWeightedGraphAddEdgeByMapping(g, 'z', 'x', 8);
    edgeWeightedGraphAddEdgeByMapping(g, 'y', 'x', 6);
    edgeWeightedGraphAddEdgeByMapping(g, 'y', 't', 7);
    edgeWeightedGraphAddEdgeByMapping(g, 'y', 'v', 8);
    edgeWeightedGraphAddEdgeByMapping(g, 'v', 't', 4);
    edgeWeightedGraphAddEdgeByMapping(g, 'v', 'x', 3);
    edgeWeightedGraphAddEdgeByMapping(g, 'v', 'u', 3);
    edgeWeightedGraphAddEdgeByMapping(g, 'v', 'w', 4);
    edgeWeightedGraphAddEdgeByMapping(g, 'x', 'w', 6);
    edgeWeightedGraphAddEdgeByMapping(g, 'u', 'w', 3);
    edgeWeightedGraphAddEdgeByMapping(g, 'u', 't', 2);
    printf("**************** print structure of graph ***************\n");
    printf("%s", edgeWeightedGraphToStringWithMapping(g, valueOf));
    printf("*********************************************************\n");
    
    struct PrimMST *mst = primMSTInit(g);
    primMSTDisplayAllEdgesByMapping(mst, valueOf);
    primMSTRelease(&mst);
    edgeWeightedGraphRelease(&g);
    return 0;
}
