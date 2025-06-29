#include <cstdlib>
#include <iostream>

#include "chunk.h"
#include "memory.h"
#include "value.h"

Chunk::Chunk()
{
  init_chunk();
}

void Chunk::init_chunk()
{
  count = 0;
  capacity = 0;
  code = nullptr;
  lines = nullptr;
  constants = ValueArray();
}

void Chunk::write(uint8_t byte, int line)
{
  if (capacity < count + 1)
  {
    int old_capacity = capacity;
    capacity = grow_capacity(old_capacity);
    code = grow_array<uint8_t>(code, old_capacity, capacity);
    lines = grow_array<int>(lines, old_capacity, capacity);
  }

  code[count] = byte;
  lines[count] = line;
  count++;
}

void Chunk::free()
{
  free_array<uint8_t>(code, capacity);
  free_array<int>(lines, capacity);
  constants.free();
  init_chunk();
}

int Chunk::add_constant(Value value)
{
  constants.write(value);
  return constants.count - 1;
}