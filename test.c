#include "command.h"
#include "tokenize.h"
#include "prompt.h"
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_exec_cmd()
{
    printf("#test_exec_cmd: ");
    fflush(stdout);
    Cmd *cmd = make_cmd();
    cmd->argv[0] = "echo";
    cmd->argv[1] = "success";
    cmd->argv[2] = NULL;
    cmd->argc = 2;
    exec_cmd(cmd);
}

void test_tokenize()
{
    printf("#test_tokenize: ");
    char *p1 = malloc(sizeof(char)*32);
    strcpy(p1, "ls -la");
    Cmd *cmd1 = tokenize(p1);
    assert(cmd1->argc == 2);
    assert(strcmp(cmd1->argv[0], "ls")==0);
    assert(strcmp(cmd1->argv[1], "-la")==0);
    assert(cmd1->argv[2] == NULL);
    assert(cmd1->next == NULL);

    char *p2 = malloc(sizeof(char)*32);
    strcpy(p2, "ls -la | head");
    Cmd *cmd2 = tokenize(p2);
    assert(cmd2->argc == 2);
    assert(strcmp(cmd2->argv[0], "ls")==0);
    assert(strcmp(cmd2->argv[1], "-la")==0);
    assert(cmd2->argv[2] == NULL);
    assert(cmd2->next != NULL);
    assert(cmd2->next->argc==1);
    assert(strcmp(cmd2->next->argv[0], "head")==0);
    assert(cmd2->next->next == NULL);
    printf("success\n");
}

void test_prompt()
{
    prompt();
}

int main()
{
    // test_exec_cmd();
    // test_tokenize();
    test_prompt();
    return 0;
}