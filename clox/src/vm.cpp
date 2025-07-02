#include "vm.h"
#include "debug.h"

VM::VM()
{
    resetStack();
}

void VM::resetStack()
{
    stackTop = stack;
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
        std::cout << "      ";
        for (Value *slot = stack; slot < stackTop; slot++)
        {
            std::cout << "[ " << *slot << " ]";
        }
        std::cout << "\n";

        Debugger debugger;
        debugger.disassemble_instruction(chunk, (int)(ip - chunk->code));
#endif
        uint8_t instruction = *ip++;

        switch (static_cast<OpCode>(instruction))
        {
        case OpCode::OP_CONSTANT:
        {
            Value constant = chunk->constants.values[*ip++];
            push(constant);
            break;
        }
        case OpCode::OP_NEGATE:
        {
            push(-pop());
            break;
        }
        case OpCode::OP_RETURN:
        {
            std::cout << pop() << "\n";
            return InterpretResult::INTERPRET_OK;
        }
        }
    }
}

void VM::push(Value value)
{
    *stackTop = value;
    stackTop++;
}

Value VM::pop()
{
    stackTop--;
    return *stackTop;
}