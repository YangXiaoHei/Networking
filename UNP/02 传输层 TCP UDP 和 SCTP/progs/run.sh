#! /bin/bash

cc ConnectAtTheSameTime.c

killall a.out

./a.out 127.0.0.1 50001 53000 &
./a.out 127.0.0.1 53000 50001 &
sleep 1
netstat -an | grep -e 50001 -e 53000
