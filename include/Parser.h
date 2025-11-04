#pragma once

#include <unordered_map>
#include "Lexer.h"
#include "AST.h"
#include "ErrorReporter.h"

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
    explicit Parser(Lexer& lexer, ErrorReporter& errorReporter);

    std::unique_ptr<Program> ParseProgram();

private:
    void NextToken();
    std::unique_ptr<Statement> ParseStatement();
    std::unique_ptr<LetStatement> ParseLetStatement();
    std::unique_ptr<ReturnStatement> ParseReturnStatement();
    std::unique_ptr<BlockStatement> ParseBlockStatement();
    std::unique_ptr<FunctionDeclaration> ParseFunctionDeclaration();
    std::unique_ptr<CallExpression> ParseCallExpression(std::unique_ptr<Expression> function);
    std::unique_ptr<Expression> ParsePrefixExpression();
    std::unique_ptr<Expression> ParseExpression(int precedence);

    Lexer& lexer;
    ErrorReporter& errorReporter;
    Token currentToken;
    Token peekToken;
    static std::unordered_map<TokenType, Precedence> precedences;
};
