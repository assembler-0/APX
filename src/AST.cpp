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

std::string FloatLiteral::ToString() const {
    return std::to_string(value);
}

std::string VariableDeclaration::ToString() const {
    std::stringstream ss;
    if (isConst) {
        ss << "const ";
    }
    ss << name->ToString();
    if (type) {
        ss << ": " << type->ToString();
    }
    if (isConst) {
        ss << " = ";
    } else {
        ss << " := ";
    }
    ss << value->ToString() << ";";
    return ss.str();
}

std::string ReturnStatement::ToString() const {
    std::stringstream ss;
    ss << "return " << returnValue->ToString() << ";";
    return ss.str();
}

std::string BlockStatement::ToString() const {
    std::stringstream ss;
    for (const auto& stmt : statements) {
        ss << stmt->ToString();
    }
    return ss.str();
}

std::string FunctionDeclaration::ToString() const {
    std::stringstream ss;
    ss << "fn " << name->ToString() << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        ss << parameters[i]->ToString();
        if (i < parameters.size() - 1) {
            ss << ", ";
        }
    }
    ss << ") -> " << returnType->ToString() << " {" << body->ToString() << "}";
    return ss.str();
}

std::string CallExpression::ToString() const {
    std::stringstream ss;
    ss << function->ToString() << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        ss << arguments[i]->ToString();
        if (i < arguments.size() - 1) {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

std::string InfixExpression::ToString() const {
    std::stringstream ss;
    ss << "(" << left->ToString() << " " << op << " " << right->ToString() << ")";
    return ss.str();
}

std::string ExpressionStatement::ToString() const {
    return expression->ToString() + ";";
}

std::string PrefixExpression::ToString() const {
    std::stringstream ss;
    ss << "(" << op << right->ToString() << ")";
    return ss.str();
}