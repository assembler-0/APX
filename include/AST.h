
#pragma once

#include <string>
#include <vector>
#include <memory>

// Base class for all nodes in the AST
class Node {
public:
    virtual ~Node() = default;
    virtual std::string ToString() const = 0;
};

// Base class for all expressions
class Expression : public Node {};

// Base class for all statements
class Statement : public Node {};

// Represents the entire program
class Program : public Node {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    std::string ToString() const override;
};

// Represents an identifier
class Identifier : public Expression {
public:
    std::string value;
    std::string ToString() const override;
};

// Represents an integer literal
class IntegerLiteral : public Expression {
public:
    int64_t value;
    std::string ToString() const override;
};

// Represents a let statement (e.g., x := 10)
class LetStatement : public Statement {
public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Expression> value;
    std::string ToString() const override;
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> returnValue;
    std::string ToString() const override;
};

class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    std::string ToString() const override;
};

class FunctionDeclaration : public Statement {
public:
    std::unique_ptr<Identifier> name;
    std::vector<std::unique_ptr<Identifier>> parameters;
    std::unique_ptr<Identifier> returnType;
    std::unique_ptr<BlockStatement> body;
    std::string ToString() const override;
};

class CallExpression : public Expression {
public:
    std::unique_ptr<Identifier> function;
    std::vector<std::unique_ptr<Expression>> arguments;
    std::string ToString() const override;
};

class InfixExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
    std::string ToString() const override;
};
