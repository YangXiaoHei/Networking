#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static char absloutePath[1024];

char *getAbsolutePath(const char *argv0)
{
    char buf[1024];
    getcwd(buf, sizeof(buf));
    snprintf(absloutePath, 1024, "%s/%s", buf, argv0);
    return absloutePath;
}
