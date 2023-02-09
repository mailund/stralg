
cmake_minimum_required(VERSION 3.2.0)
project(StrAlg)
cmake_policy(SET CMP0110 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/Modules/"
)
include(CTest)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")



# I stole this one from https://tinyurl.com/yxfkjh7j
# It lets me organise files in folders in Xcode.
MACRO(SOURCE_GROUP_BY_FOLDER target)
    SET(SOURCE_GROUP_DELIMITER "/")
    SET(last_dir "")
    SET(files "")
    FOREACH(file ${${target}_SRC} ${${target}_HEADERS})
        file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}/${target}" ${file})
        GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
        IF (NOT "${dir}" STREQUAL "${last_dir}")
            IF (files)
            SOURCE_GROUP("${last_dir}" FILES ${files})
            ENDIF (files)
            SET(files "")
        ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
        SET(files ${files} ${file})
        SET(last_dir "${dir}")
    ENDFOREACH(file)
    IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
    ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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
add_subdirectory(performance)
