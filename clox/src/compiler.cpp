#include "debug.h"
#include "scanner.h"
#include "compiler.h"
#include "vm.h"
#include <iomanip>

extern VM vm;
static const int MAX_LOCAL_COUNT = UINT8_MAX + 1;

Compiler::Compiler(std::string source, Chunk *chunk)
    : scanner(source),
      parser(),
      compiling_chunk(chunk)
{
    current->locals.reserve(MAX_LOCAL_COUNT);
    current->scope_depth = 0;
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

void Compiler::number(bool can_assign)
{
    double value = std::stod(parser.previous.lexeme);
    emit_constant(Value(value));
}

void Compiler::grouping(bool can_assign)
{
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary(bool can_assign)
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

void Compiler::binary(bool can_assign)
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

void Compiler::literal(bool can_assign)
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

void Compiler::string(bool can_assign)
{
    std::string s = parser.previous.lexeme.substr(1, parser.previous.lexeme.length() - 2);
    StringObject *obj = vm.take_string(s);
    emit_constant(Value(obj));
}

void Compiler::block()
{
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE))
    {
        declaration();
    }

    consume(TokenType::RIGHT_BRACE, "Exoect '}' after block.");
}

void Compiler::begin_scope()
{
    current->scope_depth++;
}

void Compiler::end_scope()
{
    current->scope_depth--;

    while (current->locals.size() > 0 && current->locals[current->locals.size() - 1].depth > current->scope_depth)
    {
        emit_byte(OpCode::OP_POP);
        current->locals.pop_back();
    }
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
    define_global_variable(global);
}

void Compiler::statement()
{
    if (match(TokenType::PRINT))
    {
        print_statement();
    }
    else if (match(TokenType::LEFT_BRACE))
    {
        begin_scope();
        block();
        end_scope();
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

void Compiler::variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}

void Compiler::named_variable(Token name, bool can_assign)
{
    OpCode getOp, setOp;
    uint8_t arg = resolve_local(current, &name);
    if (arg != -1)
    {
        getOp = OpCode::OP_GET_LOCAL;
        setOp = OpCode::OP_SET_LOCAL;
    }
    else
    {
        arg = identifier_constant(&name);
        getOp = OpCode::OP_GET_GLOBAL;
        setOp = OpCode::OP_SET_GLOBAL;
    }

    if (can_assign && match(TokenType::EQUAL))
    {
        expression();
        emit_bytes({static_cast<uint8_t>(setOp), arg});
    }
    else
    {
        emit_bytes({static_cast<uint8_t>(getOp), arg});
    }
}

void Compiler::add_local(Token name)
{
    if (current->locals.size() == MAX_LOCAL_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }

    Local local = {name, -1};
    current->locals.push_back(local);
}

uint8_t Compiler::resolve_local(Locals *locals, Token *name)
{
    for (int i = locals->locals.size() - 1; i >= 0; i--)
    {
        Local *local = &locals->locals[i];
        if (name->lexeme == local->name.lexeme)
        {
            if (local->depth == -1)
            {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

uint8_t Compiler::parse_variable(const std::string &error_message)
{
    consume(TokenType::IDENTIFIER, error_message);

    declare_local_variable();
    if (current->scope_depth > 0)
        return 0;

    return identifier_constant(&parser.previous);
}

void Compiler::define_global_variable(uint8_t global)
{
    if (current->scope_depth > 0)
    {
        mark_initialized();
        return;
    }

    emit_bytes({static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL), global});
}

void Compiler::mark_initialized()
{
    current->locals[current->locals.size() - 1].depth = current->scope_depth;
}

void Compiler::declare_local_variable()
{
    if (current->scope_depth == 0)
        return;

    Token *name = &parser.previous;
    for (int i = current->locals.size() - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth)
        {
            break;
        }

        if (name->lexeme == local->name.lexeme)
        {
            error("Already variable with this name in this scope.");
        }
    }

    add_local(*name);
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

    bool can_assign = precedence <= Precedence::ASSIGNMENT;
    (this->*prefix_rule)(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        (this->*infix_rule)(can_assign);
    }

    if (can_assign && match(TokenType::EQUAL))
    {
        error("Invalid assignment target.");
    }
}