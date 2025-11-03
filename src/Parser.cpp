
#include "Parser.h"

#include <unordered_map>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    // Initialize currentToken and peekToken
    NextToken();
    NextToken();
}

void Parser::NextToken() {
    currentToken = peekToken;
    peekToken = lexer.NextToken();
}

std::unique_ptr<Program> Parser::ParseProgram() {
    auto program = std::make_unique<Program>();

    while (currentToken.type != TokenType::Eof) {
        auto stmt = ParseStatement();
        if (stmt) {
            program->statements.push_back(std::move(stmt));
        }
        NextToken();
    }

    return program;
}

std::unique_ptr<Statement> Parser::ParseStatement() {
    if (currentToken.type == TokenType::Identifier && peekToken.type == TokenType::ColonAssign) {
        return ParseLetStatement();
    } else if (currentToken.type == TokenType::Return) {
        return ParseReturnStatement();
    }
    return nullptr;
}



std::unique_ptr<LetStatement> Parser::ParseLetStatement() {
    auto stmt = std::make_unique<LetStatement>();

    stmt->name = std::make_unique<Identifier>();
    stmt->name->value = currentToken.literal;

    NextToken(); // currentToken is now ColonAssign
    NextToken(); // currentToken is now the expression

    stmt->value = ParseExpression(0);

    if (peekToken.type == TokenType::Semicolon) {
        NextToken();
    }

    return stmt;
}

std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
    auto stmt = std::make_unique<ReturnStatement>();

    NextToken(); // Consume "return"

    stmt->returnValue = ParseExpression(0);

    if (peekToken.type == TokenType::Semicolon) {
        NextToken();
    }

    return stmt;
}

std::unordered_map<TokenType, Precedence> Parser::precedences = {
    {TokenType::Equal, Precedence::EQUALS},
    {TokenType::NotEqual, Precedence::EQUALS},
    {TokenType::LessThan, Precedence::LESSGREATER},
    {TokenType::GreaterThan, Precedence::LESSGREATER},
    {TokenType::Plus, Precedence::SUM},
    {TokenType::Minus, Precedence::SUM},
    {TokenType::Slash, Precedence::PRODUCT},
    {TokenType::Asterisk, Precedence::PRODUCT},
};

std::unique_ptr<Expression> Parser::ParseExpression(int precedence) {
    std::unique_ptr<Expression> left;
    if (currentToken.type == TokenType::Integer) {
        auto literal = std::make_unique<IntegerLiteral>();
        literal->value = std::stoll(currentToken.literal);
        left = std::move(literal);
    } else if (currentToken.type == TokenType::Identifier) {
        auto ident = std::make_unique<Identifier>();
        ident->value = currentToken.literal;
        left = std::move(ident);
    }

    while (peekToken.type != TokenType::Semicolon && precedence < precedences[peekToken.type]) {
        NextToken();
        auto infix = std::make_unique<InfixExpression>();
        infix->left = std::move(left);
        infix->op = currentToken.literal;
        int currentPrecedence = precedences[currentToken.type];
        NextToken();
        infix->right = ParseExpression(currentPrecedence);
        left = std::move(infix);
    }

    return left;
}
