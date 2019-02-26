#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>

static int sockfd;

#define QSIZE 8
#define MAXDG 4096

typedef struct {
    void *dg_data;
    size_t dg_len;
    struct sockaddr *dg_sa;
    socklen_t dg_salen;
} DG;

static DG dg[QSIZE];
static long cntread[QSIZE + 1];

static int iget;
static int iput;
static int nqueue;
static socklen_t clilen;

static void sig_io(int);
static void sig_hup(int);

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
    if (sigaction(signo, &newact, &oldact) < 0) {
        return SIG_ERR;
    }
    return oldact.sa_handler;
}

void sig_hup(int signo)
{
    for (int i = 0; i < QSIZE; i++) {
        printf("cntread[%d] = %ld\n", i, cntread[i]);
    }
}

void sig_io(int signo)
{
    printf("sig_io hahaha\n");
    int nread = 0;
    for (;;) {
        if (nqueue > QSIZE)
            exit(1);
        
        DG *ptr = &dg[iput];
        ptr->dg_salen = clilen;
        ssize_t len = 0;
        if ((len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0, ptr->dg_sa, &ptr->dg_salen)) < 0) {
           if (errno == EAGAIN)
                break; 
            perror("recvfrom error!");
            exit(1);
        }
        ptr->dg_len = len;
        nread++;
        nqueue++;
        if (++iput >= QSIZE)    
            iput = 0;
    }
    cntread[nread]++;
}

void dg_echo(int fd, struct sockaddr *cliaddr, socklen_t len)
{

    sigset_t zeromask, newmask, oldmask;

    sockfd = fd;
    clilen = len;

    for (int i = 0; i < QSIZE; i++) {
        dg[i].dg_data = malloc(MAXDG);
        dg[i].dg_sa = malloc(clilen);
        dg[i].dg_salen = clilen;
    }
    iget = iput = nqueue = 0;
    
    const int on = 1;

    fuck(SIGHUP, sig_hup);
    fuck(SIGIO, sig_io);
    fcntl(sockfd, F_SETOWN, getpid());
    ioctl(sockfd, FIOASYNC, &on);
    ioctl(sockfd, FIONBIO, &on);    
   
    sigemptyset(&zeromask);
    sigemptyset(&oldmask);
    sigemptyset(&newmask);
    
    /*
     * block the SIGIO signal
     */
    sigaddset(&newmask, SIGIO); 
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    for (;;) {
        while (nqueue == 0)
            sigsuspend(&zeromask);
        
        printf("wake up by sigio\n");
        sigprocmask(SIG_SETMASK, &oldmask, NULL);

        if (sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0, dg[iget].dg_sa, dg[iget].dg_salen) < 0) {
            perror("sendto error!");
            exit(1);
        }

        if (++iget >= QSIZE)
            iget = 0;

        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        nqueue--;
    }
}
