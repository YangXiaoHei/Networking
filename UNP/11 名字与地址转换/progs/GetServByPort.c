#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <port> <protocol>\n", argv[0]);
        exit(1);
    }

    struct servent *sptr = NULL;
    if ((sptr = getservbyport(htons(atoi(argv[1])), argv[2])) == NULL) {
        printf("getservbyport error! %s\n", hstrerror(h_errno));
        exit(1);
    }

    printf("official name = %s\n", sptr->s_name);
    for (char **pptr = sptr->s_aliases; *pptr != NULL; ++pptr)
        printf("\talias : %s\n", *pptr);
    printf("\tport : %d\n", htons(sptr->s_port));
    printf("\tprotocol : %s\n", sptr->s_proto);
    return 0;
}