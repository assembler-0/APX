#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "ArgParser.h"
#include "Logger.h"
#include "Version.h"

std::mutex g_output_mutex;

int main(int argc, char **argv) {
    ArgParser arg_parser(argc, argv);
    auto [operation,
        inputFile,
        outputFile,
        showHelp,
        hasError,
        showVersion,
        errorMessage
    ] = arg_parser.Parse();

    if (showHelp) {
        ArgParser::PrintUsage(argv[0]);
        return 0;
    }

    if (showVersion) {
        out::info(APXC_VERSION);
        return 0;
    }

    if (hasError) {
        out::error(errorMessage);
        ArgParser::PrintUsage(argv[0]);
        return 1;
    }

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        out::error("Could not open input file: {}", inputFile);
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
        out::error("Could not open output file: {}", outputFile);
        return 1;
    }

    outFile << assembly;
    out::success("Compiled: {} from: {}", outputFile, inputFile);

    return 0;
}

