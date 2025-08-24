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
    Value *stack_top;
    Object *objects;

    void reset_stack();
    void free_objects();

public:
    VM();
    void free();
    InterpretResult run();
    InterpretResult interpret(Chunk *_chunk);
    InterpretResult interpret(std::string source);

    Value peek(int distance);
    void push(Value value);
    Value pop();

    Object *allocate_object(Object::Type type);
    StringObject *allocate_string(std::string str);
};

#endif