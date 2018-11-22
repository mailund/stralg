#!/bin/sh

rm -rf CMakeCache.txt CMakeFiles CTestTestfile.cmake
rm -rf build
rm coverage.info cmake_install.cmake
(cd stralg ; rm -rf CMakeCache.txt CMakeFiles CTestTestfile.cmake)
(cd tests ; rm -rf CMakeCache.txt CMakeFiles CTestTestfile.cmake)
cmake . -DCMAKE_BUILD_TYPE=Debug
make clean

