#include "debug.h"
#include "scanner.h"
#include "compiler.h"
#include <iomanip>

Compiler::Compiler(std::string source, Chunk *chunk)
    : scanner(std::move(source)),
      parser(),
      compiling_chunk(chunk)
{
}

void Compiler::free()
{
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error)
    {
        Debugger debugger;
        debugger.disassemble(current_chunk(), "code");
    }
#endif
}

void Compiler::error_at(Token *token, std::string_view message)
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

void Compiler::error_at_current(std::string_view message)
{
    error_at(&parser.current, message);
}

void Compiler::error(std::string_view message)
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

        error_at_current(std::string_view(parser.current.start, parser.current.length));
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
    parse_precedence(Precedence::ASSIGNMENT);
}

void Compiler::number()
{
    double value = std::atof(parser.previous.start);
    emit_constant(Value(value));
}

void Compiler::grouping()
{
    expression();
    consume(TokenType::RIGHT_PAREN, "Exprect ')' after expression");
}

void Compiler::unary()
{
    TokenType operator_type = parser.previous.type;

    parse_precedence(Precedence::UNARY);

    switch (operator_type)
    {
    case TokenType::MINUS:
        emit_byte(OpCode::OP_NEGATE);
        break;
    default:
        return;
    }
}

void Compiler::binary()
{
    TokenType operator_type = parser.previous.type;

    ParseRule *rule = get_rule(operator_type);
    parse_precedence((Precedence)((int)rule->precedence + 1));

    switch (operator_type)
    {
    case TokenType::PLUS:
        emit_byte(OpCode::OP_PLUS);
        break;
    case TokenType::MINUS:
        emit_byte(OpCode::OP_SUBTRACT);
        break;
    case TokenType::STAR:
        emit_byte(OpCode::OP_MULTIPLY);
        break;
    case TokenType::SLASH:
        emit_byte(OpCode::OP_DIVIDE);
        break;
    default:
        return;
    }
}

void Compiler::parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == nullptr)
    {
        error("Expect expression");
        return;
    }

    (this->*prefix_rule)();

    while (precedence <= get_rule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        (this->*infix_rule)();
    }
}