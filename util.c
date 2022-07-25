#include "util.h"
void *xmalloc(size_t size)
{
    void *p;
    p = calloc(1, size);
    if(p == NULL){
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size)
{
    void *p;
    if(ptr == NULL){
        return xmalloc(size);
    }
    p = realloc(ptr, size);
    if(p == NULL){
        exit(EXIT_FAILURE);
    }
    return p;
}
