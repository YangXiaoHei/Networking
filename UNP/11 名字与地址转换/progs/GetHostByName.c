#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        printf("usage : %s [hostname1, hostname2, ...]\n", argv[0]);
        exit(1);
    }

    struct hostent *ent = NULL;
    while (--argc > 0) {
        const char *name = *++argv;
        if ((ent = gethostbyname(name)) == NULL) {
            printf("gethostbyname error: %s %s\n", name, hstrerror(h_errno));
            continue;
        }

        printf("official hostname : %s\n", ent->h_name);
        for (char **pptr = ent->h_aliases; *pptr != NULL; pptr++)
            printf("\talias : %s\n", *pptr);

        switch(ent->h_addrtype) {
            case AF_INET : {
                char **pptr = ent->h_addr_list;
                for (; *pptr != NULL; pptr++) 
                    printf("\taddress: %s\n", inet_ntoa(*(struct in_addr *)*pptr));
                break;
            };
            default : 
                printf("unknow address type");
                break;
        }
    }
    return 0;
}