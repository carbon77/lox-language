#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"

class Object
{
public:
    enum class Type
    {
        STRING,
    };

    Object() {}

    Type type;
    Object *next;

    bool is_string() const
    {
        return type == Type::STRING;
    }

    bool equals(Object *b);
};

class StringObject : public Object
{
    using Object::Object;

public:
    StringObject(std::string str) : str(std::move(str)) {}

    std::string str;
};

#endif