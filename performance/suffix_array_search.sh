#!/bin/bash

program=../xcodebuild/performance/Debug/suffix_array_search

for (( n = 500; n <= 600; n += 10 ))
do
    for (( m = 100; m <= 500; m += 100 ))
    do
        for alg in "BWT" "SA" "ST";
        do
            $program $alg $n $m
        done
    done
done
