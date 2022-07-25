#ifndef __EXEC_H_INCLUDED
#define __EXEC_H_INCLUDED

#include "util.h"
#define ARG_CAPA 16
#define SIZE_BUF 1024

typedef struct Cmd Cmd;
struct Cmd
{
    char **argv;
    int argc;
    int capa;
    Cmd *next;
};

Cmd *make_cmd(void);
void free_cmd(Cmd *cmd);
void print_cmd(Cmd *cmd);
void exec_cmd(Cmd *cmd);

#endif // __EXEC_H_INCLUDED