#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("usage : %s <hostname>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    struct hostent *ent = NULL;

    if ((ent = gethostbyname(hostname)) == NULL) {
        printf("gethostname error! %s : %s", hostname, hstrerror(h_errno));
        exit(1);
    }

    ent->h_length = 999;

    struct hostent *eachent = NULL;
    for (char **pptr = ent->h_addr_list; *pptr != NULL; pptr++) {
        if ((eachent = gethostbyaddr((struct in_addr *)*pptr, 4, ent->h_addrtype)) == NULL) {
            char *ip = inet_ntoa(*(struct in_addr *)*pptr);
            printf("gethostbyaddr error! %s : %s\n", ip, hstrerror(h_errno));
            continue;
        }
        printf("%d\n", ent->h_length);
        char *ip = inet_ntoa(*(struct in_addr *)*pptr);
        printf("%s -> h_name = %s\n", ip, eachent->h_name);
    }
    return 0;
}