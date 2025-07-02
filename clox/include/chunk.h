#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

enum class OpCode : uint8_t
{
  OP_CONSTANT,
  OP_RETURN,
  OP_NEGATE,
  OP_PLUS,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
};

class Chunk
{
  friend class Debugger;
  friend class VM;

private:
  int count;
  int capacity;
  uint8_t *code;
  int *lines;
  ValueArray constants;

  void init_chunk();

public:
  Chunk();
  void write(uint8_t byte, int line);
  void free();

  int add_constant(Value value);
};

#endif // !CLOX_CHUNK_H
