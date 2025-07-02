#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"

const int STACK_MAX = 256;

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
    Value stack[STACK_MAX];
    Value *stackTop;

    void resetStack();

public:
    VM();
    void free();
    InterpretResult run();
    InterpretResult interpret(Chunk *_chunk);
    void push(Value value);
    Value pop();
};

#endif