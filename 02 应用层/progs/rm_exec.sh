#! /bin/bash

for file in `find . -type f -print`
do
    if [ -x $file ]
    then
        rm $file
    fi
done
