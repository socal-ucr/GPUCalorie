#!/usr/bin/env bash

DATA_DIR="/home/mchow009/GPUCalorie/HotSpot/examples/gpu/ptraces"
for trace in *.mtrace.gz
do
    bm=${trace%".mtrace.gz"}
    echo $bm
    ./getPower.py $trace
    gunzip power_trace.gz
    sed -i 's/,/\t/g' power_trace
    mv power_trace ${DATA_DIR}/${bm}.ptrace 
done
