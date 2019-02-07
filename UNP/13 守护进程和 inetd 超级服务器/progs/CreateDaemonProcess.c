#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

int daemon_init(const char *pname, int facility)
{
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork error!");
        return -1;
    } else if (pid)
        _exit(0);

    if (setsid() < 0)
        return -1;

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
        return -1;

    if ((pid = fork()) < 0) {
        perror("fork error");
        return -1;
    } else if (pid)
        _exit(0);

    chdir("/");

    for (int i = 0; i < 64; i++)
        close(i);

    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_RDWR);  // stdout
    open("/dev/null", O_RDWR); // stderr

    openlog(pname, LOG_PID, facility);

    return 0;
}

int main(int argc, char const *argv[])
{
    daemon_init(argv[0], LOG_USER);

    syslog(LOG_INFO, "hahahahahah");

    while (1);

    return 0;
}