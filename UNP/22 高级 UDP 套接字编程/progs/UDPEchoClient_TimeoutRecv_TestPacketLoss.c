#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

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

ssize_t udp_recv(int fd, char *buf, size_t len, struct sockaddr_in *udpaddr, socklen_t *addrlen, size_t timeout) {
	struct msghdr msg;
	
	if (udpaddr != NULL && addrlen != NULL) {
		msg.msg_name = udpaddr;
		msg.msg_namelen = *addrlen;
	}
			
	struct iovec iov[1];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (timeout > 0)
		alarm(timeout);

	ssize_t nread = 0;
	if ((nread = recvmsg(fd, &msg, 0)) < 0) 
		if (errno == EINTR)
			errno = ETIMEDOUT;
	return nread;
}

ssize_t udp_send(int fd, char *buf, size_t nsend) {
	ssize_t nwrite = 0;
	if ((nwrite = write(fd, buf, nsend)) != nsend) 
		return nwrite;
	return nwrite;
}

ssize_t lookHowManyInRecvBuffer(int fd) {
	ssize_t ncanread = 0;
	if (ioctl(fd, FIONREAD, &ncanread) < 0) {
		perror("ioctl error!");
		return -1;
	}
	return ncanread;
}

void recv_int(int signo) { return; /* just interrupt the recvmsg call */ }

int main(int argc, char *argv[]) {
	
	if (argc != 3)  {
		printf("usage : %s <hostname/ip> <port/service>\n", argv[0]);
		exit(1);
	}
	
	setbuf(stdout, NULL);

	int fd = -1;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error!");
		exit(1);
	}
	
	if (fuck(SIGALRM, recv_int) == SIG_ERR) {
		perror("fuck error!");
		exit(1);
	}

	int rawSize = 0;
	socklen_t len = sizeof(rawSize);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &rawSize, &len) < 0) {
		perror("getsockopt error!");
		exit(1);
	}
	
	printf("orignal send buffer size = %d\n", rawSize);
	
	int newSize = rawSize * 4;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &newSize, len) < 0) {
		perror("setsockopt error!");
		exit(1);
	}

	struct sockaddr_in svraddr;
	bzero(&svraddr, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(atoi(argv[2]));
	svraddr.sin_addr.s_addr = inet_addr(argv[1]);	
	
	if (connect(fd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
		perror("connect error!");
		exit(1);
	}

	ssize_t nread, nwrite;
	char buf[1400];
	for (;;) {
		/*
		 * ssize_t nBytesInRecvBuffer = 0;
		 * if ((nBytesInRecvBuffer = lookHowManyInRecvBuffer(fd)) < 0) {
		 *     printf("peek recv buffer error!");
		 *     break;
		 * }
		 * printf("%zd can read\n", nBytesInRecvBuffer);
		 */
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			if (ferror(stdin)) {
				printf("hanson fgets error!");
				exit(1);
			} else { 
				fprintf(stderr, "all data finished!");	
				break;
			}
		} 	
		if (udp_send(fd, buf, strlen(buf)) < 0) {
			perror("hanson udp_send error!");
			exit(1);
		}
		if ((nread = udp_recv(fd, buf, sizeof(buf), NULL, NULL, 2)) < 0) {
			if (errno == ETIMEDOUT)
				continue;
			perror("hanson udp_recv error!");
			exit(1);
		}
		buf[nread] = 0;
		if (fputs(buf, stdout) == EOF) {
			printf("hanson fputs error!");
			exit(1);
		}
	}
}
