#include "Graph.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    struct G *g = createRandomGraph(3, 8);
    printf("%s", toString(g));
    destroyGraph(&g);

    return 0;
}