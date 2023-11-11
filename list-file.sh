#!/bin/bash

if [ $# -ne 1 ]; then # $# is the number of arguments, -ne = !=
    echo "Missing directory name."
    exit 1 # exit with error code 1
fi

if [ ! -d "$1" ]; then  # if $1 is not a directory, -d = directory, ! = not
    echo "The specified path is not a directory."
    exit 2
fi

for file in "$1"*; do # "$1"/* = all files in directory
    if [ -f "$file" ]; then # -f = test if file exists syntax = [ -f file ]
        echo "$file"
    fi
done

exit 0