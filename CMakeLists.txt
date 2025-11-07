cmake_minimum_required(VERSION 3.30)
project(apx
        LANGUAGES CXX ASM_NASM
        VERSION 0.0.1
        DESCRIPTION "APX Language Compiler"
        HOMEPAGE_URL "https://github.com/assembler-0/APX"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror")

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

add_library(apx SHARED ${SOURCES})

add_executable(apxc src/main.cpp)
target_link_libraries(apxc apx)
target_link_libraries(apxc fmt)

install(TARGETS apxc
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)