cmake_minimum_required(VERSION 3.30)
project(apx
        LANGUAGES CXX ASM_NASM
        VERSION 0.0.1.0
        DESCRIPTION "APX Language Compiler"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(SOURCES
    src/Lexer.cpp
    src/AST.cpp
    src/Parser.cpp
    src/CodeGenerator.cpp
    src/SymbolTable.cpp
    src/ArgParser.cpp
    src/ErrorReporter.cpp
)

include_directories(
        include
)

add_library(apxlib STATIC ${SOURCES})

add_executable(apxc src/main.cpp)
target_link_libraries(apxc apxlib)

add_custom_target(run
        COMMAND apxc ${CMAKE_CURRENT_BINARY_DIR}/main.apx
        VERBATIM
)