#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

#include "common.h"

enum class TokenType : uint8_t
{
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,

    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    ERROR,
    END_OF_FILE,
};

class Token
{
public:
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

class Scanner
{
public:
    Scanner(std::string source);
    Token scan_token();
    void print_tokens();

private:
    std::string source;
    size_t start;
    size_t current;
    int line;
    int column;

    bool is_at_end();
    Token make_token(TokenType type);
    Token error_token(std::string message);
    char advance();
    bool match(char expected);
    void skip_whitespaces();
    char peek();
    char peek_next();
    Token string();
    Token number();
    Token identifier();
    TokenType identifier_type();
    TokenType check_keyword(size_t start, const std::string &rest, TokenType type);
};

std::string tokenTypeToString(TokenType type);
void printToken(const Token token);

#endif