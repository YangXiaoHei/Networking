
#ifndef TCP_LEVEL_H
#define TCP_LEVEL_H

int TCP_service_init(unsigned short bind_port);
int TCP_connect(const char *peer_ip, unsigned short peer_port);
int TCP_close(int fd);
ssize_t TCP_recv(int fd, char *recv_buf, ssize_t maxlen);
ssize_t TCP_send(int fd, const char *send_buf, ssize_t send_buf_len);

#endif /* TCP_LEVEL_H */