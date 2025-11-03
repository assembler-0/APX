
#pragma once

#include "AST.h"
#include <string>

class CodeGenerator {
public:
    std::string Generate(const Program& program);

private:
    void GenerateStatement(const Statement& statement);
    void GenerateExpression(const Expression& expression);

    std::stringstream output;
};
