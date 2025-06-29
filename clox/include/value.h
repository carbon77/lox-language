#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

typedef double Value;

class ValueArray
{
    friend class Chunk;
    friend class VM;
    friend class Debugger;

private:
    int capacity;
    int count;
    Value *values;

public:
    ValueArray();
    void write(Value value);
    void free();
};

#endif