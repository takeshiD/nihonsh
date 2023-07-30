#include "complete.h"
#include "parse.h"
#include <string.h>
void complete(char* buf, char* cur, char* tail, size_t size)
{
    char* tmp = new char[size];
    strncpy(tmp, buf, cur-buf);
    TokenList tknlist = tokenize(tmp);
    std::cout << tknlist.at(tknlist.size()-1) << std::endl;
    delete tmp;
}