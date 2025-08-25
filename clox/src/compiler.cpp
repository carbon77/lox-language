#include "debug.h"
#include "scanner.h"
#include "compiler.h"
#include "vm.h"
#include <iomanip>

extern VM vm;

Compiler::Compiler(std::string source, Chunk *chunk)
    : scanner(source),
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

void Compiler::error_at(Token token, std::string message)
{
    if (parser.panic_mode)
        return;
    parser.panic_mode = true;
    std::cerr << "(" << token.line << ":" << token.column << ") Error";

    if (token.type == TokenType::END_OF_FILE)
    {
        std::cerr << " at the end";
    }
    else if (token.type == TokenType::ERROR)
    {
        // Nothing
    }
    else
    {
        std::cerr << " at '" << token.lexeme << "'";
    }

    std::cerr << ": " << message << std::endl;
    parser.had_error = true;
}

void Compiler::synchronize()
{
    parser.panic_mode = false;

    while (parser.current.type != TokenType::END_OF_FILE)
    {
        if (parser.previous.type == TokenType::SEMICOLON)
            return;

        switch (parser.current.type)
        {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;

        default:
            break;
        }
    }
}

void Compiler::error_at_current(std::string message)
{
    error_at(parser.current, message);
}

void Compiler::error(std::string message)
{
    error_at(parser.previous, message);
}

void Compiler::advance()
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scanner.scan_token();
        if (parser.current.type != TokenType::ERROR)
            break;

        error_at_current(parser.current.lexeme);
    }
}

bool Compiler::check(TokenType type)
{
    return parser.current.type == type;
}

bool Compiler::match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
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

    while (!match(TokenType::END_OF_FILE))
    {
        declaration();
    }

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
    double value = std::stod(parser.previous.lexeme);
    emit_constant(Value(value));
}

void Compiler::grouping()
{
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression");
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
    case TokenType::BANG:
        emit_byte(OpCode::OP_NOT);
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
        emit_byte(OpCode::OP_ADD);
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
    case TokenType::EQUAL_EQUAL:
        emit_byte(OpCode::OP_EQUAL);
        break;
    case TokenType::BANG_EQUAL:
        emit_byte(OpCode::OP_EQUAL);
        emit_byte(OpCode::OP_NOT);
        break;
    case TokenType::GREATER:
        emit_byte(OpCode::OP_GREATER);
        break;
    case TokenType::GREATER_EQUAL:
        emit_byte(OpCode::OP_LESS);
        emit_byte(OpCode::OP_NOT);
        break;
    case TokenType::LESS:
        emit_byte(OpCode::OP_LESS);
        break;
    case TokenType::LESS_EQUAL:
        emit_byte(OpCode::OP_GREATER);
        emit_byte(OpCode::OP_NOT);
        break;
    default:
        return;
    }
}

void Compiler::literal()
{
    switch (parser.previous.type)
    {
    case TokenType::NIL:
        emit_byte(OpCode::OP_NIL);
        break;
    case TokenType::TRUE:
        emit_byte(OpCode::OP_TRUE);
        break;
    case TokenType::FALSE:
        emit_byte(OpCode::OP_FALSE);
        break;
    default:
        return;
    }
}

void Compiler::string()
{
    std::string s = parser.previous.lexeme.substr(1, parser.previous.lexeme.length() - 2);
    StringObject *obj = vm.take_string(s);
    emit_constant(Value(obj));
}

void Compiler::declaration()
{
    if (match(TokenType::VAR))
    {
        var_declaration();
    }
    else
    {
        statement();
    }

    if (parser.panic_mode)
        synchronize();
}

void Compiler::var_declaration()
{
    uint8_t global = parse_variable("Expect variable name.");

    if (match(TokenType::EQUAL))
    {
        expression();
    }
    else
    {
        emit_byte(OpCode::OP_NIL);
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    define_variable(global);
}

void Compiler::statement()
{
    if (match(TokenType::PRINT))
    {
        print_statement();
    }
    else
    {
        expression_statement();
    }
}

void Compiler::print_statement()
{
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emit_byte(OpCode::OP_PRINT);
}

void Compiler::expression_statement()
{
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emit_byte(OpCode::OP_POP);
}

void Compiler::variable()
{
    named_variable(parser.previous);
}

void Compiler::named_variable(Token name)
{
    uint8_t arg = identifier_constant(&name);
    emit_bytes({static_cast<uint8_t>(OpCode::OP_GET_GLOBAL), arg});
}

uint8_t Compiler::parse_variable(const std::string &error_message)
{
    consume(TokenType::IDENTIFIER, error_message);
    return identifier_constant(&parser.previous);
}

void Compiler::define_variable(uint8_t global)
{
    emit_bytes({static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL), global});
}

uint8_t Compiler::identifier_constant(Token *name)
{
    return make_constant(vm.take_string(name->lexeme));
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