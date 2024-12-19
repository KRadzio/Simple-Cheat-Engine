#ifndef MATCH_HPP
#define MATCH_HPP

#include <unistd.h>

enum Type
{
    EXECUTABLE = 0,
    NOT_EXECUTABLE = 1
};

struct Match
{
    unsigned long address;
    Type type;
    __int8_t size;
};


#endif
