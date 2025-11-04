#pragma once

#include <string>
#include "CodeGenerator.h"

struct CompileConfiguration {
    APXC_OPERATION operation = APXC_OPERATION::APXC_UNKNOWN;
    std::string inputFile;
    std::string outputFile;
    bool showHelp = false;
    bool hasError = false;
    std::string errorMessage;
};

class ArgParser {
public:
    ArgParser(int argc, char **argv);
    [[nodiscard]] CompileConfiguration Parse() const;
    static void PrintUsage(const std::string& programName);

private:
    int argc;
    char **argv;
};