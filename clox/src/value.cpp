#include "memory.h"
#include "value.h"

ValueArray::ValueArray()
{
    values = nullptr;
    capacity = 0;
    count = 0;
}

void ValueArray::write(Value value)
{
    if (capacity < count + 1)
    {
        int old_capacity = capacity;
        capacity = grow_capacity(old_capacity);
        values = grow_array<Value>(values, old_capacity, capacity);
    }

    values[count] = value;
    count++;
}

void ValueArray::free()
{
    free_array<Value>(values, capacity);
    values = nullptr;
    capacity = 0;
    count = 0;
}

Value::Value() : data(std::monostate{}), type(Type::NIL) {}
Value::Value(bool b) : data(b), type(Type::BOOLEAN) {}
Value::Value(double d) : data(d), type(Type::NUMBER) {}
Value::Value(Object *obj) : data(obj), type(Type::OBJECT) {}

Value Value::nil()
{
    return Value();
}

Object *Value::as_object() const
{
    if (auto *obj = std::get_if<Object *>(&data))
    {
        return *obj;
    }
    throw std::runtime_error("Not an object value!");
}

StringObject *Value::as_string_object() const
{
    if (!is_string())
    {
        throw std::runtime_error("Not a string value!");
    }
    return (StringObject *)as_object();
}

std::string Value::as_string() const
{
    return as_string_object()->str;
}

bool Value::as_boolean() const
{
    if (auto *b = std::get_if<bool>(&data))
    {
        return *b;
    }
    throw std::runtime_error("Not a boolean value!");
}

double Value::as_number() const
{
    if (auto *num = std::get_if<double>(&data))
    {
        return *num;
    }
    throw std::runtime_error("Not a number value!");
}

bool Value::is_boolean() const
{
    return std::holds_alternative<bool>(data);
}

bool Value::is_number() const
{
    return std::holds_alternative<double>(data);
}

bool Value::is_nil() const
{
    return std::holds_alternative<std::monostate>(data);
}

bool Value::is_object() const
{
    return std::holds_alternative<Object *>(data);
}

bool Value::is_string() const
{
    return is_object_type(Object::Type::STRING);
}

bool Value::is_object_type(Object::Type object_type) const
{
    return is_object() && as_object()->type == object_type;
}

bool Value::is_truthy() const
{
    if (is_nil())
    {
        return false;
    }
    else if (is_boolean())
    {
        return as_boolean();
    }
    else if (is_number())
    {
        // Typically, 0 is falsey and non-zero is truthy for numbers
        return as_number() != 0.0;
    }
    else if (is_object())
    {
        return as_object() != nullptr;
    }

    return false; // Should never reach here
}

bool Value::equals(Value b)
{
    if (type != b.type)
        return false;

    switch (type)
    {
    case Value::Type::BOOLEAN:
        return as_boolean() == b.as_boolean();
    case Value::Type::NIL:
        return true;
    case Value::Type::NUMBER:
        return as_number() == b.as_number();
    case Value::Type::OBJECT:
        return as_object()->equals(b.as_object());
    }
}

static void print_object(std::ostream &os, const Value &value)
{
    switch (value.as_object()->type)
    {
    case Object::Type::STRING:
        os << "\"" << value.as_string() << "\"";
        break;
    }
}

std::ostream &operator<<(std::ostream &os, const Value &value)
{
    switch (value.type)
    {
    case Value::Type::NIL:
        os << "nil";
        break;
    case Value::Type::BOOLEAN:
        os << (value.as_boolean() ? "true" : "false");
        break;
    case Value::Type::NUMBER:
        os << value.as_number();
        break;
    case Value::Type::OBJECT:
        print_object(os, value);
        break;
    }
    return os;
}
