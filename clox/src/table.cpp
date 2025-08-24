#include "table.h"

bool Table::set(StringObject *key, Value *value)
{
    bool is_new_key = table.find(key) == table.end();
    table[key] = value;

    return is_new_key;
}
