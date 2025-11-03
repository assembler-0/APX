cmake_minimum_required(VERSION 3.30)
project(apx
        LANGUAGES CXX ASM_NASM
        VERSION 0.0.1.0
        DESCRIPTION "APX Language Compiler"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(SOURCES
    src/main.cpp
)

add_executable(apxc ${SOURCES})
