#! /bin/bash

cc  tool.c TCP.c sender.c log.c -o sender && ./sender 127.0.0.1 5000
