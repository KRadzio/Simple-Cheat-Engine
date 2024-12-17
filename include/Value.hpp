#ifndef VALUE_HPP
#define VALUE_HPP

#include "unistd.h"

struct Value
{
    unsigned long address;
    long value;
    __int8_t size;
};

#endif
