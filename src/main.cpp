#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"

int main(int argc, char **argv) {
    bool printAST = false;
    std::string inputFile;
    std::string outputFile;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-E") {
            printAST = true;
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (inputFile.empty()) {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Usage: apxc [-E] [-o output] <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << inputFile << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.ParseProgram();

    if (!program) {
        std::cerr << "Error: failed to parse program" << std::endl;
        return 1;
    }

    if (printAST) {
        std::cout << program->ToString() << std::endl;
        return 0;
    }

    CodeGenerator generator;
    std::string assembly = generator.Generate(*program);

    if (outputFile.empty()) {
        outputFile = inputFile + ".asm";
    }

    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: could not open output file " << outputFile << std::endl;
        return 1;
    }

    outFile << assembly;
    std::cout << "Generated " << outputFile << std::endl;

    return 0;
}

