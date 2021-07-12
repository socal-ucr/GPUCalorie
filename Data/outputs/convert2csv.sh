#!/usr/bin/env bash

for file in `ls *.grid.steady`
do
    echo $file
    awk '{print $2}' $file |tr '\n' ',' | sed 's|,,|\n|g' > temp
    mv temp $file
done
