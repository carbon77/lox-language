#include "compiler.h"
#include "vm.h"
#include "debug.h"

static inline void binary_op(VM *vm, char ch);

VM::VM()
{
    reset_stack();
    objects = nullptr;

    handlers = {
        {OpCode::OP_CONSTANT, [this]()
         {
             Value constant = chunk->constants.values[*ip++];
             push(constant);
         }},
        {OpCode::OP_NEGATE, [this]()
         {
             if (!peek(0).is_number())
             {
                 throw std::runtime_error("Operand must be a number.");
             }

             double value = pop().as_number();
             push(Value(-value));
         }},
        {OpCode::OP_ADD, [this]()
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
         }},
        {OpCode::OP_SUBTRACT, [this]()
         { binary_op(this, '-'); }},
        {OpCode::OP_MULTIPLY, [this]()
         { binary_op(this, '*'); }},
        {OpCode::OP_DIVIDE, [this]()
         { binary_op(this, '/'); }},
        {OpCode::OP_GREATER, [this]()
         { binary_op(this, '>'); }},
        {OpCode::OP_LESS, [this]()
         { binary_op(this, '<'); }},
        {OpCode::OP_NIL, [this]()
         { push(Value::nil()); }},
        {OpCode::OP_TRUE, [this]()
         { push(Value(true)); }},
        {OpCode::OP_FALSE, [this]()
         { push(Value(false)); }},
        {OpCode::OP_NOT, [this]()
         { push(Value(!pop().is_truthy())); }},
        {OpCode::OP_EQUAL, [this]()
         {
             Value b = pop();
             Value a = pop();
             push(Value(a.equals(b)));
         }},
        {OpCode::OP_PRINT, [this]()
         {
             Value value = pop();
             std::cout << value << std::endl;
         }},
        {OpCode::OP_POP, [this]()
         { pop(); }},
        {OpCode::OP_DEFINE_GLOBAL, [this]()
         {
             StringObject *name = read_constant().as_string_object();
             globals[name] = peek(0);
             pop();
         }},
        {OpCode::OP_GET_GLOBAL, [this]()
         {
             StringObject *name = read_constant().as_string_object();

             if (globals.find(name) == globals.end())
             {
                 throw std::runtime_error("Undefined variable '" + name->str + "'.");
             }

             push(globals[name]);
         }},
        {OpCode::OP_SET_GLOBAL, [this]()
         {
             StringObject *name = read_constant().as_string_object();
             if (globals.find(name) == globals.end())
             {
                 throw std::runtime_error("Undefined variable '" + name->str + "'.");
             }
         }},
    };
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
        OpCode op_code = static_cast<OpCode>(instruction);
        if (op_code == OpCode::OP_RETURN)
        {
            return InterpretResult::INTERPRET_OK;
        }

        handlers[op_code]();
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

uint8_t VM::read_byte()
{
    return *ip++;
}

Value VM::read_constant()
{
    return chunk->constants.values[read_byte()];
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
    StringObject *temp_obj = new StringObject(str);
    StringObject *new_str;
    if (auto iter = string_pool.find(temp_obj); iter != string_pool.end())
    {
        new_str = *iter;
    }
    else
    {
        new_str = allocate_string(str);
    }

    delete temp_obj;
    return new_str;
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