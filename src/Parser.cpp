
#include "Parser.h"

#include <unordered_map>

std::unordered_map<TokenType, Precedence> Parser::precedences = {
    {TokenType::Equal, Precedence::EQUALS},
    {TokenType::NotEqual, Precedence::EQUALS},
    {TokenType::LessThan, Precedence::LESSGREATER},
    {TokenType::GreaterThan, Precedence::LESSGREATER},
    {TokenType::Plus, Precedence::SUM},
    {TokenType::Minus, Precedence::SUM},
    {TokenType::Slash, Precedence::PRODUCT},
    {TokenType::Asterisk, Precedence::PRODUCT},
    {TokenType::LParen, Precedence::CALL},
};

Parser::Parser(Lexer& lexer, ErrorReporter& errorReporter) : lexer(lexer), errorReporter(errorReporter) {
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
    } else if (currentToken.type == TokenType::Function) {
        return ParseFunctionDeclaration();
    }
    errorReporter.AddError("Invalid statement", 0, 0);
    return nullptr;
}

std::unique_ptr<LetStatement> Parser::ParseLetStatement() {
    auto stmt = std::make_unique<LetStatement>();

    stmt->name = std::make_unique<Identifier>();
    stmt->name->value = currentToken.literal;

    NextToken(); // currentToken is now ColonAssign
    NextToken(); // currentToken is now the expression

    stmt->value = ParseExpression(LOWEST);

    if (peekToken.type == TokenType::Semicolon) {
        NextToken();
    }

    return stmt;
}

std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
    auto stmt = std::make_unique<ReturnStatement>();

    NextToken(); // Consume "return"

    stmt->returnValue = ParseExpression(LOWEST);

    if (peekToken.type == TokenType::Semicolon) {
        NextToken();
    }

    return stmt;
}

std::unique_ptr<BlockStatement> Parser::ParseBlockStatement() {
    auto block = std::make_unique<BlockStatement>();

    NextToken(); // Consume '{'

    while (currentToken.type != TokenType::RBrace && currentToken.type != TokenType::Eof) {
        auto stmt = ParseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        }
        NextToken();
    }
    // Don't call NextToken() here - the '}' will be consumed by the caller
    return block;
}

std::unique_ptr<FunctionDeclaration> Parser::ParseFunctionDeclaration() {
    auto func = std::make_unique<FunctionDeclaration>();

    NextToken(); // Consume 'fn'

    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected function name", 0, 0);
        return nullptr; // Error
    }
    func->name = std::make_unique<Identifier>();
    func->name->value = currentToken.literal;

    NextToken(); // Consume function name

    if (currentToken.type != TokenType::LParen) {
        errorReporter.AddError("Expected '(' after function name", 0, 0);
        return nullptr; // Error
    }

    // Parse parameters
    NextToken(); // Consume '('
    while (currentToken.type != TokenType::RParen && currentToken.type != TokenType::Eof) {
        if (currentToken.type != TokenType::Identifier) {
            errorReporter.AddError("Expected parameter name", 0, 0);
            return nullptr; // Error
        }
        auto param = std::make_unique<Identifier>();
        param->value = currentToken.literal;
        func->parameters.push_back(std::move(param));

        NextToken(); // move past identifier
        if (currentToken.type == TokenType::Colon) {
            NextToken(); // consume ':'
            if (currentToken.type != TokenType::Identifier) {
                errorReporter.AddError("Expected type identifier", 0, 0);
                return nullptr; // expected type identifier
            }
            NextToken(); // consume type identifier
        }
        if (currentToken.type == TokenType::Comma) {
            NextToken(); // move to next parameter or ')'
        }
    }

    if (currentToken.type != TokenType::RParen) {
        errorReporter.AddError("Expected ')' after parameters", 0, 0);
        return nullptr; // Error
    }

    NextToken(); // Consume ')'

    if (currentToken.type == TokenType::Arrow) {
        NextToken(); // Consume '->'
        if (currentToken.type != TokenType::Identifier) {
            errorReporter.AddError("Expected return type", 0, 0);
            return nullptr; // Error (return type)
        }
        func->returnType = std::make_unique<Identifier>();
        func->returnType->value = currentToken.literal;
        NextToken(); // Consume return type
    } else {
        // Optional return type: default to i32 if omitted
        func->returnType = std::make_unique<Identifier>();
        func->returnType->value = "i32";
    }

    if (currentToken.type != TokenType::LBrace) {
        errorReporter.AddError("Expected '{' before function body", 0, 0);
        return nullptr; // Error
    }

    func->body = ParseBlockStatement();
    if (!func->body) {
        return nullptr;
    }

    return func;
}

std::unique_ptr<CallExpression> Parser::ParseCallExpression(std::unique_ptr<Expression> function) {
    auto call = std::make_unique<CallExpression>();
    call->function = std::unique_ptr<Identifier>(dynamic_cast<Identifier*>(function.release()));

    NextToken(); // currentToken now '('
    NextToken(); // advance to first argument or ')'

    while (currentToken.type != TokenType::RParen && currentToken.type != TokenType::Eof) {
        auto arg = ParseExpression(LOWEST);
        if (arg) {
            call->arguments.push_back(std::move(arg));
        }
        NextToken();
        if (currentToken.type == TokenType::Comma) {
            NextToken();
        }
    }
    NextToken(); // consume ')'

    return call;
}

std::unique_ptr<Expression> Parser::ParsePrefixExpression() {
    if (currentToken.type == TokenType::Integer) {
        auto literal = std::make_unique<IntegerLiteral>();
        literal->value = std::stoll(currentToken.literal);
        return literal;
    } else if (currentToken.type == TokenType::Identifier) {
        auto ident = std::make_unique<Identifier>();
        ident->value = currentToken.literal;
        return ident;
    }
    errorReporter.AddError("Invalid prefix expression", 0, 0);
    return nullptr; // Error
}

std::unique_ptr<Expression> Parser::ParseExpression(int precedence) {
    std::unique_ptr<Expression> leftExp = ParsePrefixExpression();

    while (peekToken.type != TokenType::Semicolon && precedence < precedences[peekToken.type]) {
        // If it's a call expression
        if (peekToken.type == TokenType::LParen) {
            // Don't consume '(', ParseCallExpression will do it
            leftExp = ParseCallExpression(std::move(leftExp));
        }
        // If it's an infix expression
        else if (precedences.count(peekToken.type)) { // Check if it's an infix operator
            NextToken(); // Consume the operator
            auto infix = std::make_unique<InfixExpression>();
            infix->left = std::move(leftExp);
            infix->op = currentToken.literal;
            int currentPrecedence = precedences[currentToken.type];
            NextToken(); // Consume the right-hand side of the infix expression
            infix->right = ParseExpression(currentPrecedence);
            leftExp = std::move(infix);
        } else {
            return leftExp; // Not an infix operator or call, so return
        }
    }

    return leftExp;
}
