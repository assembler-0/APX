#include "ErrorReporter.h"
#include <iostream>

void ErrorReporter::AddError(const std::string& message, const int line, const int column) {
    errors.push_back({message, line, column});
}

bool ErrorReporter::HasErrors() const {
    return !errors.empty();
}

void ErrorReporter::PrintErrors() const {
    for (const auto&[message, line, column] : errors) {
        std::cerr << "Error at line " << line << ", column " << column << ": " << message << std::endl;
    }
}