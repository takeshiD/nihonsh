#include "builtin.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prompt.h"
static int builtin_cd(int argc, char** argv)
{
    if(argc != 2){
        std::cerr << "Error: arguments are not 2" << std::endl;
        return 1;
    }
    char* path = (char*)calloc(1, strlen(argv[1]));
    memcpy(path, argv[1], strlen(argv[1]));
    if(strncmp(path, "~", 1) == 0){
        char* tmphome = getenv("HOME");
        memmove(path+strlen(tmphome), path+1, strlen(path));
        memcpy(path, tmphome, strlen(tmphome));
    }
    if(chdir(path) < 0){
        std::perror(path);
    }
    return 0;
}
static int builtin_exit(int argc, char** argv)
{
    if (argc != 1) {
        std::cerr << "Error: argument is not 1" << std::endl;
        return 1;
    }
    exit(0);
}
static int builtin_pwd(int argc, char** argv)
{
    char buf[1024];

    if (argc != 1) {
        std::cerr << "Error: argument is not 1" << std::endl;
        return 1;
    }
    if (!getcwd(buf, 1024)) {
        std::cerr << argv[0] << " cannot get working directory" << std::endl;
        return 1;
    }
    printf("%s\n", buf);
    return 0;
}
static builtin_t builtins[] = {
    {"cd",   builtin_cd},
    {"exit", builtin_exit},
    {"pwd",  builtin_pwd},
    {nullptr, nullptr}
};
builtin_t* lookup_builtin(char* cmd)
{
    size_t length = strlen(cmd);
    for(builtin_t* blt = builtins; blt->name != nullptr; blt++)
    {
        if(strncmp(blt->name, cmd, length) == 0){
            return blt;
        }
    }
    return nullptr;
}