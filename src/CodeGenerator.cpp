
#include "CodeGenerator.h"
#include <stdexcept>

std::string CodeGenerator::Generate(const Program& program) {
    output.str("");
    output.clear();

    output << "section .text" << std::endl;
    output << "global _start" << std::endl;
    output << std::endl;
    output << "_start:" << std::endl;

    for (const auto& stmt : program.statements) {
        GenerateStatement(*stmt);
    }

    // Exit syscall
    output << "    mov rax, 60" << std::endl;
    output << "    xor rdi, rdi" << std::endl;
    output << "    syscall" << std::endl;

    return output.str();
}

void CodeGenerator::GenerateStatement(const Statement& statement) {
    if (const auto* letStmt = dynamic_cast<const LetStatement*>(&statement)) {
        // For now, just generate the expression
        GenerateExpression(*letStmt->value);
    } else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(&statement)) {
        GenerateExpression(*returnStmt->returnValue);
        output << "    mov rdi, rax" << std::endl; // Set exit code
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void CodeGenerator::GenerateExpression(const Expression& expression) {
    if (const auto* intLiteral = dynamic_cast<const IntegerLiteral*>(&expression)) {
        output << "    mov rax, " << intLiteral->value << std::endl;
    } else {
        throw std::runtime_error("Unknown expression type");
    }
}
