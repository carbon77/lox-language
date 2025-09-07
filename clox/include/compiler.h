#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "common.h"
#include "chunk.h"
#include "scanner.h"
#include <initializer_list>

enum class Precedence : uint8_t
{
    NONE,
    ASSIGNMENT, // =
    OR,         // or
    AND,        // and
    EQUALITY,   // == !=
    COMPARISON, // < > <= >=
    TERM,       // + -
    FACTOR,     // * /
    UNARY,      // ! -
    CALL,       // . ()
    PRIMARY
};

class Compiler;
typedef void (Compiler::*ParseFn)(bool can_assign);

struct ParseRule
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

class Parser
{
public:
    Parser() : had_error(false),
               panic_mode(false) {};

    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
};

struct Local
{
    Token name;
    int depth;
};

struct Locals
{
    std::vector<Local> locals;
    int scope_depth;
};

class Compiler
{
public:
    Compiler(std::string source, Chunk *chunk);
    void free();
    void compile();

private:
    Parser parser;
    Scanner scanner;
    Chunk *compiling_chunk;
    bool can_assign;
    Locals *current;

    void consume(TokenType token, std::string message);
    void advance();
    bool match(TokenType type);
    bool check(TokenType type);

    void error(std::string message);
    void error_at_current(std::string message);
    void error_at(Token token, std::string message);
    void synchronize();

    Chunk *current_chunk();
    void emit_byte(uint8_t byte);
    void emit_byte(OpCode byte);
    void emit_bytes(std::initializer_list<uint8_t> bytes);
    void emit_return();
    void emit_constant(Value value);
    uint8_t make_constant(Value value);

    void parse_precedence(Precedence precedence);
    void expression();
    void number(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);
    void string(bool can_assign);

    void block();
    void begin_scope();
    void end_scope();

    void declaration();
    void var_declaration();
    void statement();
    void print_statement();
    void expression_statement();
    void variable(bool can_assign);
    void named_variable(Token name, bool can_assign);

    void add_local(Token name);
    int resolve_local(Locals *locals, Token *name);
    uint8_t parse_variable(const std::string &error_message);
    void mark_initialized();
    void define_global_variable(uint8_t global);
    void declare_local_variable();
    uint8_t identifier_constant(Token *name);

    ParseRule *get_rule(TokenType type)
    {
        return &rules[(int)type];
    };

    std::vector<ParseRule> rules = {
        /* LEFT_PAREN */ {&Compiler::grouping, nullptr, Precedence::NONE},
        /* RIGHT_PAREN */ {nullptr, nullptr, Precedence::NONE},
        /* LEFT_BRACE */ {nullptr, nullptr, Precedence::NONE},
        /* RIGHT_BRACE */ {nullptr, nullptr, Precedence::NONE},
        /* COMMA */ {nullptr, nullptr, Precedence::NONE},
        /* DOT */ {nullptr, nullptr, Precedence::NONE},
        /* MINUS */ {&Compiler::unary, &Compiler::binary, Precedence::TERM},
        /* PLUS */ {nullptr, &Compiler::binary, Precedence::TERM},
        /* SEMICOLON */ {nullptr, nullptr, Precedence::NONE},
        /* SLASH */ {nullptr, &Compiler::binary, Precedence::FACTOR},
        /* STAR */ {nullptr, &Compiler::binary, Precedence::FACTOR},
        /* BANG */ {&Compiler::unary, nullptr, Precedence::UNARY},
        /* BANG_EQUAL */ {nullptr, &Compiler::binary, Precedence::EQUALITY},
        /* EQUAL */ {nullptr, nullptr, Precedence::NONE},
        /* EQUAL_EQUAL */ {nullptr, &Compiler::binary, Precedence::EQUALITY},
        /* GREATER */ {nullptr, &Compiler::binary, Precedence::COMPARISON},
        /* GREATER_EQUAL */ {nullptr, &Compiler::binary, Precedence::COMPARISON},
        /* LESS */ {nullptr, &Compiler::binary, Precedence::COMPARISON},
        /* LESS_EQUAL */ {nullptr, &Compiler::binary, Precedence::COMPARISON},
        /* IDENTIFIER */ {&Compiler::variable, nullptr, Precedence::NONE},
        /* STRING */ {&Compiler::string, nullptr, Precedence::NONE},
        /* NUMBER */ {&Compiler::number, nullptr, Precedence::NONE},
        /* AND */ {nullptr, nullptr, Precedence::NONE},
        /* CLASS */ {nullptr, nullptr, Precedence::NONE},
        /* ELSE */ {nullptr, nullptr, Precedence::NONE},
        /* FALSE */ {&Compiler::literal, nullptr, Precedence::NONE},
        /* FOR */ {nullptr, nullptr, Precedence::NONE},
        /* FUN */ {nullptr, nullptr, Precedence::NONE},
        /* IF */ {nullptr, nullptr, Precedence::NONE},
        /* NIL */ {&Compiler::literal, nullptr, Precedence::NONE},
        /* OR */ {nullptr, nullptr, Precedence::NONE},
        /* PRINT */ {nullptr, nullptr, Precedence::NONE},
        /* RETURN */ {nullptr, nullptr, Precedence::NONE},
        /* SUPER */ {nullptr, nullptr, Precedence::NONE},
        /* THIS */ {nullptr, nullptr, Precedence::NONE},
        /* TRUE */ {&Compiler::literal, nullptr, Precedence::NONE},
        /* VAR */ {nullptr, nullptr, Precedence::NONE},
        /* WHILE */ {nullptr, nullptr, Precedence::NONE},
        /* ERROR */ {nullptr, nullptr, Precedence::NONE},
        /* END_OF_FILE */ {nullptr, nullptr, Precedence::NONE}};
};

#endif