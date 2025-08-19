#include "scanner.h"

static bool is_digit(char c)
{
    return '0' <= c && c <= '9';
}

static bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           c == '_';
}

Scanner::Scanner(std::string source) : source(source),
                                       start(0),
                                       current(0),
                                       line(1),
                                       column(0)
{
}

Token Scanner::scan_token()
{
    skip_whitespaces();
    start = current;

    if (is_at_end())
        return make_token(TokenType::END_OF_FILE);

    char c = advance();

    if (is_alpha(c))
        return identifier();

    if (is_digit(c))
        return number();

    switch (c)
    {
    case '+':
        return make_token(TokenType::PLUS);
    case '-':
        return make_token(TokenType::MINUS);
    case '*':
        return make_token(TokenType::STAR);
    case '/':
        if (peek_next() == '/')
        {
            while (peek() != '\n' && !is_at_end())
                advance();
        }
        else
        {
            return make_token(TokenType::SLASH);
        }
        break;
    case '(':
        return make_token(TokenType::LEFT_PAREN);
    case ')':
        return make_token(TokenType::RIGHT_PAREN);
    case '{':
        return make_token(TokenType::LEFT_BRACE);
    case '}':
        return make_token(TokenType::RIGHT_BRACE);
    case ';':
        return make_token(TokenType::SEMICOLON);
    case ',':
        return make_token(TokenType::COMMA);
    case '.':
        return make_token(TokenType::DOT);
    case '!':
        return make_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    case '=':
        return make_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    case '<':
        return make_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    case '>':
        return make_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    case '"':
        return string();
    }

    std::cout << "scanning error = " << c << std::endl;
    return error_token("Unexpected character.");
}

void Scanner::print_tokens()
{
    while (true)
    {
        Token token = scan_token();
        if (token.type == TokenType::END_OF_FILE)
        {
            return;
        }
        else if (token.type == TokenType::ERROR)
        {
            std::cout << "ERROR" << std::endl;
            break;
        }

        printToken(token);
    }
}

bool Scanner::is_at_end()
{
    return current >= source.length();
}

Token Scanner::make_token(TokenType type)
{
    return {
        type,
        source.substr(start, current - start),
        line,
        column};
}

Token Scanner::error_token(std::string message)
{
    return {
        TokenType::ERROR,
        message,
        line,
        column};
}

char Scanner::advance()
{
    column++;
    return source[current++];
}

bool Scanner::match(char expected)
{
    if (is_at_end())
        return false;

    if (source[current] != expected)
        return false;

    advance();
    return true;
}

void Scanner::skip_whitespaces()
{
    while (true)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            line++;
            column = 0;
            advance();
            break;
        default:
            return;
        }
    }
}

char Scanner::peek()
{
    return source[current];
}

char Scanner::peek_next()
{
    if (current + 1 >= source.length())
        return '\0';
    return source[current + 1];
}

Token Scanner::string()
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (is_at_end())
        return error_token("Unterminated string.");

    // Closing quote
    advance();
    return make_token(TokenType::STRING);
}

Token Scanner::number()
{
    while (is_digit(peek()))
        advance();

    if (peek() == '.' && is_digit(peek_next()))
    {
        advance();
        while (is_digit(peek()))
            advance();
    }

    return make_token(TokenType::NUMBER);
}

Token Scanner::identifier()
{
    while (is_alpha(peek()) || is_digit(peek()))
        advance();

    return make_token(identifier_type());
}

TokenType Scanner::identifier_type()
{
    switch (source[start])
    {
    case 'a':
        return check_keyword(1, "nd", TokenType::AND);
    case 'c':
        return check_keyword(1, "lass", TokenType::CLASS);
    case 'e':
        return check_keyword(1, "lse", TokenType::ELSE);
    case 'i':
        return check_keyword(1, "f", TokenType::IF);
    case 'n':
        return check_keyword(1, "il", TokenType::NIL);
    case 'o':
        return check_keyword(1, "r", TokenType::OR);
    case 'p':
        return check_keyword(1, "rint", TokenType::PRINT);
    case 'r':
        return check_keyword(1, "eturn", TokenType::RETURN);
    case 's':
        return check_keyword(1, "uper", TokenType::SUPER);
    case 'v':
        return check_keyword(1, "ar", TokenType::VAR);
    case 'w':
        return check_keyword(1, "hile", TokenType::WHILE);
    case 'f':
        if (current - start > 1)
        {
            switch (source[start + 1])
            {
            case 'a':
                return check_keyword(2, "lse", TokenType::FALSE);
            case 'o':
                return check_keyword(2, "r", TokenType::FOR);
            case 'u':
                return check_keyword(2, "n", TokenType::FUN);
            }
        }
        break;
    case 't':
        if (current - start > 1)
        {
            switch (source[start + 1])
            {
            case 'h':
                return check_keyword(2, "is", TokenType::THIS);
            case 'r':
                return check_keyword(2, "ue", TokenType::TRUE);
            }
        }
        break;
    }

    return TokenType::IDENTIFIER;
}

TokenType Scanner::check_keyword(size_t offset, const std::string &rest, TokenType type)
{
    size_t length = rest.length();
    if (current - start == offset + length && source.compare(start + offset, length, rest) == 0)
    {
        return type;
    }
    return TokenType::IDENTIFIER;
}

std::string tokenTypeToString(TokenType type)
{
    static const std::unordered_map<TokenType, std::string> tokenNames = {
        {TokenType::LEFT_PAREN, "LEFT_PAREN"},
        {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
        {TokenType::LEFT_BRACE, "LEFT_BRACE"},
        {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::DOT, "DOT"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::SLASH, "SLASH"},
        {TokenType::STAR, "STAR"},
        {TokenType::BANG, "BANG"},
        {TokenType::BANG_EQUAL, "BANG_EQUAL"},
        {TokenType::EQUAL, "EQUAL"},
        {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
        {TokenType::GREATER, "GREATER"},
        {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
        {TokenType::LESS, "LESS"},
        {TokenType::LESS_EQUAL, "LESS_EQUAL"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::STRING, "STRING"},
        {TokenType::NUMBER, "NUMBER"},
        {TokenType::AND, "AND"},
        {TokenType::CLASS, "CLASS"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::FOR, "FOR"},
        {TokenType::FUN, "FUN"},
        {TokenType::IF, "IF"},
        {TokenType::NIL, "NIL"},
        {TokenType::OR, "OR"},
        {TokenType::PRINT, "PRINT"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::SUPER, "SUPER"},
        {TokenType::THIS, "THIS"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::VAR, "VAR"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::ERROR, "ERROR"},
        {TokenType::END_OF_FILE, "EOF"}};

    if (auto it = tokenNames.find(type); it != tokenNames.end())
    {
        return it->second;
    }
    return "UNKNOWN_TOKEN";
}

inline void printToken(Token token)
{
    std::cout << "Token(type=" << tokenTypeToString(token.type)
              << ", lexeme=\"" << token.lexeme << "\""
              << ", line=" << token.line
              << ", column=" << token.column << ")\n";
}