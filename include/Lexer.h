
#pragma once

#include <string>
#include <vector>

enum class TokenType {
    // Special
    Eof,
    Illegal,

    // Identifiers and literals
    Identifier,
    Integer,
    Float,
    String,

    // Operators
    Assign,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Bang,
    LessThan,
    GreaterThan,
    Equal,
    NotEqual,
    LessThanEqual,
    GreaterThanEqual,
    Arrow,
    ColonAssign,
    PlusAssign,
    MinusAssign,
    AsteriskAssign,
    SlashAssign,
    Range,
    Ampersand,

    // Delimiters
    Comma,
    Semicolon,
    Colon,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Hash,

    // Keywords
    Function,
    If,
    Else,
    While,
    Return,
    Struct,
    Impl,
    Asm,
    Const,
    Protocol,
    Unsafe,
    Mut,
    Ref,
    Deref,
};

std::string TokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string literal;
};

class Lexer {
public:
    explicit Lexer(std::string input);
    Token NextToken();
    size_t GetPosition() const { return position; }
    const std::string& GetInput() const { return input; }

private:
    void NextChar();
    static TokenType LookupIdent(const std::string& ident);

    std::string input;
    size_t position;
    size_t readPosition;
    char ch;
};
