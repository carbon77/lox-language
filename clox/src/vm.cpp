#include "compiler.h"
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

InterpretResult VM::interpret(std::string source)
{
    Chunk chunk;

    try {
        compile(source, &chunk);
    } catch (const CompileException& e) {
        chunk.free();
        return InterpretResult::INTERPRET_COMPILE_ERROR;
    }

    this->chunk = &chunk;
    this->ip = this->chunk->code;

    InterpretResult result = run();
    chunk.free();
    return result;
}

static inline void binary_op(VM *vm, char ch);

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
        case OpCode::OP_PLUS:
            binary_op(this, '+');
            break;
        case OpCode::OP_SUBTRACT:
            binary_op(this, '-');
            break;
        case OpCode::OP_MULTIPLY:
            binary_op(this, '*');
            break;
        case OpCode::OP_DIVIDE:
            binary_op(this, '/');
            break;
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

static inline void binary_op(VM *vm, char ch)
{
    Value b = vm->pop();
    Value a = vm->pop();

    switch (ch)
    {
    case '+':
        vm->push(a + b);
        break;
    case '-':
        vm->push(a - b);
        break;
    case '*':
        vm->push(a * b);
        break;
    case '/':
        vm->push(a / b);
        break;
    default:
        break;
    }
}