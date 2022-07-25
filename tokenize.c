#include "tokenize.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

static bool isidentify(char p)
{
    switch(p){
        case '|': return true;
        case '>': return true;
        case '<': return true;
        default: return false;
    }
}

static bool isvalidchar(char p)
{
    return !isidentify(p) && !isspace(p) && 32<=p && p<=126;
}

Cmd *tokenize(char *p)
{
    Cmd *cmd = make_cmd();
    while(*p)
    {
        // skip white space
        while(*p && isspace(*p)){
            *p = '\0';
            p++;
        }
        if(*p && isidentify(*p)){
            break;
        }
        if(*p && isvalidchar(*p)){
            if(cmd->capa <= cmd->argc){
                cmd->capa *= 2;
                cmd->argv = xrealloc(cmd->argv, cmd->capa);
            }
            cmd->argv[cmd->argc] = p;
            cmd->argc++;
        }
        while(*p && isvalidchar(*p)){
            p++;
        }
    }
    if(cmd->capa <= cmd->argc){
        cmd->capa++;
        cmd->argv = xrealloc(cmd->argv, cmd->capa);
    }
    cmd->argv[cmd->argc] = NULL;
    if(*p == '|' || *p == '>'){
        cmd->next = tokenize(p+1);
        if(cmd->next == NULL || cmd->argc == 0){
            fprintf(stderr, "Tokenize error\n");
            exit(EXIT_FAILURE);
        }
    }
    return cmd;
}