#!/bin/sh

./fasta_test test-data/ref.fa > test-data/fasta-test-observed.txt

if cmp test-data/fasta-test-observed.txt test-data/fasta-test-expected.txt;
then
    echo "ok"

else
    echo "error"
    exit 1
fi
