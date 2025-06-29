#include "chunk.h"
#include "vm.h"
#include "debug.h"

int main()
{
  VM vm;
  Chunk chunk;

  int constant = chunk.add_constant(1.2);
  chunk.write((uint8_t)OpCode::OP_CONSTANT, 123);
  chunk.write(constant, 123);

  chunk.write((uint8_t)OpCode::OP_RETURN, 123);

  Debugger debugger;
  debugger.disassemble(&chunk, "test");

  vm.free();
  chunk.free();

  return 0;
}
