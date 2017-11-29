
cmake_minimum_required(VERSION 3.2.0)
set(CMAKE_C_FLAGS "-std=c99")
project(StrAlg)
include(CTest)

add_subdirectory(stralg)
add_subdirectory(tests)
add_subdirectory(tools)