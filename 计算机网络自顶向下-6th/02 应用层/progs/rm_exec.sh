#! /bin/bash

for file in `find . -type f -print`
do
    if [ -x $file ] && [[ "`basename $file`" != "`basename $0`" ]] 
    then
        rm $file
    fi
done
