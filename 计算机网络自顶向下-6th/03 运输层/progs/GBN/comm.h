
#ifndef _COMM_H_
#define _COMM_H_

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

struct packet_t {
    unsigned char isACK;
    unsigned char isACKed;
    unsigned char isTransmited;
    unsigned short seq;
    unsigned short checksum;
    unsigned long timeout_timestamp;
    char data[8];
};

#define SENDER_WIN_SZ 4
#define TIMEOUT_INTERVAL 3000000

#endif