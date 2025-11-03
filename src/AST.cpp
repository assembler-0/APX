
#include "AST.h"
#include <sstream>

std::string Program::ToString() const {
    std::stringstream ss;
    for (const auto& stmt : statements) {
        ss << stmt->ToString();
    }
    return ss.str();
}

std::string Identifier::ToString() const {
    return value;
}

std::string IntegerLiteral::ToString() const {
    return std::to_string(value);
}

std::string LetStatement::ToString() const {
    std::stringstream ss;
    ss << "let " << name->ToString() << " = " << value->ToString() << ";";
    return ss.str();
}

std::string ReturnStatement::ToString() const {
    std::stringstream ss;
    ss << "return " << returnValue->ToString() << ";";
    return ss.str();
}

std::string InfixExpression::ToString() const {
    std::stringstream ss;
    ss << "(" << left->ToString() << " " << op << " " << right->ToString() << ")";
    return ss.str();
}
