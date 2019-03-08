#ifndef _COMMON_TOOL_H_
#define _COMMON_TOOL_H_

int tcp_connect_cb(const char *hostname, const char *service, int(*cb)(int));
int tcp_connect(const char *hostname, const char *service);
int tcp_server(const char *service);
int tcp_server_cb(const char *service, int(*cb)(int));
int udp_connect(const char *hostname, const char *service);
int udp_connect_cb(const char *hostname, const char *service, int(*cb)(int));
int udp_server(const char *service);
int udp_server_cb(const char *service, int(*cb)(int));

#endif
