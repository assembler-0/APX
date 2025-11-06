#include "ArgParser.h"
#include <string>
#include <iostream>

ArgParser::ArgParser(const int argc, char **argv) : argc(argc), argv(argv) {}

CompileConfiguration ArgParser::Parse() const {
    CompileConfiguration config;
    
    if (argc < 2) {
        config.hasError = true;
        config.errorMessage = "No input file specified";
        return config;
    }
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            config.showHelp = true;
            return config;
        } else if (arg == "-E") {
            if (config.operation != APXC_OPERATION::APXC_UNKNOWN) {
                config.hasError = true;
                config.errorMessage = "Multiple operation flags specified";
                return config;
            }
            config.operation = APXC_OPERATION::APXC_PREPROCESS;
        } else if (arg == "-c") {
            if (config.operation != APXC_OPERATION::APXC_UNKNOWN) {
                config.hasError = true;
                config.errorMessage = "Multiple operation flags specified";
                return config;
            }
            config.operation = APXC_OPERATION::APXC_COMPILE_WO_ENTRY;
        } else if (arg == "-o") {
            if (i + 1 >= argc) {
                config.hasError = true;
                config.errorMessage = "Option -o requires an argument";
                return config;
            }
            if (!config.outputFile.empty()) {
                config.hasError = true;
                config.errorMessage = "Multiple output files specified";
                return config;
            }
            config.outputFile = argv[++i];
        } else if (arg[0] == '-') {
            config.hasError = true;
            config.errorMessage = "Unknown option: " + arg;
            return config;
        } else {
            if (!config.inputFile.empty()) {
                config.hasError = true;
                config.errorMessage = "Multiple input files specified";
                return config;
            }
            config.inputFile = arg;
        }
    }
    
    if (config.inputFile.empty() && !config.showHelp) {
        config.hasError = true;
        config.errorMessage = "No input file specified";
        return config;
    }
    
    if (config.operation == APXC_OPERATION::APXC_UNKNOWN) {
        config.operation = APXC_OPERATION::APXC_COMPILE_W_ENTRY;
    }
    
    return config;
}

void ArgParser::PrintUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options] <input-file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -E              Preprocess only\n";
    std::cout << "  -c              Compile without entry point\n";
    std::cout << "  -o <file>       Specify output file\n";
    std::cout << "  -h, --help      Show this help message\n\n";
}
