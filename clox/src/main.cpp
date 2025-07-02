#include "chunk.h"
#include "vm.h"
#include "debug.h"

int main()
{
  VM vm;
  Chunk chunk;

  int constant = chunk.add_constant(1.2);
  int constant2 = chunk.add_constant(2.4);
  chunk.write((uint8_t)OpCode::OP_CONSTANT, 123);
  chunk.write(constant, 123);

  chunk.write((uint8_t)OpCode::OP_CONSTANT, 124);
  chunk.write(constant2, 124);

  chunk.write((uint8_t)OpCode::OP_PLUS, 125);
  chunk.write((uint8_t)OpCode::OP_RETURN, 126);

  // Debugger debugger;
  // debugger.disassemble(&chunk, "debug");
  // std::cout << "======\n";

  vm.interpret(&chunk);

  vm.free();
  chunk.free();

  return 0;
}
