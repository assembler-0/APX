
#include "Parser.h"

#include <unordered_map>

std::unordered_map<TokenType, Precedence> Parser::precedences = {
    {TokenType::Equal, Precedence::EQUALS},
    {TokenType::NotEqual, Precedence::EQUALS},
    {TokenType::LessThan, Precedence::LESSGREATER},
    {TokenType::GreaterThan, Precedence::LESSGREATER},
    {TokenType::LessThanEqual, Precedence::LESSGREATER},
    {TokenType::GreaterThanEqual, Precedence::LESSGREATER},
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
    // Parse attributes first if present
    std::vector<std::unique_ptr<Attribute>> attributes;
    if (currentToken.type == TokenType::Hash) {
        attributes = ParseAttributes();
    }
    
    std::unique_ptr<Statement> stmt;
    switch (currentToken.type) {
        case TokenType::Const:
            stmt = ParseConstDeclaration();
            break;
        case TokenType::Function:
            stmt = ParseFunctionDeclaration();
            break;
        case TokenType::If:
            stmt = ParseIfStatement();
            break;
        case TokenType::While:
            stmt = ParseWhileStatement();
            break;
        case TokenType::Unsafe:
            stmt = ParseUnsafeStatement();
            break;
        case TokenType::Asm:
            stmt = ParseInlineAssemblyStatement();
            break;
        case TokenType::Return:
            stmt = ParseReturnStatement();
            break;
        case TokenType::Asterisk:
            // Check if this is a dereferenced assignment
            if (peekToken.type == TokenType::Identifier) {
                // Look ahead to see if there's an assignment after the identifier
                // For now, assume it's a dereferenced assignment
                stmt = ParseDereferenceAssignmentStatement();
            } else {
                stmt = ParseExpressionStatement();
            }
            break;
        case TokenType::Identifier:
            // Check for variable declaration patterns
            if (peekToken.type == TokenType::ColonAssign || peekToken.type == TokenType::Colon) {
                stmt = ParseVariableDeclaration();
            }
            // Check for assignment
            else if (peekToken.type == TokenType::Assign) {
                stmt = ParseAssignmentStatement();
            } else {
                stmt = ParseExpressionStatement();
            }
            break;
        default:
            stmt = ParseExpressionStatement();
            break;
    }
    
    // Apply attributes to the statement
    if (stmt && !attributes.empty()) {
        stmt->attributes = std::move(attributes);
        
        // Process specific attributes for variable declarations
        if (auto* varDecl = dynamic_cast<VariableDeclaration*>(stmt.get())) {
            for (const auto& attr : stmt->attributes) {
                if (attr->name == "global") {
                    varDecl->isGlobal = true;
                } else if (attr->name == "align" && !attr->arguments.empty()) {
                    varDecl->alignment = std::stoi(attr->arguments[0]);
                }
            }
        }
    }
    
    return stmt;
}

std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration() {
    auto stmt = std::make_unique<VariableDeclaration>();

    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected identifier", lexer.GetLine(), lexer.GetColumn());
        return nullptr;
    }

    stmt->name = std::make_unique<Identifier>();
    stmt->name->value = currentToken.literal;

    NextToken();

    // Handle type annotation: x: i32 = 42
    if (currentToken.type == TokenType::Colon) {
        NextToken();
        if (currentToken.type != TokenType::Identifier) {
            errorReporter.AddError("Expected type identifier", lexer.GetLine(), lexer.GetColumn());
            return nullptr;
        }
        stmt->type = std::make_unique<Identifier>();
        stmt->type->value = currentToken.literal;
        NextToken();
        
        if (currentToken.type != TokenType::Assign) {
            errorReporter.AddError("Expected '=' after type annotation", lexer.GetLine(), lexer.GetColumn());
            return nullptr;
        }
    } else if (currentToken.type == TokenType::ColonAssign) {
        // Type inference: x := 42
        // No type annotation needed
    } else {
        errorReporter.AddError("Expected ':=' or ':' in variable declaration", lexer.GetLine(), lexer.GetColumn());
        return nullptr;
    }

    NextToken(); // Move to expression
    stmt->value = ParseExpression(LOWEST);

    if (PeekTokenIs(TokenType::Semicolon)) {
        NextToken();
    }

    return stmt;
}

std::unique_ptr<VariableDeclaration> Parser::ParseConstDeclaration() {
    auto stmt = std::make_unique<VariableDeclaration>();
    stmt->isConst = true;

    NextToken(); // Consume 'const'

    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected identifier after 'const'", 0, 0);
        return nullptr;
    }

    stmt->name = std::make_unique<Identifier>();
    stmt->name->value = currentToken.literal;

    NextToken();

    if (currentToken.type != TokenType::Colon) {
        errorReporter.AddError("Expected ':' after const identifier", 0, 0);
        return nullptr;
    }

    NextToken();
    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected type identifier", 0, 0);
        return nullptr;
    }

    stmt->type = std::make_unique<Identifier>();
    stmt->type->value = currentToken.literal;

    NextToken();
    if (currentToken.type != TokenType::Assign) {
        errorReporter.AddError("Expected '=' in const declaration", 0, 0);
        return nullptr;
    }

    NextToken();
    stmt->value = ParseExpression(LOWEST);

    if (PeekTokenIs(TokenType::Semicolon)) {
        NextToken();
    }

    return stmt;
}

std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->expression = ParseExpression(LOWEST);

    if (PeekTokenIs(TokenType::Semicolon)) {
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
    
    if (currentToken.type != TokenType::RBrace) {
        errorReporter.AddError("Expected '}' to close block", 0, 0);
        return nullptr;
    }
    
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
    switch (currentToken.type) {
        case TokenType::Integer: {
            auto literal = std::make_unique<IntegerLiteral>();
            if (currentToken.literal.substr(0, 2) == "0x" || currentToken.literal.substr(0, 2) == "0X") {
                literal->value = std::stoll(currentToken.literal, nullptr, 16);
            } else {
                literal->value = std::stoll(currentToken.literal);
            }
            return literal;
        }
        case TokenType::Float: {
            auto literal = std::make_unique<FloatLiteral>();
            literal->value = std::stod(currentToken.literal);
            return literal;
        }
        case TokenType::Identifier: {
            auto ident = std::make_unique<Identifier>();
            ident->value = currentToken.literal;
            return ident;
        }
        case TokenType::Bang:
        case TokenType::Minus: {
            auto prefix = std::make_unique<PrefixExpression>();
            prefix->op = currentToken.literal;
            NextToken();
            prefix->right = ParseExpression(PREFIX);
            return prefix;
        }
        case TokenType::Asterisk: {
            auto deref = std::make_unique<DereferenceExpression>();
            NextToken();
            deref->operand = ParseExpression(PREFIX);
            return deref;
        }
        case TokenType::Ampersand: {
            auto addrOf = std::make_unique<AddressOfExpression>();
            NextToken();
            addrOf->operand = ParseExpression(PREFIX);
            return addrOf;
        }
        case TokenType::LParen: {
            NextToken();
            auto exp = ParseExpression(LOWEST);
            if (!ExpectPeek(TokenType::RParen)) {
                return nullptr;
            }
            return exp;
        }
        default:
            errorReporter.AddError("No prefix parse function for " + currentToken.literal, lexer.GetLine(), lexer.GetColumn());
            return nullptr;
    }
}

std::unique_ptr<Expression> Parser::ParseExpression(int precedence) {
    std::unique_ptr<Expression> leftExp = ParsePrefixExpression();

    while (peekToken.type != TokenType::Semicolon && precedence < GetPrecedence(peekToken.type)) {
        // If it's a call expression
        if (peekToken.type == TokenType::LParen) {
            // Don't consume '(', ParseCallExpression will do it
            leftExp = ParseCallExpression(std::move(leftExp));
        }
        // If it's an infix expression
        else if (GetPrecedence(peekToken.type) != LOWEST) { // Check if it's an infix operator
            NextToken(); // Consume the operator
            auto infix = std::make_unique<InfixExpression>();
            infix->left = std::move(leftExp);
            infix->op = currentToken.literal;
            int currentPrecedence = GetPrecedence(currentToken.type);
            NextToken(); // Consume the right-hand side of the infix expression
            infix->right = ParseExpression(currentPrecedence);
            leftExp = std::move(infix);
        } else {
            return leftExp; // Not an infix operator or call, so return
        }
    }

    return leftExp;
}

// Helper methods
bool Parser::ExpectPeek(TokenType type) {
    if (PeekTokenIs(type)) {
        NextToken();
        return true;
    }
    errorReporter.AddError("Expected " + TokenTypeToString(type) + ", got " + TokenTypeToString(peekToken.type), 0, 0);
    return false;
}

bool Parser::CurrentTokenIs(TokenType type) const {
    return currentToken.type == type;
}

bool Parser::PeekTokenIs(TokenType type) const {
    return peekToken.type == type;
}

Precedence Parser::GetPrecedence(TokenType type) const {
    auto it = precedences.find(type);
    if (it != precedences.end()) {
        return it->second;
    }
    return LOWEST;
}

std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
    auto ifStmt = std::make_unique<IfStatement>();

    NextToken(); // Consume 'if'

    // Check if parentheses are used
    bool hasParens = currentToken.type == TokenType::LParen;
    if (hasParens) {
        NextToken(); // Consume '('
    }

    ifStmt->condition = ParseExpression(LOWEST);

    if (hasParens && !ExpectPeek(TokenType::RParen)) {
        return nullptr;
    }

    if (!ExpectPeek(TokenType::LBrace)) {
        return nullptr;
    }

    ifStmt->consequence = ParseBlockStatement();
    if (!ifStmt->consequence) {
        return nullptr;
    }

    // Check for optional else clause
    if (PeekTokenIs(TokenType::Else)) {
        NextToken(); // Consume 'else'
        
        if (!ExpectPeek(TokenType::LBrace)) {
            return nullptr;
        }
        
        ifStmt->alternative = ParseBlockStatement();
        if (!ifStmt->alternative) {
            return nullptr;
        }
    }

    return ifStmt;
}

std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
    auto whileStmt = std::make_unique<WhileStatement>();

    NextToken(); // Consume 'while'

    // Check if parentheses are used
    bool hasParens = currentToken.type == TokenType::LParen;
    if (hasParens) {
        NextToken(); // Consume '('
    }

    whileStmt->condition = ParseExpression(LOWEST);

    if (hasParens && !ExpectPeek(TokenType::RParen)) {
        return nullptr;
    }

    if (!ExpectPeek(TokenType::LBrace)) {
        return nullptr;
    }

    whileStmt->body = ParseBlockStatement();
    if (!whileStmt->body) {
        return nullptr;
    }

    return whileStmt;
}

std::unique_ptr<AssignmentStatement> Parser::ParseAssignmentStatement() {
    auto assignStmt = std::make_unique<AssignmentStatement>();

    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected identifier in assignment", 0, 0);
        return nullptr;
    }

    assignStmt->name = std::make_unique<Identifier>();
    assignStmt->name->value = currentToken.literal;

    if (!ExpectPeek(TokenType::Assign)) {
        return nullptr;
    }

    NextToken(); // Move to expression
    assignStmt->value = ParseExpression(LOWEST);

    if (PeekTokenIs(TokenType::Semicolon)) {
        NextToken();
    }

    return assignStmt;
}

std::unique_ptr<UnsafeStatement> Parser::ParseUnsafeStatement() {
    auto unsafeStmt = std::make_unique<UnsafeStatement>();

    NextToken(); // Consume 'unsafe'

    if (currentToken.type != TokenType::LBrace) {
        errorReporter.AddError("Expected '{' after 'unsafe'", 0, 0);
        return nullptr;
    }

    unsafeStmt->body = ParseBlockStatement();
    if (!unsafeStmt->body) {
        return nullptr;
    }

    return unsafeStmt;
}

std::unique_ptr<DereferenceAssignmentStatement> Parser::ParseDereferenceAssignmentStatement() {
    auto derefAssign = std::make_unique<DereferenceAssignmentStatement>();

    NextToken(); // Consume '*'
    derefAssign->pointer = ParseExpression(PREFIX);

    if (!ExpectPeek(TokenType::Assign)) {
        return nullptr;
    }

    NextToken(); // Move to expression
    derefAssign->value = ParseExpression(LOWEST);

    if (PeekTokenIs(TokenType::Semicolon)) {
        NextToken();
    }

    return derefAssign;
}

std::unique_ptr<InlineAssemblyStatement> Parser::ParseInlineAssemblyStatement() {
    auto asmStmt = std::make_unique<InlineAssemblyStatement>();

    NextToken(); // Consume 'asm'

    if (currentToken.type != TokenType::LBrace) {
        errorReporter.AddError("Expected '{' after 'asm'", 0, 0);
        return nullptr;
    }

    NextToken(); // Consume '{'

    // Collect raw assembly between braces. Use '!' to terminate an instruction line.
    std::string assembly;
    bool atLineStart = true;

    while (currentToken.type != TokenType::RBrace && currentToken.type != TokenType::Eof) {
        if (currentToken.type == TokenType::Bang) {
            // '!' explicitly ends the current instruction line
            if (!assembly.empty() && assembly.back() != '\n') assembly += '\n';
            atLineStart = true;
        } else if (currentToken.type == TokenType::Comma) {
            assembly += ", ";
            atLineStart = false;
        } else {
            // Insert a space between tokens when appropriate
            if (!atLineStart && !assembly.empty()) {
                char last = assembly.back();
                if (last != ' ' && last != '\n') assembly += ' ';
            }
            assembly += currentToken.literal;
            atLineStart = false;
        }
        NextToken();
    }

    if (currentToken.type != TokenType::RBrace) {
        errorReporter.AddError("Expected '}' to close asm block", 0, 0);
        return nullptr;
    }

    asmStmt->assembly_code = assembly;
    return asmStmt;
}

std::vector<std::unique_ptr<Attribute>> Parser::ParseAttributes() {
    std::vector<std::unique_ptr<Attribute>> attributes;
    
    while (currentToken.type == TokenType::Hash) {
        auto attr = ParseAttribute();
        if (attr) {
            attributes.push_back(std::move(attr));
        }
    }
    
    return attributes;
}

std::unique_ptr<Attribute> Parser::ParseAttribute() {
    auto attr = std::make_unique<Attribute>();
    
    if (currentToken.type != TokenType::Hash) {
        errorReporter.AddError("Expected '#' at start of attribute", 0, 0);
        return nullptr;
    }
    
    NextToken(); // Consume '#'
    
    if (currentToken.type != TokenType::LBracket) {
        errorReporter.AddError("Expected '[' after '#'", 0, 0);
        return nullptr;
    }
    
    NextToken(); // Consume '['
    
    if (currentToken.type != TokenType::Identifier) {
        errorReporter.AddError("Expected attribute name", 0, 0);
        return nullptr;
    }
    
    attr->name = currentToken.literal;
    NextToken(); // Consume attribute name
    
    // Parse optional arguments
    if (currentToken.type == TokenType::LParen) {
        NextToken(); // Consume '('
        
        while (currentToken.type != TokenType::RParen && currentToken.type != TokenType::Eof) {
            if (currentToken.type == TokenType::Identifier || currentToken.type == TokenType::Integer) {
                attr->arguments.push_back(currentToken.literal);
                NextToken();
                
                if (currentToken.type == TokenType::Comma) {
                    NextToken(); // Consume ','
                }
            } else {
                errorReporter.AddError("Expected argument in attribute", 0, 0);
                return nullptr;
            }
        }
        
        if (currentToken.type != TokenType::RParen) {
            errorReporter.AddError("Expected ')' after attribute arguments", 0, 0);
            return nullptr;
        }
        
        NextToken(); // Consume ')'
    }
    
    if (currentToken.type != TokenType::RBracket) {
        errorReporter.AddError("Expected ']' to close attribute", 0, 0);
        return nullptr;
    }
    
    NextToken(); // Consume ']'
    
    return attr;
}
