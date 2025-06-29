#include "vm.h"
#include "debug.h"

VM::VM()
{
}

void VM::free()
{
}

InterpretResult VM::interpret(Chunk *_chunk)
{
    chunk = _chunk;
    ip = chunk->code;
    return run();
}

InterpretResult VM::run()
{
    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
        Debugger debugger;
        debugger.disassemble_instruction(chunk, (int)(ip - chunk->code));
#endif
        uint8_t instruction = *ip++;

        switch (static_cast<OpCode>(instruction))
        {
        case OpCode::OP_CONSTANT:
        {
            Value constant = chunk->constants.values[*ip++];
            std::cout << constant << "\n";
        }
        case OpCode::OP_RETURN:
        {
            return InterpretResult::INTERPRET_OK;
        }
        }
    }
}
