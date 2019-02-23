#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#define SVR_PORT 50001

typedef void(*sig_handler)(int);

sig_handler fuck(int signo, sig_handler newhandler)
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_handler = newhandler;
    
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        newact.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        newact.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &newact, &oldact) < 0)
        return SIG_ERR;
    return oldact.sa_handler;
}

int udp_server(unsigned short port)
{
    int fd = -1;
    if ((fd = socket(AF_INET,SOCK_DGRAM, 0)) < 0) {
        perror("socket error!");
        return -1;
    }
    struct sockaddr_in svraddr;
    bzero(&svraddr, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt error!");
        return -1;
    }
    
    if (bind(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    return fd;
}

static unsigned short port_used = SVR_PORT;

int main(int argc, char *argv[]) 
{
    int fd = -1;
    if ((fd = udp_server(SVR_PORT)) < 0) {
        perror("udp_server start error!");
        exit(1);
    }        
    
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    
    char c = 0; 
    char buf[1024];
    ssize_t nread = 0;
    for (;;) {
        socklen_t len = clilen;
        if ((nread = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("recvfrom error!");
            continue;
        }
        buf[nread] = 0;
        printf("recv from client %s\n", buf);
        if (strncmp(buf, "syn", 3) != 0)
            continue;
        
        if (port_used - SVR_PORT > 5) {
            printf("connection refused!\n");    
            continue;
        }
        int connfd = -1;
        if ((connfd = udp_server(++port_used)) < 0) {
            perror("udp_server select new sockaddr fail!\n");
            exit(1);
        }
        pid_t pid;
        if ((pid = fork()) < 0) {
            perror("fork error!");
            exit(1);
        } else if (pid == 0) {
            snprintf(buf, sizeof(buf), "ack:%d", port_used);
            if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
                perror("sendto error!");
                exit(0);
            }       
            for (;;) {
                char buf[1024];
                socklen_t len = clilen;
                ssize_t nread = 0;
                if ((nread = recvfrom(connfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
                    perror("recvfrom error!");
                    break;
                }
                buf[nread] = 0;
                if (strncmp(buf, "fin", 3) == 0) {
                    printf("udp connection close\n");   
                    break;
                }
                if (sendto(connfd, buf, nread, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
                    perror("sendto error!");
                    break;
                }
            }     
            exit(0); 
        }
        /* parent process */
        close(connfd);
    }    
}
