#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	int bytes = 100 << 10;  /* 3M */
	if (argc == 2)
		bytes = atoi(argv[1]) << 10;
	
	if (access("data", F_OK) == 0)
		remove("data");

    srand((unsigned int)time(NULL));
    int fd = open("data", O_RDWR | O_CREAT, 0644);
    const char *msg = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < bytes; i++) {
        int ridx = rand() % strlen(msg);
        write(fd, &msg[ridx], 1);
    }
    return 0;
}
