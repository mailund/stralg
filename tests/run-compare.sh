#!/bin/sh

prog=$1
shift

expected="test-data/${prog}_expected.txt"
current="test-data/${prog}_current.txt"

./$prog $@ > $current
cmp $current $expected
