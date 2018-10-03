
#ifndef _COMM_H_
#define _COMM_H_

struct packet_t {
    unsigned short seq;
    unsigned short checksum;
    unsigned short isACK;
    unsigned short isACKed;
    char data[8];
};

#define SENDER_WIN_SZ 5
#define TIMEOUT_INTERVAL 3000000

#endif