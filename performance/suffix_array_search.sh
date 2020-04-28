#!/bin/bash

program=./suffix_array_search

#for (uint32_t n = 10000; n <= 250000; n += 1000) {
for (( n = 100000; n <= 10000000; n += 100000 ))
do
    for (( m = 100; m <= 500; m += 100 ))
    do
        for alg in "BWT" "SA" "ST";
        do
            $program $alg $n $m
        done
    done
done
