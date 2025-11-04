#pragma once

#include <string>
#include <vector>

class ErrorReporter {
public:
    void AddError(const std::string& message, int line, int column);
    [[nodiscard]] bool HasErrors() const;
    void PrintErrors() const;

private:
    struct Error {
        std::string message;
        int line;
        int column;
    };
    std::vector<Error> errors;
};