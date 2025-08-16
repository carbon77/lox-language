#include "scanner.h"
#include "compiler.h"
#include <iomanip>

Compiler::Compiler(std::string source, Chunk *chunk) : scanner(source)
{
    compiling_chunk = chunk;
}

Compiler::~Compiler()
{
    emit_return();
}

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

void Compiler::consume(TokenType token, std::string message)
{
    if (parser.current.type == token)
    {
        advance();
        return;
    }

    error_at_current(message);
}

void Compiler::compile()
{

    advance();
    expression();
    consume(TokenType::END_OF_FILE, "Expect end of expression.");

    if (parser.had_error)
    {
        throw CompileException();
    }
}

Chunk *Compiler::current_chunk()
{
    return compiling_chunk;
}

void Compiler::emit_byte(uint8_t byte)
{
    current_chunk()->write(byte, parser.previous.line);
}

void Compiler::emit_byte(OpCode byte)
{
    emit_byte(static_cast<uint8_t>(byte));
}

void Compiler::emit_bytes(std::initializer_list<uint8_t> bytes)
{
    for (uint8_t byte : bytes)
    {
        emit_byte(byte);
    }
}

void Compiler::emit_return()
{
    emit_byte(OpCode::OP_RETURN);
}

void Compiler::emit_constant(Value value)
{
    emit_bytes({static_cast<uint8_t>(OpCode::OP_CONSTANT), make_constant(value)});
}

uint8_t Compiler::make_constant(Value value)
{
    int constant = current_chunk()->add_constant(value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }
    return constant;
}

void Compiler::expression()
{
}

void Compiler::number()
{
    double value = std::atof(parser.previous.start);
    emit_constant(value);
}
