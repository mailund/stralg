#!/bin/sh

prog=$1
echo $prog

expected="test-data/${prog}_expected.txt"
current="test-data/${prog}_current.txt"

./$prog | sort > $current
cmp $current $expected
