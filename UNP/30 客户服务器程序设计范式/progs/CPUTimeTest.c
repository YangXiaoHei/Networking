#include <stdio.h>
#include "../../tool/TimeTool.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd = open("haha", O_CREAT | O_APPEND | O_RDWR, 0644);

    int n = 2;
    for (int i = 0; i < 100000; i++) {
        n *= n;
        write(fd, &n, sizeof(int));
        write(fd, &n, sizeof(int));
        write(fd, &n, sizeof(int));
        write(fd, &n, sizeof(int));
    }
    cpu_time();
}
