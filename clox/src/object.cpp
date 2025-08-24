#include "object.h"

bool Object::equals(Object *b)
{
    if (type != b->type)
    {
        return false;
    }

    switch (type)
    {
    case Object::Type::STRING:
        std::string a_string = ((StringObject *)this)->str;
        std::string b_string = ((StringObject *)b)->str;
        return a_string == b_string;
    }
}

bool StringObject::operator==(const StringObject &other) const
{
    return str == other.str;
}

std::size_t StringObjectPtrHash::operator()(const StringObject *p) const
{
    return std::hash<std::string>{}(p->str);
}
