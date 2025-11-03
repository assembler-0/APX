
#include <iostream>
#include "Lexer.h"
#include "Parser.h"

int main(int argc, char **argv) {
    std::string code = R"(
        x := 5 + 5;
        return x;
    )";

    Lexer lexer(code);
    Parser parser(lexer);

    if (const auto program = parser.ParseProgram()) {
        std::cout << program->ToString() << std::endl;
    }

    return 0;
}
