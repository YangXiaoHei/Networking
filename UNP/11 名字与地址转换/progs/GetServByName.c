#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <name> <protocl>\n", argv[0]);
        exit(1);
    }

    struct servent *sptr = NULL;

    const char *proto = argv[2];
    if (strcmp(proto, "NULL") == 0)
        proto = NULL;
    if ((sptr = getservbyname(argv[1], proto)) == NULL) {
        perror("getservbyname error!");
        exit(1);
    }

    /*
     * struct servent {
        char *s_name;
        char **s_aliases;
        int s_port;
        char *s_proto;
     }
     */
    printf("official name = %s\n", sptr->s_name);
    for (char **pptr = sptr->s_aliases; *pptr != NULL; pptr++)
        printf("\talias = %s\n", *pptr);
    printf("port = %d\n", ntohs(sptr->s_port));
    printf("protocol = %s\n", sptr->s_proto);

    return 0;
}