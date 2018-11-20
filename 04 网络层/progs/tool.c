#include "tool.h"
#include "log.h"

int randomBetween(int v, int w) 
{
    if (v > w) {
        LOG("invalid argument : v=%d w=%d", v, w);
        return -1;
    }
    return rand() % (w - v) + v;
}
