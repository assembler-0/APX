#include "Lexer.h"
#include <unordered_map>

std::string TokenTypeToString(const TokenType type) {
    switch (type) {
        case TokenType::Eof: return "Eof";
        case TokenType::Illegal: return "Illegal";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Integer: return "Integer";
        case TokenType::Float: return "Float";
        case TokenType::String: return "String";
        case TokenType::Assign: return "Assign";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Asterisk: return "Asterisk";
        case TokenType::Slash: return "Slash";
        case TokenType::Bang: return "Bang";
        case TokenType::LessThan: return "LessThan";
        case TokenType::GreaterThan: return "GreaterThan";
        case TokenType::Equal: return "Equal";
        case TokenType::NotEqual: return "NotEqual";
        case TokenType::LessThanEqual: return "LessThanEqual";
        case TokenType::GreaterThanEqual: return "GreaterThanEqual";
        case TokenType::Arrow: return "Arrow";
        case TokenType::ColonAssign: return "ColonAssign";
        case TokenType::PlusAssign: return "PlusAssign";
        case TokenType::MinusAssign: return "MinusAssign";
        case TokenType::AsteriskAssign: return "AsteriskAssign";
        case TokenType::SlashAssign: return "SlashAssign";
        case TokenType::Range: return "Range";
        case TokenType::Ampersand: return "Ampersand";
        case TokenType::Comma: return "Comma";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Colon: return "Colon";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::Hash: return "Hash";
        case TokenType::Function: return "Function";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::Return: return "Return";
        case TokenType::Struct: return "Struct";
        case TokenType::Impl: return "Impl";
        case TokenType::Asm: return "Asm";
        case TokenType::Const: return "Const";
        case TokenType::Protocol: return "Protocol";
        case TokenType::Unsafe: return "Unsafe";
        case TokenType::Mut: return "Mut";
        case TokenType::Ref: return "Ref";
        case TokenType::Deref: return "Deref";
        default: return "Unknown";
    }
}

Lexer::Lexer(std::string input)
    : input(std::move(input)), position(0), readPosition(0), ch(0), line(1), column(1) {
    NextChar();
}

void Lexer::NextChar() {
    if (readPosition >= input.length()) {
        ch = 0;
    } else {
        ch = input[readPosition];
    }
    
    if (ch == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    
    position = readPosition;
    readPosition++;
}



TokenType Lexer::LookupIdent(const std::string& ident) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"fn", TokenType::Function},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"while", TokenType::While},
        {"return", TokenType::Return},
        {"struct", TokenType::Struct},
        {"impl", TokenType::Impl},
        {"asm", TokenType::Asm},
        {"const", TokenType::Const},
        {"protocol", TokenType::Protocol},
        {"unsafe", TokenType::Unsafe},
        {"mut", TokenType::Mut},
    };

    if (const auto it = keywords.find(ident); it != keywords.end()) {
        return it->second;
    }
    return TokenType::Identifier;
}

Token Lexer::NextToken() {
    Token tok;

    while (isspace(ch)) {
        NextChar();
    }

    switch (ch) {
    case '"': {
        std::string str;
        NextChar(); // Consume initial '"'
        while (ch != '"' && ch != 0) {
            str += ch;
            NextChar();
        }
        if (ch == '"') {
            tok = {TokenType::String, str};
        } else {
            tok = {TokenType::Illegal, str};
        }
        break;
    }
    case '=':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::Equal, "=="};
        } else {
            tok = {TokenType::Assign, "="};
        }
        break;
    case ':':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::ColonAssign, ":="};
        } else {
            tok = {TokenType::Colon, ":"};
        }
        break;
    case '+':
         if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::PlusAssign, "+="};
        } else {
            tok = {TokenType::Plus, "+"};
        }
        break;
    case '-':
        if (input[readPosition] == '>') {
            NextChar();
            tok = {TokenType::Arrow, "->"};
        } else if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::MinusAssign, "-="};
        } else {
            tok = {TokenType::Minus, "-"};
        }
        break;
    case '*':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::AsteriskAssign, "*="};
        } else {
            tok = {TokenType::Asterisk, "*"};
        }
        break;
    case '/':
        if (input[readPosition] == '/') {
            while (ch != '\n' && ch != 0) {
                NextChar();
            }
            return NextToken(); // Continue to the next token
        }
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::SlashAssign, "/="};
        } else {
            tok = {TokenType::Slash, "/"};
        }
        break;
    case '<':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::LessThanEqual, "<="};
        } else {
            tok = {TokenType::LessThan, "<"};
        }
        break;
    case '>':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::GreaterThanEqual, ">="};
        } else {
            tok = {TokenType::GreaterThan, ">"};
        }
        break;
    case '!':
        if (input[readPosition] == '=') {
            NextChar();
            tok = {TokenType::NotEqual, "!="};
        } else {
            tok = {TokenType::Bang, "!"};
        }
        break;
    case '.':
        if (input[readPosition] == '.') {
            NextChar();
            tok = {TokenType::Range, ".."};
        } else {
            tok = {TokenType::Illegal, "."};
        }
        break;
    case '(': tok = {TokenType::LParen, "("}; break;
    case ')': tok = {TokenType::RParen, ")"}; break;
    case '{': tok = {TokenType::LBrace, "{"}; break;
    case '}': tok = {TokenType::RBrace, "}"}; break;
    case '[': tok = {TokenType::LBracket, "["}; break;
    case ']': tok = {TokenType::RBracket, "]"}; break;
    case '#': tok = {TokenType::Hash, "#"}; break;
    case '&': tok = {TokenType::Ampersand, "&"}; break;
    case ',': tok = {TokenType::Comma, ","}; break;
    case ';': tok = {TokenType::Semicolon, ";"}; break;
    case 0: tok = {TokenType::Eof, ""}; break;
    default:
        if (isalpha(ch) || ch == '_') {
            std::string ident;
            while (isalnum(ch) || ch == '_') {
                ident += ch;
                NextChar();
            }
            tok.type = LookupIdent(ident);
            tok.literal = ident;
            return tok; // Early return to avoid NextChar() at the end
        }
        if (isdigit(ch) || (ch == '0' && (input[readPosition] == 'x' || input[readPosition] == 'X'))) {
            std::string num;
            bool is_float = false;
            bool is_hex = false;
            
            // Check for hex prefix
            if (ch == '0' && (input[readPosition] == 'x' || input[readPosition] == 'X')) {
                is_hex = true;
                num += ch; // '0'
                NextChar();
                num += ch; // 'x' or 'X'
                NextChar();
                
                // Parse hex digits
                while (isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                    num += ch;
                    NextChar();
                }
            } else {
                // Parse decimal number
                while (isdigit(ch) || (ch == '.' && input[readPosition] != '.')) {
                    if (ch == '.') is_float = true;
                    num += ch;
                    NextChar();
                }
            }
            
            tok.type = is_float ? TokenType::Float : TokenType::Integer;
            tok.literal = num;
            return tok; // Early return
        }
        tok = {TokenType::Illegal, std::string(1, ch)};
        break;
    }

    NextChar();
    return tok;
}