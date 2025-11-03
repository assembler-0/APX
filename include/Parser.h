
#pragma once

#include <unordered_map>
#include "Lexer.h"
#include "AST.h"

enum Precedence {
    LOWEST,
    EQUALS,      // ==
    LESSGREATER, // > or <
    SUM,         // +
    PRODUCT,     // *
    PREFIX,      // -X or !X
    CALL,        // myFunction(X)
};

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<Program> ParseProgram();

private:
    void NextToken();
    std::unique_ptr<Statement> ParseStatement();
    std::unique_ptr<LetStatement> ParseLetStatement();
    std::unique_ptr<ReturnStatement> ParseReturnStatement();
    std::unique_ptr<Expression> ParseExpression(int precedence);

    Lexer& lexer;
    Token currentToken;
    Token peekToken;
    static std::unordered_map<TokenType, Precedence> precedences;
};
