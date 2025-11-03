
#include "CodeGenerator.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>

int main() {
    std::string code = "return 0;";
    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.ParseProgram();
    if (!program) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    }
    CodeGenerator generator;
    std::string assembly = generator.Generate(*program);
    if (assembly.empty()) {
        std::cerr << "Generated assembly is empty" << std::endl;
        return 1;
    }
    return 0;
}
