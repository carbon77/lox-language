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
public:
    std::string str;

    StringObject() {}
    StringObject(const std::string &str) : str(str) {}

    bool operator==(const StringObject &other) const;
};

struct StringObjectPtrHash
{
    std::size_t operator()(const StringObject *p) const;
};

struct StringObjectPtrEqual
{
    bool operator()(const StringObject *lhs, const StringObject *rhs) const
    {
        return lhs->str == rhs->str;
    }
};

#endif