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
    std::unique_ptr<VariableDeclaration> ParseVariableDeclaration();
    std::unique_ptr<VariableDeclaration> ParseConstDeclaration();
    std::unique_ptr<ReturnStatement> ParseReturnStatement();
    std::unique_ptr<ExpressionStatement> ParseExpressionStatement();
    std::unique_ptr<BlockStatement> ParseBlockStatement();
    std::unique_ptr<FunctionDeclaration> ParseFunctionDeclaration();
    std::unique_ptr<IfStatement> ParseIfStatement();
    std::unique_ptr<WhileStatement> ParseWhileStatement();
    std::unique_ptr<AssignmentStatement> ParseAssignmentStatement();
    std::unique_ptr<UnsafeStatement> ParseUnsafeStatement();
    std::unique_ptr<DereferenceAssignmentStatement> ParseDereferenceAssignmentStatement();
    std::unique_ptr<InlineAssemblyStatement> ParseInlineAssemblyStatement();
    std::unique_ptr<CallExpression> ParseCallExpression(std::unique_ptr<Expression> function);
    std::unique_ptr<Expression> ParsePrefixExpression();
    std::unique_ptr<Expression> ParseExpression(int precedence);
    
    // Legacy alias
    std::unique_ptr<LetStatement> ParseLetStatement() { return ParseVariableDeclaration(); }

    Lexer& lexer;
    ErrorReporter& errorReporter;
    Token currentToken;
    Token peekToken;
    static std::unordered_map<TokenType, Precedence> precedences;
    
    // Helper methods
    bool ExpectPeek(TokenType type);
    bool CurrentTokenIs(TokenType type) const;
    bool PeekTokenIs(TokenType type) const;
    Precedence GetPrecedence(TokenType type) const;
};
