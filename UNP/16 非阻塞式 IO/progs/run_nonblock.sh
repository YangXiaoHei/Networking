#! /bin/bash

conn=5

if [ $# -eq 1 ]
then
    conn=$1
fi

cc WebClientMultiConnectDownload_Nonblock.c TimeTool.c && ./a.out $conn hansonyang.com 50001 img_1.jpg img_2.jpg img_3.jpg img_4.jpg img_5.jpg img_6.jpg
