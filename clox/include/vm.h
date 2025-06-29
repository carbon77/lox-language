#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"

class VM
{
private:
    Chunk *chunk;

public:
    VM();
    void free();
};

#endif