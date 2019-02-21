
cmake_minimum_required(VERSION 3.2.0)
project(StrAlg)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/Modules/"
)
include(CTest)
include(GoogleTest)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME coverage
        EXECUTABLE ctest
    )
endif(CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory(stralg)
add_subdirectory(bioinf)
add_subdirectory(tests)
add_subdirectory(tools)

