#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char *msg = "hansonyang";
    char buf[64];
    memset(buf, 'a', sizeof(buf));

    /* strcpy 会自动在 buf 后填 \0 */
    strcpy(buf, msg);
    printf("%s\n", buf);

    /* snprintf 也会自动在 buf 后填 \0 */
    memset(buf, 'a', sizeof(buf));
    snprintf(buf, sizeof(buf), "hello world");
    printf("%s\n", buf);
    return 0;
}