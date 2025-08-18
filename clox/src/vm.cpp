#include "compiler.h"
#include "vm.h"
#include "debug.h"

VM::VM()
{
    resetStack();
}

void VM::resetStack()
{
    stack_top = stack;
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
    Chunk _chunk;
    Compiler compiler(std::move(source), &_chunk);

    try
    {
        compiler.compile();
    }
    catch (const CompileException &e)
    {
        _chunk.free();
        return InterpretResult::INTERPRET_COMPILE_ERROR;
    }

    compiler.free();
    chunk = &_chunk;
    ip = chunk->code;

    InterpretResult result = run();
    chunk->free();
    return result;
}

static inline void binary_op(VM *vm, char ch);

InterpretResult VM::run()
{
    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "      ";
        for (Value *slot = stack; slot < stack_top; slot++)
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
            if (!peek(0).is_number())
            {
                throw std::runtime_error("Operand must be a number.");
            }

            double value = pop().get_number();
            push(Value(-value));
            break;
        }
        case OpCode::OP_RETURN:
        {
            std::cout << pop() << "\n";
            return InterpretResult::INTERPRET_OK;
        }
        case OpCode::OP_ADD:
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
        case OpCode::OP_NIL:
            push(Value::nil());
            break;
        case OpCode::OP_TRUE:
            push(Value(true));
            break;
        case OpCode::OP_FALSE:
            push(Value(false));
            break;
        }
    }
}

void VM::push(Value value)
{
    *stack_top = value;
    stack_top++;
}

Value VM::pop()
{
    stack_top--;
    return *stack_top;
}

Value VM::peek(int distance)
{
    return stack_top[-1 - distance];
}

static inline void binary_op(VM *vm, char ch)
{
    if (!vm->peek(0).is_number() || !vm->peek(1).is_number())
    {
        throw std::runtime_error("Operands must be numbers.");
    }

    double b = vm->pop().get_number();
    double a = vm->pop().get_number();

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