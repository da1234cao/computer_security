#!/bin/sh

SECONDS=0
value=0

while [ 1 ]; do
    value=$(($value +1))
    duration=$SECONDS
    min=$(($duration / 60))
    sec=$(($duration % 60))
    echo " it has tried $min mins $sec ses"
    echo "it has tried $value times"
    ./stack
done
