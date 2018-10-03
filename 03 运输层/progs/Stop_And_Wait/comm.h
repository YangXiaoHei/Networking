
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include "log.h"
#include "tool.h"
#include "TCP.h"

enum sender_state {
    sender_begin = 100,
    sender_wait_ACK_0,
    sender_wait_ACK_1,
    sender_wait_0,
    sender_wait_1,
    sender_end = 200
};

enum receiver_state {
    receiver_begin = 300,
    receiver_state_wait_0,
    receiver_state_wait_1,
    receiver_end = 400
};

struct packet_t {
    unsigned short seq;
    unsigned short checksum;
    unsigned short isACK;
    char data[8];
};

#define TIMEOUT_INTERVAL (3000000)

#endif
