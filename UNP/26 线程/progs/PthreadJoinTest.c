#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *doit(void *arg)
{
    int index = (int)arg;
    int ri = 10 - index;
    printf("thread %d sleep %d seconds\n", index, ri);
    sleep(ri);
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[10];
    
    for (int i = 0; i < 10; i++) {
        pthread_create(tid + i, NULL, doit, i);
    }

    for (int i = 0; i < 10; i++) {
        printf("begin to join %d\n", i);
        pthread_join(tid[i], NULL);
        printf("end join %d\n", i);
    }
}
