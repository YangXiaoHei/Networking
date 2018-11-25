#! /bin/bash

if [ $# -ne 1 ]
then
    file="main.c"
else
    file=$1
fi

dir="."

output="test"

cc ${dir}/PrimMST.c ${dir}/PriorityQueue.c ${dir}/BreadthFirstPath.c ${dir}/Array.c ${dir}/Stack.c ${dir}/Dijkstra.c ${dir}/tool.c ${dir}/Graph.c $file ${dir}/log.c -o "$output" && ./"$output"
