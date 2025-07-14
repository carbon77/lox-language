#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

#include "common.h"

enum class TokenType
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
    const char *start;
    int length;
    int line;
};

class Scanner
{
private:
    const char *start;
    const char *current;
    int line;

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
    TokenType check_keyword(int start, int length, const char *rest, TokenType type);

public:
    Scanner(std::string source);
    Token scan_token();
};

#endif