#include "chunk.h"
#include "debug.h"

int main()
{
  Chunk chunk;

  int constant = chunk.add_constant(1.2);
  chunk.write((uint8_t)OpCode::OP_CONSTANT, 123);
  chunk.write(constant, 123);

  chunk.write((uint8_t)OpCode::OP_RETURN, 123);

  Debugger debugger;
  debugger.disassemble(&chunk, "test");

  return 0;
}
