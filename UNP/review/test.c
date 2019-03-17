#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char *argv[])
{
    printf("%d\n", errno);
    sleep(1);
    printf("%d\n", errno);
}
