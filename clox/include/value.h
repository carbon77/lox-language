#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"
#include <variant>

class Value
{
    enum class Type
    {
        BOOLEAN,
        NIL,
        NUMBER,
    };

public:
    Value() : data(std::monostate{}) {}
    Value(bool b) : data(b) {}
    Value(double d) : data(d) {}

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