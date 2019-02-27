#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define NLOOP 5000

static int counter;

void *increase(void *arg)
{
    int index = (int)arg;
    int var;
    for (int i = 0; i < NLOOP; i++) {
        var = counter;
        printf("%ld : %d\n", index,  var + 1);
        counter = var + 1;
    }
}

int main(int argc, char *argv[])
{
    int error = 0;
    pthread_t tidA, tidB;
    int i = 1, j = 2;
    if ((error = pthread_create(&tidA, NULL, increase, i)) != 0) {
        errno = error;
        perror("pthread_create fail!");
        exit(1);
    } 

    if ((error = pthread_create(&tidB, NULL, increase, j)) != 0) {
        errno = error;
        perror("pthread_create fail!");
        exit(1);
    }

    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
}
