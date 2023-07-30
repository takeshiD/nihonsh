#ifndef BUILTIN_H
#define BUILTIN_H
#include <functional>
struct builtin_t
{
    char* name;
    std::function<int(int,char**)> func;
};
builtin_t* lookup_builtin(char* cmd);
#endif