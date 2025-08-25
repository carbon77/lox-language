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

typedef std::unordered_map<StringObject *, Value, StringObjectPtrHash, StringObjectPtrEqual> Table;
typedef std::unordered_set<StringObject *, StringObjectPtrHash, StringObjectPtrEqual> StringPool;

class VM
{
    friend class Chunk;

private:
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stack_top;
    Object *objects;
    StringPool string_pool;
    Table globals;

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

    uint8_t read_byte();
    Value read_constant();

    Object *allocate_object(Object::Type type);

    StringObject *take_string(std::string str);
    StringObject *allocate_string(std::string str);
};

#endif