#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"

enum class InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

class VM
{
    friend class Chunk;

private:
    Chunk *chunk;
    uint8_t *ip;

public:
    VM();
    void free();
    InterpretResult run();
    InterpretResult interpret(Chunk *_chunk);
};

#endif