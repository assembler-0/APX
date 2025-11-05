
#include "CodeGenerator.h"
#include <iostream>
#include <stdexcept>

std::string CodeGenerator::Generate(const Program& program, const APXC_OPERATION operation) {
    output.str("");
    output.clear();

    // Add sections
    output << "section .data" << std::endl;
    
    // Generate global variables in data section
    for (const auto& stmt : program.statements) {
        if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(stmt.get())) {
            if (varDecl->isConst || !dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
                output << "    " << varDecl->name->value << ": ";
                if (varDecl->type && varDecl->type->value == "f32") {
                    output << "dd ";
                } else {
                    output << "dq ";
                }
                // For now, just put placeholder values - proper constant evaluation needed
                if (const auto* intLit = dynamic_cast<const IntegerLiteral*>(varDecl->value.get())) {
                    output << intLit->value;
                } else if (const auto* floatLit = dynamic_cast<const FloatLiteral*>(varDecl->value.get())) {
                    output << floatLit->value;
                } else {
                    output << "0"; // Default value
                }
                output << std::endl;
            }
        }
    }
    
    output << std::endl;
    output << "section .bss" << std::endl;
    output << std::endl;
    output << "section .text" << std::endl;
    output << "default rel" << std::endl;
    if (operation == APXC_OPERATION::APXC_COMPILE_W_ENTRY) {
        output << "global _start" << std::endl;
    }
    output << std::endl;

    // First pass: Register global variables and populate functions map
    for (const auto& stmt : program.statements) {
        if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(stmt.get())) {
            symbolTable.DefineGlobal(varDecl->name->value);
        } else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
            std::string funcName = funcDecl->name->value;
            functions[funcName] = funcDecl;
        }
    }

    // Second pass: Generate code for all function declarations
    for (const auto& stmt : program.statements) {
        if (dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
            GenerateStatement(*stmt);
        }
    }
    if (operation == APXC_OPERATION::APXC_COMPILE_W_ENTRY) {
        // Entry point
        output << "_start:" << std::endl;
        output << "    push rbp" << std::endl;
        output << "    mov rbp, rsp" << std::endl;

        // Call the APX main function
        if (functions.find("main") != functions.end()) {
            output << "    call main" << std::endl;
        } else {
            output << "    mov rax, 0" << std::endl; // Default return value
        }

        // Exit with the return value
        output << "    mov rdi, rax" << std::endl;
        output << "    mov rax, 60" << std::endl;
        output << "    syscall" << std::endl;
    }
    return output.str();
}

void CodeGenerator::GenerateStatement(const Statement& statement) {
    if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(&statement)) {
        symbolTable.Define(varDecl->name->value);
        GenerateExpression(*varDecl->value);
        output << "    mov [rbp" << symbolTable.Get(varDecl->name->value) << "], rax" << std::endl;
    } else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(&statement)) {
        GenerateExpression(*returnStmt->returnValue);
        output << "    leave" << std::endl;
        output << "    ret" << std::endl;
    } else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(&statement)) {
        std::string funcName = funcDecl->name->value;
        
        output << funcName << ":" << std::endl;
        output << "    push rbp" << std::endl;
        output << "    mov rbp, rsp" << std::endl;
        
        // Calculate stack space needed for local variables
        int localVarCount = 0;
        for (const auto& stmt : funcDecl->body->statements) {
            if (dynamic_cast<const VariableDeclaration*>(stmt.get())) {
                localVarCount++;
            }
        }
        if (localVarCount > 0) {
            output << "    sub rsp, " << (localVarCount * 8) << std::endl;
        }

        symbolTable.EnterScope();

        // Process parameters and add them to symbol table
        int paramOffset = 16; // rbp + 8 is return address, rbp + 16 is first argument
        for (const auto& param : funcDecl->parameters) {
            symbolTable.DefineParameter(param->value, paramOffset);
            paramOffset += 8;
        }

        bool hasReturn = false;
        for (const auto& stmt : funcDecl->body->statements) {
            GenerateStatement(*stmt);
            if (dynamic_cast<const ReturnStatement*>(stmt.get())) {
                hasReturn = true;
                break; // Don't generate code after return
            }
        }

        symbolTable.LeaveScope();

        // Add default return if no explicit return
        if (!hasReturn) {
            output << "    mov rax, 0" << std::endl;
            output << "    leave" << std::endl;
            output << "    ret" << std::endl;
        }
    } else if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&statement)) {
        GenerateExpression(*exprStmt->expression);
    } else if (const auto* ifStmt = dynamic_cast<const IfStatement*>(&statement)) {
        static int labelCounter = 0;
        int currentLabel = labelCounter++;
        
        // Generate condition
        GenerateExpression(*ifStmt->condition);
        output << "    test rax, rax" << std::endl;
        output << "    jz .else" << currentLabel << std::endl;
        
        // Generate consequence block
        for (const auto& stmt : ifStmt->consequence->statements) {
            GenerateStatement(*stmt);
        }
        output << "    jmp .endif" << currentLabel << std::endl;
        
        // Generate else block (if exists)
        output << ".else" << currentLabel << ":" << std::endl;
        if (ifStmt->alternative) {
            for (const auto& stmt : ifStmt->alternative->statements) {
                GenerateStatement(*stmt);
            }
        }
        
        output << ".endif" << currentLabel << ":" << std::endl;
    } else if (const auto* whileStmt = dynamic_cast<const WhileStatement*>(&statement)) {
        static int loopCounter = 0;
        int currentLoop = loopCounter++;
        
        output << ".loop" << currentLoop << ":" << std::endl;
        
        // Generate condition
        GenerateExpression(*whileStmt->condition);
        output << "    test rax, rax" << std::endl;
        output << "    jz .endloop" << currentLoop << std::endl;
        
        // Generate loop body
        for (const auto& stmt : whileStmt->body->statements) {
            GenerateStatement(*stmt);
        }
        
        output << "    jmp .loop" << currentLoop << std::endl;
        output << ".endloop" << currentLoop << ":" << std::endl;
    } else if (const auto* assignStmt = dynamic_cast<const AssignmentStatement*>(&statement)) {
        GenerateExpression(*assignStmt->value);
        if (symbolTable.IsGlobal(assignStmt->name->value)) {
            output << "    mov [" << assignStmt->name->value << "], rax" << std::endl;
        } else {
            int offset = symbolTable.Get(assignStmt->name->value);
            if (offset >= 0) {
                output << "    mov [rbp+" << offset << "], rax" << std::endl;
            } else {
                output << "    mov [rbp" << offset << "], rax" << std::endl;
            }
        }
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void CodeGenerator::GenerateExpression(const Expression& expression) {
    if (const auto* intLiteral = dynamic_cast<const IntegerLiteral*>(&expression)) {
        output << "    mov rax, " << intLiteral->value << std::endl;
    } else if (const auto* floatLiteral = dynamic_cast<const FloatLiteral*>(&expression)) {
        // For now, convert float to integer (proper float support needs SSE)
        output << "    mov rax, " << static_cast<int64_t>(floatLiteral->value) << std::endl;
    } else if (const auto* ident = dynamic_cast<const Identifier*>(&expression)) {
        if (symbolTable.IsGlobal(ident->value)) {
            output << "    mov rax, [" << ident->value << "]" << std::endl;
        } else {
            int offset = symbolTable.Get(ident->value);
            if (offset >= 0) {
                output << "    mov rax, [rbp+" << offset << "]" << std::endl;
            } else {
                output << "    mov rax, [rbp" << offset << "]" << std::endl;
            }
        }
    } else if (const auto* infix = dynamic_cast<const InfixExpression*>(&expression)) {
        GenerateExpression(*infix->left);
        output << "    push rax" << std::endl;
        GenerateExpression(*infix->right);
        output << "    mov rbx, rax" << std::endl;  // right operand in rbx
        output << "    pop rax" << std::endl;       // left operand in rax
        if (infix->op == "+") {
            output << "    add rax, rbx" << std::endl;
        } else if (infix->op == "-") {
            output << "    sub rax, rbx" << std::endl;
        } else if (infix->op == "*") {
            output << "    imul rax, rbx" << std::endl;
        } else if (infix->op == "/") {
            output << "    cqo" << std::endl;
            output << "    idiv rbx" << std::endl;
        } else if (infix->op == "==") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    sete al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else if (infix->op == "!=") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    setne al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else if (infix->op == "<") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    setl al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else if (infix->op == ">") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    setg al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else if (infix->op == "<=") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    setle al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else if (infix->op == ">=") {
            output << "    cmp rax, rbx" << std::endl;
            output << "    setge al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        } else {
            throw std::runtime_error("Unknown infix operator: " + infix->op);
        }
    } else if (const auto* call = dynamic_cast<const CallExpression*>(&expression)) {
        // Push arguments onto the stack in correct order (last argument first)
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
    } else if (const auto* prefix = dynamic_cast<const PrefixExpression*>(&expression)) {
        GenerateExpression(*prefix->right);
        if (prefix->op == "-") {
            output << "    neg rax" << std::endl;
        } else if (prefix->op == "!") {
            output << "    test rax, rax" << std::endl;
            output << "    setz al" << std::endl;
            output << "    movzx rax, al" << std::endl;
        }
    } else {
        throw std::runtime_error("Unknown expression type");
    }
}
