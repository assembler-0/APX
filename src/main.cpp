#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "ArgParser.h"


int main(int argc, char **argv) {
    ArgParser arg_parser(argc, argv);
    auto [operation,
        inputFile,
        outputFile,
        showHelp,
        hasError,
        errorMessage
        ] = arg_parser.Parse();

    if (showHelp) {
        ArgParser::PrintUsage(argv[0]);
        return 0;
    }

    if (hasError) {
        std::cerr << "Error: " << errorMessage << std::endl;
        ArgParser::PrintUsage(argv[0]);
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
    ErrorReporter errorReporter;
    Parser parser(lexer, errorReporter);
    auto program = parser.ParseProgram();

    if (errorReporter.HasErrors()) {
        errorReporter.PrintErrors();
        return 1;
    }

    if (operation == APXC_OPERATION::APXC_PREPROCESS) {
        std::cout << program->ToString() << std::endl;
        return 0;
    }

    CodeGenerator generator;
    std::string assembly = generator.Generate(*program, operation);

    if (outputFile.empty()) {
        outputFile = inputFile + ".asm";
    }

    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: could not open output file " << outputFile << std::endl;
        return 1;
    }

    outFile << assembly;
    std::cout << "Compiled:  " << outputFile << std::endl;

    return 0;
}

