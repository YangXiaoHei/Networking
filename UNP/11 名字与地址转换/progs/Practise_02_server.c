int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("usage : %s <ip/hostname> <port/service>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *service = argv[2];

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
                exitt(1);
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

        } else {
            printf("not supported socket type!\n");
            continue;
        }
    } while ((res = res->ai_next) != NULL);

    if (listenfd < 0 && udpfd < 0) {
        printf("%s %s server start fail!\n", hostname, service);
        exit(1);
    }

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
    srand((unsigned int)time(NULL)); 
    int nready = 0;
    int maxi = 1;
    int maxfd = listenfd > udpfd ? listenfd : udpfd;
    for (;;) {
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select error!");
            continue;
        }

        /* udpfd */
        if (FD_ISSET(fds[0], &rset)) {
            pid_t pid = 0;
            if ((pid = fork()) < 0) {
                perror("fork error!");
                if (--nready <= 0)
                    continue;
            } else if (pid == 0) {
                char c = 0;
                struct sockaddr_in cliaddr;
                socklen_t clilen = sizeof(cliaddr);
                if (recvfrom(fds[0], &c, 1, &cliaddr, &clilen) < 0) {
                    perror("recvfrom error!");
                    exit(1);
                }

                if (connect(fds[0], &cliaddr, clilen) < 0) {
                    perror("connect to client error!");
                    exit(1);
                }

                for (;;) {
                    int ridx = rand() % msglen;
                    ssize_t nwrite = 0;
                    if ((nwrite = write(fds[0], msg[ridx], 1)) <= 0) {
                        perror("write error!");
                        exit(1);
                    }
                }
            }
        }

        /* listenfd */
        if (FD_ISSET(fds[1], &rset)) {

        }

    }
    


    return 0;
}