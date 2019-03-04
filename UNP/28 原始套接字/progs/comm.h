#ifndef _COMM_H_
#define _COMM_H_

struct icmpd_err {
    int icmpd_errno;
    char icmpd_type;
    char icmpd_code;
    socklen_t icmpd_len;
    struct sockaddr_storage icmpd_dst;
};

#endif
