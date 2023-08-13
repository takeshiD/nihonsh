#include "prompt.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
int main(int argc, char** argv)
{
    char* ps = "tkcd";
    prompt(ps);
    return 0;
}