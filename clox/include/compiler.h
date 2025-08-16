#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "common.h"
#include "chunk.h"
#include "scanner.h"
#include <initializer_list>

enum class Precendence : uint8_t
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

class Parser
{
public:
    Parser()
    {
        had_error = false;
        panic_mode = false;
    };

    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
};

class Compiler
{
public:
    Compiler(std::string source, Chunk *chunk);
    ~Compiler();
    void compile();

private:
    Parser parser;
    Scanner scanner;
    Chunk *compiling_chunk;

    void consume(TokenType token, std::string message);
    void advance();

    void error(std::string message);
    void error_at_current(std::string message);
    void error_at(Token *token, std::string message);

    Chunk *current_chunk();
    void emit_byte(uint8_t byte);
    void emit_byte(OpCode byte);
    void emit_bytes(std::initializer_list<uint8_t> bytes);
    void emit_return();
    void emit_constant(Value value);
    uint8_t make_constant(Value value);

    void parse_precedence(Precendence precedence);
    void expression();
    void number();
    void grouping();
    void unary();
};

#endif