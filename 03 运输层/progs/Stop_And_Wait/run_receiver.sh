#! /bin/bash

cc  tool.c TCP.c receiver.c log.c -o receiver && ./receiver 5000
