
#pragma once

#include <sstream>
#include "AST.h"
#include <string>
#include "SymbolTable.h"

enum class APXC_OPERATION {
    APXC_COMPILE_W_ENTRY,
    APXC_COMPILE_WO_ENTRY,
    APXC_PREPROCESS,
    APXC_UNKNOWN,
};

class CodeGenerator {
public:
    std::string Generate(const Program& program, APXC_OPERATION operation);

private:
    void GenerateStatement(const Statement& statement);
    void GenerateExpression(const Expression& expression);

    std::stringstream output;
    SymbolTable symbolTable;
    std::unordered_map<std::string, const FunctionDeclaration*> functions;
};
