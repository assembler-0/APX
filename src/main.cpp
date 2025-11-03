

#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: apxc <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << argv[1] << std::endl;
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

    std::cout << program->ToString() << std::endl;

    CodeGenerator generator;
    std::string assembly = generator.Generate(*program);

    std::string outputFilename = std::string(argv[1]) + ".asm";
    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: could not open output file " << outputFilename << std::endl;
        return 1;
    }

    outputFile << assembly;

    std::cout << "Generated " << outputFilename << std::endl;

    return 0;
}

