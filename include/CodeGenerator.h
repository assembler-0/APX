
#pragma once

#include <sstream>
#include "AST.h"
#include <string>

#include "SymbolTable.h"

class CodeGenerator {
public:
    std::string Generate(const Program& program);

private:
    void GenerateStatement(const Statement& statement);
    void GenerateExpression(const Expression& expression);

    std::stringstream output;
    SymbolTable symbolTable;
    std::unordered_map<std::string, const FunctionDeclaration*> functions;
};
