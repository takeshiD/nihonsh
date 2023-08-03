#include "builtin.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include "prompt.h"
static int builtin_cd(int argc, char** argv)
{
    if(argc != 2){
        std::cerr << "Error: arguments are not 2" << std::endl;
        return 1;
    }
    if(chdir(argv[1]) < 0){
        std::perror(argv[1]);
    }
    return 0;
}
static int builtin_exit(int argc, char** argv)
{
    if (argc != 1) {
        std::cerr << "Error: argument is not 1" << std::endl;
        return 1;
    }
    disable_shell_mode();
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