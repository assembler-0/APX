
#include "CodeGenerator.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

// Helper function to compile and run APX code, returning the exit code
int compileAndRun(const std::string& apxCode) {
    // Write APX code to a temporary file
    std::string apxFilename = "temp.apx";
    std::ofstream apxFile(apxFilename);
    if (!apxFile.is_open()) {
        throw std::runtime_error("Could not create " + apxFilename);
    }
    apxFile << apxCode;
    apxFile.close();

    // Lex, Parse, and Generate Assembly
    Lexer lexer(apxCode);
    Parser parser(lexer);
    auto program = parser.ParseProgram();
    if (!program) {
        throw std::runtime_error("Failed to parse program");
    }

    CodeGenerator generator;
    std::string assembly = generator.Generate(*program);

    // Write assembly to a temporary .asm file
    std::string asmFilename = "temp.asm";
    std::ofstream asmFile(asmFilename);
    if (!asmFile.is_open()) {
        throw std::runtime_error("Could not create " + asmFilename);
    }
    asmFile << assembly;
    asmFile.close();

    // Assemble using NASM
    std::string nasmCommand = "nasm -f elf64 " + asmFilename + " -o temp.o";
    if (system(nasmCommand.c_str()) != 0) {
        throw std::runtime_error("NASM assembly failed");
    }

    // Link using LD
    std::string ldCommand = "ld temp.o -o temp -e main";
    if (system(ldCommand.c_str()) != 0) {
        throw std::runtime_error("LD linking failed");
    }

    // Run the executable and get the exit code
    int exitCode = system("./temp");

    // Clean up temporary files
    system(("rm " + apxFilename + " " + asmFilename + " temp.o temp").c_str());

    // The exit code from system() is usually WEXITSTATUS(status) << 8
    // So, we need to shift it right by 8 bits to get the actual exit code.
    return WEXITSTATUS(exitCode) << 8;
}

int main() {
    // Test 1: Simple function call
    try {
        std::string code = R"(
            fn add(a: i32, b: i32) -> i32 {
                return a + b;
            }
            fn main() -> i32 {
                return add(5, 7);
            }
        )";
        int result = compileAndRun(code);
        if (result != 12) {
            std::cerr << "Test 1 Failed: Expected 12, got " << result << std::endl;
            return 1;
        }
        std::cout << "Test 1 Passed" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 Failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
