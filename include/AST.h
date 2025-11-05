
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

// Represents a float literal
class FloatLiteral : public Expression {
public:
    double value;
    std::string ToString() const override;
};

// Represents a variable declaration (e.g., x := 10 or x: i32 = 10)
class VariableDeclaration : public Statement {
public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Identifier> type; // Optional type annotation
    std::unique_ptr<Expression> value;
    bool isConst = false;
    std::string ToString() const override;
};

// Legacy alias for compatibility
using LetStatement = VariableDeclaration;

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

// Represents an expression statement (e.g., function calls as statements)
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    std::string ToString() const override;
};

// Represents a prefix expression (e.g., -x, !x)
class PrefixExpression : public Expression {
public:
    std::string op;
    std::unique_ptr<Expression> right;
    std::string ToString() const override;
};

// Represents an if statement
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStatement> consequence;
    std::unique_ptr<BlockStatement> alternative; // Optional else block
    std::string ToString() const override;
};

// Represents a while loop
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStatement> body;
    std::string ToString() const override;
};

// Represents an assignment statement (e.g., x = 5)
class AssignmentStatement : public Statement {
public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Expression> value;
    std::string ToString() const override;
};

// Represents an unsafe block
class UnsafeStatement : public Statement {
public:
    std::unique_ptr<BlockStatement> body;
    std::string ToString() const override;
};

// Represents pointer dereference (*ptr)
class DereferenceExpression : public Expression {
public:
    std::unique_ptr<Expression> operand;
    std::string ToString() const override;
};

// Represents address-of (&var)
class AddressOfExpression : public Expression {
public:
    std::unique_ptr<Expression> operand;
    std::string ToString() const override;
};

// Represents dereferenced assignment (*ptr = value)
class DereferenceAssignmentStatement : public Statement {
public:
    std::unique_ptr<Expression> pointer;
    std::unique_ptr<Expression> value;
    std::string ToString() const override;
};

// Represents inline assembly block
class InlineAssemblyStatement : public Statement {
public:
    std::string assembly_code;
    std::string ToString() const override;
};
