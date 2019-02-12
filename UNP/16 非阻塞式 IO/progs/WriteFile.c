#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    srand((unsigned int)time(NULL));
    int fd = open("data", O_RDWR | O_CREAT, 0644);
    const char *msg = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 50000; i++) {
        int ridx = rand() % strlen(msg);
        write(fd, &msg[ridx], 1);
    }
    return 0;
}