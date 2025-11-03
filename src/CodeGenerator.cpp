
#include "CodeGenerator.h"
#include <iostream>
#include <stdexcept>

std::string CodeGenerator::Generate(const Program& program) {
    output.str("");
    output.clear();

    output << "section .text" << std::endl;
    output << "global main" << std::endl;
    output << std::endl;

    // First pass: Populate the functions map
    for (const auto& stmt : program.statements) {
        if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
            std::string funcName = funcDecl->name->value;
            functions[funcName] = funcDecl;
        }
    }

    // Second pass: Generate code for all function declarations
    for (const auto& stmt : program.statements) {
        if (dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
            GenerateStatement(*stmt); // This will generate the function's code
        }
    }

    // Main entry point
    output << "main:" << std::endl;
    output << "    push rbp" << std::endl;
    output << "    mov rbp, rsp" << std::endl;
    output << "    sub rsp, 8" << std::endl; // Allocate space for one variable (for now)

    symbolTable.EnterScope();

    // Call the 'main' function of the APX program
    // For now, assume there's a function named 'main'
    output << "    call main" << std::endl; // Assuming the APX main function is named main_apx

    symbolTable.LeaveScope();

    // Exit syscall
    output << "    mov rax, 60" << std::endl;
    output << "    leave" << std::endl;
    output << "    syscall" << std::endl;
    output << "    ret" << std::endl;

    return output.str();
}

void CodeGenerator::GenerateStatement(const Statement& statement) {
    if (const auto* letStmt = dynamic_cast<const LetStatement*>(&statement)) {
        symbolTable.Define(letStmt->name->value);
        GenerateExpression(*letStmt->value);
        output << "    mov [rbp" << symbolTable.Get(letStmt->name->value) << "], rax" << std::endl;
    } else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(&statement)) {
        GenerateExpression(*returnStmt->returnValue); // result in rax
        output << "    leave" << std::endl;
        output << "    ret" << std::endl;
    } else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(&statement)) {
        std::string funcName = funcDecl->name->value;
        output << funcName << ":" << std::endl;
        output << "    push rbp" << std::endl;
        output << "    mov rbp, rsp" << std::endl;
        // TODO: Allocate stack space for local variables

        symbolTable.EnterScope();

        // Process parameters and add them to symbol table
        int paramOffset = 16; // rbp + 8 is return address, rbp + 16 is first argument
        for (const auto& param : funcDecl->parameters) {
            symbolTable.DefineParameter(param->value, paramOffset);
            paramOffset += 8;
        }

        for (const auto& stmt : funcDecl->body->statements) {
            GenerateStatement(*stmt);
        }

        symbolTable.LeaveScope();

        output << "    leave" << std::endl;
        output << "    ret" << std::endl;
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void CodeGenerator::GenerateExpression(const Expression& expression) {
    if (const auto* intLiteral = dynamic_cast<const IntegerLiteral*>(&expression)) {
        output << "    mov rax, " << intLiteral->value << std::endl;
    } else if (const auto* ident = dynamic_cast<const Identifier*>(&expression)) {
        int offset = symbolTable.Get(ident->value);
        if (offset >= 0) {
            output << "    mov rax, [rbp+" << offset << "]" << std::endl;
        } else {
            output << "    mov rax, [rbp" << offset << "]" << std::endl;
        }
    } else if (const auto* infix = dynamic_cast<const InfixExpression*>(&expression)) {
        GenerateExpression(*infix->left);
        output << "    push rax" << std::endl;
        GenerateExpression(*infix->right);
        output << "    pop rbx" << std::endl;
        if (infix->op == "+") {
            output << "    add rax, rbx" << std::endl;
        } else if (infix->op == "-") {
            output << "    sub rbx, rax" << std::endl;
            output << "    mov rax, rbx" << std::endl;
        } else if (infix->op == "*") {
            output << "    mul rbx" << std::endl;
        } else if (infix->op == "/") {
            output << "    xor rdx, rdx" << std::endl;
            output << "    div rbx" << std::endl;
        } else {
            throw std::runtime_error("Unknown infix operator: " + infix->op);
        }
    } else if (const auto* call = dynamic_cast<const CallExpression*>(&expression)) {
        // Push arguments onto the stack in reverse order
        for (auto it = call->arguments.rbegin(); it != call->arguments.rend(); ++it) {
            GenerateExpression(**it);
            output << "    push rax" << std::endl;
        }

        // Call the function
        std::string funcName = call->function->value;
        if (functions.find(funcName) == functions.end()) {
            throw std::runtime_error("Undefined function: " + funcName);
        }
        output << "    call " << funcName << std::endl;

        // Clean up arguments from the stack
        if (!call->arguments.empty()) {
            output << "    add rsp, " << call->arguments.size() * 8 << std::endl;
        }
    } else {
        throw std::runtime_error("Unknown expression type");
    }
}
