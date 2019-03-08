#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include "CommonTool.h"

int tcp_connect_cb(const char *hostname, const char *service, int(*cb)(int))
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    saveerrno = 0;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }

        if (cb && cb(fd) < 0) {
            saveerrno = errno;
            continue;
        }

        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int tcp_connect(const char *hostname, const char *service)
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int tcp_server(const char *service)
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(NULL, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n",, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        int on = 1;
        if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
            saveerrno = errno;
            continue;
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        if (listen(fd, 1000000) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int tcp_server_cb(const char *service, int(*cb)(int))
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(NULL, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n", service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        if (cb && cb(fd) < 0) {
            saveerrno = errno;
            continue;
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        if (listen(fd, 1000000) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}
int udp_connect_cb(const char *hostname, const char *service, int(*cb)(int))
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        if (cb && cb(fd) < 0) {
            saveerrno = errno;
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int udp_connect(const char *hostname, const char *service)
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s %s : %s\n", hostname, service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int udp_server(const char *service)
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(NULL, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n", service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        int on = 1;
        if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
            saveerrno = errno;
            continue;
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}

int udp_server_cb(const char *service, int(*cb)(int))
{
    int fd = -1;
    struct addrinfo hints, *res = NULL, *ressave = NULL;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    int error = 0;
    if ((error = getaddrinfo(NULL, service, &hints, &res)) != 0) {
        printf("getaddrinfo error! %s : %s\n", service, gai_strerror(error));
        exit(1);
    }
    ressave = res;
    do {
        if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            saveerrno = errno;
            continue;
        }
        if (cb && cb(fd) < 0) {
            saveerrno = errno;
            continue;
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
            saveerrno = errno;
            continue;
        }
        break;
    } while ((res = res->ai_next) != NULL);
    if (res == NULL) {
        errno = saveerrno;
        return -1;
    }
    freeaddrinfo(ressave);
    return fd;
}
