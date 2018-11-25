#! /bin/bash

if [ $# -ne 1 ]
then
    file="main.c"
else
    file=$1
fi

dir="."

cc ${dir}/PriorityQueue.c ${dir}/BreadthFirstPath.c ${dir}/Array.c ${dir}/Stack.c ${dir}/Dijkstra.c ${dir}/tool.c ${dir}/Graph.c $file ${dir}/log.c -o "$file.exe" && ./"$file.exe"
