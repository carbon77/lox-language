#include "debug.h"

int Debugger::simple_instruction(std::string name, int offset)
{
  std::cout << name << "\n";
  return offset + 1;
}

int Debugger::constant_instruction(std::string name, Chunk *chunk, int offset)
{
  uint8_t constant = chunk->code[offset + 1];
  std::cout << name << " " << constant << " '" << chunk->constants.values[constant].get_number() << "'\n";
  return offset + 2;
}

void Debugger::disassemble(Chunk *chunk, std::string name)
{
  std::cout << "==" << name << "==\n";

  for (int offset = 0; offset < chunk->count;)
  {
    offset = disassemble_instruction(chunk, offset);
  }
}

int Debugger::disassemble_instruction(Chunk *chunk, int offset)
{
  std::cout << std::setfill('0') << std::setw(4) << std::hex << offset << " ";
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
  {
    std::cout << "  |";
  }
  else
  {
    std::cout << std::dec << chunk->lines[offset];
  }

  std::cout << "  ";

  uint8_t instruction = chunk->code[offset];
  switch (static_cast<OpCode>(instruction))
  {
  case OpCode::OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  case OpCode::OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", chunk, offset);
  case OpCode::OP_NEGATE:
    return simple_instruction("OP_NEGATE", offset);
  case OpCode::OP_ADD:
    return simple_instruction("OP_ADD", offset);
  case OpCode::OP_SUBTRACT:
    return simple_instruction("OP_SUBTRACT", offset);
  case OpCode::OP_MULTIPLY:
    return simple_instruction("OP_MULTIPLY", offset);
  case OpCode::OP_DIVIDE:
    return simple_instruction("OP_DIVIDE", offset);
  case OpCode::OP_NIL:
    return simple_instruction("OP_NIL", offset);
  case OpCode::OP_TRUE:
    return simple_instruction("OP_TRUE", offset);
  case OpCode::OP_FALSE:
    return simple_instruction("OP_FALSE", offset);
  default:
    std::cout << "Unknown opcode " << instruction << "\n";
    return offset + 1;
  }
}
