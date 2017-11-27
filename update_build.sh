#!/bin/bash

rm CMakeCache.txt
cmake . -GXcode
rm CMakeCache.txt
cmake . -G"Unix Makefiles"
