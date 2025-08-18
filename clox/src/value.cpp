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

std::ostream &operator<<(std::ostream &os, const Value &value)
{
    if (value.is_boolean())
    {
        os << (value.get_boolean() ? "true" : "false");
    }
    else if (value.is_number())
    {
        os << value.get_number();
    }
    else if (value.is_nil())
    {
        os << "nil";
    }

    return os;
}
