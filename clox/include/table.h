#ifndef CLOX_TABLE_H
#define CLOX_TABLE_H

#include <unordered_map>
#include "object.h"
#include "value.h"

class Table
{
public:
    std::unordered_map<StringObject *, Value *> table;

    bool set(StringObject *key, Value *value);
};

#endif