#include "scanner.h"
#include "compiler.h"
#include <iomanip>

void Compiler::error_at(Token *token, std::string message)
{
    if (parser.panic_mode)
        return;
    parser.panic_mode = true;
    std::cerr << "[line " << token->line << "] Error.";

    if (token->type == TokenType::END_OF_FILE)
    {
        std::cerr << " at the end";
    }
    else if (token->type == TokenType::ERROR)
    {
        // Nothing
    }
    else
    {
        std::fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    std::cerr << ": " << message << std::endl;
    parser.had_error = true;
}

void Compiler::error_at_current(std::string message)
{
    error_at(&parser.current, message);
}

void Compiler::error(std::string message)
{
    error_at(&parser.previous, message);
}

void Compiler::advance()
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scanner.scan_token();
        if (parser.current.type != TokenType::ERROR)
            break;

        error_at_current(parser.current.start);
    }
}

static void expression()
{
}

void Compiler::consume(TokenType token, std::string message)
{
    if (parser.current.type == token)
    {
        advance();
        return;
    }

    error_at_current(message);
}

void Compiler::compile(std::string source, Chunk *chunk)
{
    scanner = Scanner(source);
    parser = Parser();

    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    expression();
    consume(TokenType::END_OF_FILE, "Expect end of expression.");

    if (parser.had_error)
    {
        throw CompileException();
    }
}
