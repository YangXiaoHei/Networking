#! /bin/bash

if [ $# -ne 1 ]
then
    echo "usage : $0 <#file_path>"
    exit
fi

file=$1

dir="Algorithms_C"

output="exec_$file"

cc ${dir}/PriorityQueue.c ${dir}/PrimMST.c ${dir}/Array.c ${dir}/Stack.c ${dir}/Dijkstra.c ${dir}/tool.c ${dir}/Graph.c $file ${dir}/log.c -o "${output}" && ./"${output}"


