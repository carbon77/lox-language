#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"
#include "object.h"
#include <variant>

class Value
{
private:
    std::variant<double, bool, std::monostate, Object *> data;

public:
    enum class Type
    {
        BOOLEAN,
        NIL,
        NUMBER,
        OBJECT,
    };
    Type type;

    static Value nil();

    Value();
    Value(bool b);
    Value(double d);
    Value(Object *obj);

    bool as_boolean() const;
    double as_number() const;
    Object *as_object() const;
    StringObject *as_string_object() const;
    std::string as_string() const;

    bool is_boolean() const;
    bool is_number() const;
    bool is_nil() const;
    bool is_object() const;
    bool is_string() const;
    bool is_object_type(Object::Type type) const;
    bool is_truthy() const;

    bool equals(Value b);

    friend std::ostream &operator<<(std::ostream &os, const Value &value);
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