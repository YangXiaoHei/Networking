#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include "../../tool/AddrTool.h"

int main(int argc, char const *argv[])
{
    struct sockaddr_storage cliaddr;
    socklen_t len = sizeof(cliaddr);
    
    openlog(argv[0], LOG_PID, LOG_INFO);

    char c = 0;
    if (recvfrom(0, &c, 1, 0, (struct sockaddr *)&cliaddr, &len) < 0) {
        syslog(LOG_INFO, "recvfrom error!");
        exit(1);
    }

    char *addrInfo = getAddrInfo((struct sockaddr_in *)&cliaddr);
    syslog(LOG_INFO, "udp req from %s\n", addrInfo);
    free(addrInfo);

    time_t ticks = time(NULL);
    char buf[1024];
    snprintf(buf, sizeof(buf), "%.24s", ctime(&ticks));
    if (sendto(0, buf, strlen(buf), 0, (struct sockaddr *)&cliaddr, len) < 0) {
        syslog(LOG_INFO, "sento error! : %s\n", strerror(errno));
        exit(1);
    }
    close(0);
    exit(0);
}