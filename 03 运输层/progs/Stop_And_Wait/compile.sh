#! /bin/bash

cc  tool.c TCP.c sender.c log.c -o sender
cc tool.c TCP.c receiver.c log.c -o receiver
