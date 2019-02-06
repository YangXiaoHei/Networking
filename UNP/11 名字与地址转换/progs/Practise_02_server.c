#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

typedef void(*sig_handler)(int);

sig_handler fuck(int signo, sig_handler newhandler) 
{
    struct sigaction newact, oldact;
    bzero(&newact, sizeof(newact));
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
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

void child_collect(int signo) 
{
    pid_t pid = 0;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("collcet child %ld\n", (long)pid);
    }
    if (pid < 0 && errno != ECHILD) {
        perror("waitpid error!");
    }
}


int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3) {
        printf("usage : %s [ip/hostname] <port/service>\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    const char *hostname = NULL;
    const char *service = NULL;

    if (argc == 2) {
        hostname = NULL;
        service = argv[1]; 
    } else {
        hostname = argv[1];
        service = argv[2];
    }

    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = 0;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;

    int listenfd = -1, udpfd = -1;
    do {    
        if (res->ai_socktype == SOCK_DGRAM) {

            if (udpfd >= 0) {
                printf("same port use for 2 udp service!\n");
                exit(1);
            }

            if ((udpfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
                perror("socket udp fd create fail!");
                continue;
            }

            int sndbuf = 220 << 10;
            if (setsockopt(udpfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
                perror("udp set send buffer size error!");
                close(udpfd);
                continue;
            }

            if (bind(udpfd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("udp fd bind error!");
                close(udpfd);
            }

        } else if (res->ai_socktype == SOCK_STREAM) {

            if (listenfd >= 0) {
                printf("same port use for 2 tcp service\n!");
                exit(1);
            }

            if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
                perror("socket tcp fd create fail!");
                continue;
            }

            int on = 1;
            if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
                perror("tcp fd set reuse addr error!");
                close(listenfd);
                continue;
            }

            if (bind(listenfd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("tcp fd bind error!");
                close(listenfd);
            }

            if (listen(listenfd, 1000) < 0) {
                perror("tcp fd listen error!");
                close(listenfd);
            }

        } else {
            printf("not supported socket type!\n");
            continue;
        }
    } while ((res = res->ai_next) != NULL);

    if (listenfd < 0 && udpfd < 0) {
        printf("%s %s server start fail!\n", hostname, service);
        exit(1);
    }
    printf("listenfd = %d udpfd = %d\n", listenfd, udpfd);

    fuck(SIGCHLD, child_collect);

    int fds[1024];
    for (int i = 0; i < 1024; i++)
        fds[i] = -1;

    fd_set allset, rset;
    if (udpfd >= 0) {
        fds[0] = udpfd;
        FD_SET(fds[0], &allset);
    }
    if (listenfd >= 0) {
        fds[1] = listenfd;
        FD_SET(fds[1], &allset);
    }

    int connfd = -1;
    char *msg = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t msglen = strlen(msg);
    int nready = 0;
    int maxi = 2;
    int maxfd = listenfd > udpfd ? listenfd : udpfd;

    pid_t pids[1024];
    for (int i = 0; i < 1024; i++)
        pids[i] = -1;

    for (;;) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            continue;
        }
        printf("%d fds readable\n", nready);

        /* udpfd */
        if (FD_ISSET(fds[0], &rset)) {
            pid_t pid = 0;
            if ((pid = fork()) < 0) {
                perror("fork error!");
                if (--nready <= 0)
                    continue;
            } else if (pid == 0) {
                printf("new process created for udp: pid=%ld\n", (long)pid);
                srand((unsigned int)time(NULL)); 
                char c = 0;
                struct sockaddr_in cliaddr;
                socklen_t clilen = sizeof(cliaddr);
                if (recvfrom(fds[0], &c, 1, 0, (struct sockaddr *)&cliaddr, &clilen) < 0) {
                    perror("recvfrom error!");
                    exit(1);
                }

                if (connect(fds[0], (struct sockaddr *)&cliaddr, clilen) < 0) {
                    perror("connect to client error!");
                    exit(1);
                }

                for (;;) {
                    int ridx = rand() % msglen;
                    ssize_t nwrite = 0;
                    if ((nwrite = write(fds[0], &msg[ridx], 1)) <= 0) {
                        perror("write error!");
                        printf("child process exit with code 0 pid=%ld\n", (long)getpid());
                        exit(0);
                    }
                }
            }
        }

        /* listenfd */
        if (FD_ISSET(fds[1], &rset)) {
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(clilen);
            if ((connfd = accept(fds[1], (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept error!");
                --nready;
                continue;
            }

            int i = 2;
            for (; i < 1024; i++) {
                if (fds[i] < 0) {
                    fds[i] = connfd;
                    FD_SET(connfd, &allset);
                    break;
                }
            }
            if (i == 1024) {
                printf("too many connections\n");
                close(connfd);
                if (--nready <= 0) 
                    continue;
            }

            if (connfd > maxfd)
                maxfd = connfd;

            if (i > maxi)
                maxi = i;

            pid_t pid = 0;
            if ((pid = fork()) < 0) {
                perror("fork error!");
            } else if (pid == 0) {
                printf("new process created for udp: pid=%ld\n", (long)pid);
                srand((unsigned int)time(NULL)); 
                for (;;) {
                    int ridx = rand() % msglen;
                    ssize_t nwrite = 0;
                    if ((nwrite = write(connfd, &msg[ridx], 1)) != 1) {
                        if (errno == EPIPE) {
                            close(connfd);
                            printf("client cut connection\n");
                            printf("child process exit with code 0 pid=%ld\n", (long)getpid());
                            exit(0);
                        }
                    }
                }
            } 

            pids[connfd] = pid;

            if (--nready <= 0)
                continue;
        }

        for (int i = 2; i <= maxi; i++) {
            if (fds[i] < 0) 
                continue;

            if (FD_ISSET(fds[i], &rset)) {
                char c = 0;
                ssize_t nread = 0;
                if ((nread = read(fds[i], &c, 1)) < 0) {
                    if (errno == ECONNRESET) {
                        close(connfd);
                        printf("client cut connection\n");
                        if (pids[connfd] > 0) {
                            printf("kill child process for tcp: pid=%ld\n", (long)pids[connfd]);
                            kill(pids[connfd], SIGKILL);
                            pids[connfd] = -1;
                        }
                    }
                } else if (nread == 0) {
                    close(connfd);
                    printf("client cut connection\n");
                    if (pids[connfd] > 0) {
                        printf("kill child process for tcp: pid=%ld\n", (long)pids[connfd]);
                        kill(pids[connfd], SIGKILL);
                        pids[connfd] = -1;
                    }
                }

                if (--nready <= 0)
                    continue;
            }
        }
    }
    return 0;
}