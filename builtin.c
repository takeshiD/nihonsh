#include "builtin.h"
#include "command.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void cd(char *path)
{
    int result = chdir(path);
    if(result == -1){
        char msg[512];
        sprintf(msg, "chdir(2) invalid path: %s", path);
        perror(msg);
    }
}