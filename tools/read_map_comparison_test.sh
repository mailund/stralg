#!/bin/bash

./match_readmap --algorithm=naive -d 2 ref.fa reads.fq | sort > naive.sam
./match_readmap --algorithm=bmh   -d 2 ref.fa reads.fq | sort > bmh.sam
./match_readmap --algorithm=kmp   -d 2 ref.fa reads.fq | sort > kmp.sam
./ac_readmap                      -d 2 ref.fa reads.fq | sort > ac.sam

if ! cmp naive.sam bmh.sam; then
	echo "Naive and BMH differ"
	exit 1
fi

if ! cmp naive.sam kmp.sam; then
	echo "Naive and KMP differ"
	exit 1
fi

if ! cmp naive.sam ac.sam; then
	echo "Naive and BMH differ"
	exit 1
fi
