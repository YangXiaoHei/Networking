#! /bin/bash

if [ $# -ne 1 ]
then
    echo "usage : $0 <#file_path>"
    exit
fi

file=$1

dir="DijkstraSP"

cc ${dir}/Array.c ${dir}/Stack.c ${dir}/Dijkstra.c ${dir}/tool.c ${dir}/Graph.c $file ${dir}/log.c -o "$file.exe" && ./"$file.exe"


