#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "common.h"
#include "chunk.h"

class Parser
{
public:
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
};

class Compiler
{
public:
    void compile(std::string source, Chunk *chunk);

private:
    Parser parser;
    Scanner scanner;

    void consume(TokenType token, std::string message);
    void advance();

    void error(std::string message);
    void error_at_current(std::string message);
    void error_at(Token *token, std::string message);
};

#endif