#include "compiler.h"
#include "vm.h"
#include "debug.h"

VM::VM()
{
    reset_stack();
    objects = nullptr;
}

void VM::reset_stack()
{
    stack_top = stack;
}

void VM::free()
{
    free_objects();
}

void VM::free_objects()
{
    Object *obj = objects;
    while (obj != nullptr)
    {
        Object *next = obj->next;
        delete obj;
        obj = next;
    }
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
    Compiler compiler(source, &_chunk);

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

            double value = pop().as_number();
            push(Value(-value));
            break;
        }
        case OpCode::OP_RETURN:
        {
            std::cout << pop() << "\n";
            return InterpretResult::INTERPRET_OK;
        }
        case OpCode::OP_ADD:
        {
            if (peek(0).is_string() && peek(1).is_string())
            {
                std::string b = pop().as_string();
                std::string a = pop().as_string();
                push(allocate_string(a + b));
            }
            else if (peek(0).is_number() && peek(1).is_number())
            {
                double b = pop().as_number();
                double a = pop().as_number();
                push(Value(a + b));
            }
            else
            {
                throw std::runtime_error("Unsupported types for '+' operation");
            }
            break;
        }
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
        case OpCode::OP_NOT:
            push(Value(!pop().is_truthy()));
            break;
        case OpCode::OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(Value(a.equals(b)));
            break;
        }
        case OpCode::OP_GREATER:
            binary_op(this, '>');
            break;
        case OpCode::OP_LESS:
            binary_op(this, '<');
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

Object *VM::allocate_object(Object::Type type)
{
    Object *obj;
    switch (type)
    {
    case Object::Type::STRING:
        obj = new StringObject();
    }

    obj->next = objects;
    objects = obj;
    return obj;
}

StringObject *VM::take_string(std::string str)
{
    StringObject *obj = new StringObject(str);
    StringObject *s;
    if (auto iter = string_pool.find(obj); iter != string_pool.end())
    {
        s = *iter;
    }
    s = allocate_string(str);

    delete obj;
    return s;
}

StringObject *VM::allocate_string(std::string str)
{
    StringObject *obj = (StringObject *)allocate_object(Object::Type::STRING);
    obj->str = str;

    string_pool.insert(obj);
    return obj;
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

    double b = vm->pop().as_number();
    double a = vm->pop().as_number();

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
    case '<':
        vm->push(a < b);
        break;
    case '>':
        vm->push(a > b);
        break;
    default:
        break;
    }
}