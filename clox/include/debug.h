#ifndef CLOX_DEBUG_H
#define CLOX_DEBUG_H

#include <iostream>
#include <iomanip>

#include "common.h"
#include "chunk.h"

class Debugger
{
private:
    int constant_instruction(std::string name, Chunk *chunk, int offset);
    int simple_instruction(std::string name, int offset);

public:
    void disassemble(Chunk *chunk, std::string name);
    int disassemble_instruction(Chunk *chunk, int offset);
};

#endif // !CLOX_DEBUG_H
