#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"
#include <variant>

class Value
{
public:
    enum class Type
    {
        BOOLEAN,
        NIL,
        NUMBER,
    };
    Type type;

    Value() : data(std::monostate{}), type(Type::NIL) {}
    Value(bool b) : data(b), type(Type::BOOLEAN) {}
    Value(double d) : data(d), type(Type::NUMBER) {}

    static Value nil() { return Value(); }

    friend std::ostream &operator<<(std::ostream &os, const Value &value);

    bool get_boolean() const
    {
        if (auto *b = std::get_if<bool>(&data))
        {
            return *b;
        }
        throw std::runtime_error("Not a boolean value!");
    }

    double get_number() const
    {
        if (auto *b = std::get_if<double>(&data))
        {
            return *b;
        }
        throw std::runtime_error("Not a double value!");
    }

    bool is_boolean() const
    {
        return std::holds_alternative<bool>(data);
    }

    bool is_number() const
    {
        return std::holds_alternative<double>(data);
    }

    bool is_nil() const
    {
        return std::holds_alternative<std::monostate>(data);
    }

    bool is_truthy() const
    {
        if (is_number())
        {
            return get_number() == 0;
        }
        else if (is_nil())
        {
            return false;
        }

        return get_boolean();
    }

private:
    std::variant<double, bool, std::monostate> data;
};

class ValueArray
{
    friend class Chunk;
    friend class VM;
    friend class Debugger;

private:
    int capacity;
    int count;
    Value *values;

public:
    ValueArray();
    void write(Value value);
    void free();
};

#endif