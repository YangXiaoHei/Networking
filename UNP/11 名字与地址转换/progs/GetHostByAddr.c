#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char const *argv[])
{
    char **pptr = NULL, *ptr = NULL;
    struct hostent *ent = NULL;
    struct in_addr addr;
    while (--argc > 0) {
        const char *ip = *++argv;
        if ((addr.s_addr = inet_addr(ip)) == INADDR_NONE) {
            perror("inet_addr error!");
            continue;
        }
        if ((ent = gethostbyaddr(&addr, sizeof(addr), AF_INET)) == NULL) {
            printf("not find a PTR record for this IP addr %s\n", ip);
            continue;
        }
        printf("official name = %s\n", ent->h_name);
        for (char **pptr = ent->h_aliases; *pptr != NULL; ++pptr)
            printf("\talias : %s\n", *pptr);

        if (ent->h_addrtype == AF_INET)
            for (char **pptr = ent->h_addr_list; *pptr != NULL; ++pptr) 
                printf("\tip address : %s\n", inet_ntoa(*(struct in_addr *)*pptr));
        else
            printf("unknow addr type\n");
    }
    return 0;
}