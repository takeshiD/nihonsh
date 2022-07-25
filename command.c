#include "command.h"
#include "util.h"
#include "builtin.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

Cmd *make_cmd(void)
{
    Cmd *cmd;
    cmd = xmalloc(sizeof(Cmd));
    cmd = calloc(1, sizeof(Cmd));
    if(cmd){

    }
    cmd->argv = xmalloc(sizeof(char*)*ARG_CAPA);
    cmd->argc = 0;
    cmd->capa = ARG_CAPA;
    cmd->next = NULL;
    return cmd;
}

void free_cmd(Cmd *cmd)
{
    if(cmd->next != NULL){
        free_cmd(cmd->next);
    }
    free(cmd->argv);
    free(cmd);
}

void print_cmd(Cmd *cmd)
{
    fprintf(stdout, "[");
    for(int i=0; i < cmd->argc; i++){
        fprintf(stdout, "\"%s\"", cmd->argv[i]);
        if(i < cmd->argc-1){
            fprintf(stdout, ",");
        }
    }
    fprintf(stdout, "]");
    if(cmd->next != NULL){
        fprintf(stdout, " -> ");
        print_cmd(cmd->next);
    }
    else{
        fprintf(stdout, "\n");
    }
}

void exec_cmd(Cmd *cmd)
{
    if(0 < cmd->argc && strcmp(cmd->argv[0], "exit")==0){
        exit(EXIT_SUCCESS);
    }
    if(2 <= cmd->argc && strcmp(cmd->argv[0], "cd")==0){
        cd(cmd->argv[1]);
        return;
    }
    pid_t pid = fork();
    if(pid < 0){
        perror("fork(2) failed");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp(3) failed");
        exit(EXIT_FAILURE);
    }
    if(pid > 0){
        int status;
        pid_t ret = waitpid(pid, &status, 0);
        if(ret < 0){
            perror("waipid(2) failed");
            exit(EXIT_FAILURE);
        }
        if(WIFEXITED(status)){
            // nop
        }
        else if(WIFSIGNALED(status)){
            fprintf(stderr, "Stop Siganled\n");
        }
        else{
            fprintf(stderr, "Abnormal exit\n");
        }
        return;
    }
}