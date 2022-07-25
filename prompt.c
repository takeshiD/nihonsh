#include "prompt.h"
#include "command.h"
#include "tokenize.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/utsname.h>

#define SIZE_COMPLETE 8
#define CWD_BUFSIZE 256

static char **splitcolon(char *p)
{
    int capa = 8;
    char **cand = calloc(1, sizeof(char*)*capa);
    int size = 0;
    while(*p){
        if(*p && *p == ':'){
            *p = '\0';
            p++;
        }
        if(*p && *p!=':' && !isspace(*p)){
            if(size >= capa){
                capa *= 2;
                cand = realloc(cand, sizeof(char*)*capa);
            }
            cand[size] = p;
            size++;
        }
        while(*p && *p!=':' && !isspace(*p)){
            p++;
        }
    }
    if(size >= capa){
        capa += 1;
        cand = realloc(cand, sizeof(char*)*capa);
    }
    cand[size] = NULL;
    return cand;
}

static char **get_bins(void)
{
    char *env, *buf;
    buf = getenv("PATH");
    env = calloc(1, sizeof(char*)*strlen(buf));
    memcpy(env, buf, strlen(buf));

    char **cand = splitcolon(env);
    char **cand_head_ptr = cand;

    DIR *d;
    struct dirent *ent;
    char **bins = calloc(1, sizeof(char*)*SIZE_COMPLETE);
    int capa = SIZE_COMPLETE;
    int size = 0;
    while(*cand){
        d = opendir(*cand);
        if(!d){
            perror("opendir(2)");
            exit(EXIT_FAILURE);
        }
        while((ent = readdir(d))){
            if(ent->d_type == DT_REG || ent->d_type == DT_LNK){
                if(size >= capa){
                    capa *= 2;
                    bins = realloc(bins, sizeof(char*)*capa);
                }
                bins[size] = ent->d_name;
                size++;
            }
        }
        closedir(d);
        cand++;
    }
    free(cand_head_ptr);
    return bins;
}

char *word_generator(const char *text, int state)
{
    static int index, wordlen;
    char *name;
    char **bins = get_bins();
    if(state == 0){
        wordlen = strlen(text);
        index = 0;
    }
    while((name = bins[index])){
        index++;
        if(!strncmp(text, name, wordlen)){
            return strdup(name);
        }
    }
    free(bins);
    return NULL;
}

char **on_complete(const char *text, int start, int end)
{
    if(start == 0){
        return rl_completion_matches(text, word_generator);
    }
    return NULL;
}

char *get_nodename(void)
{
    struct utsname uts;
    int rc = uname(&uts);
    if(rc < 0){
        perror("uname(2) failed");
        exit(EXIT_FAILURE);
    }
    char *buf = malloc(sizeof(char*)*strlen(uts.nodename)+1);
    strcpy(buf, uts.nodename);
    return buf;
}

char *get_currentdir(void)
{
    char *buf = malloc(sizeof(char*)*CWD_BUFSIZE);
    getcwd(buf, CWD_BUFSIZE);
    return buf;
}

char *get_currentuser(void)
{
    const char *temp = getenv("USER");
    char *buf;
    if(temp != NULL){
        buf = malloc(sizeof(char*)*strlen(temp)+1);
        if(buf == NULL){
            strcpy(buf, "user");
        }
        strcpy(buf, temp);
    }
    return buf;
}

void prompt()
{
    char *PS = NULL;
    char *nodename = NULL;
    char *currentdir = NULL;
    char *currentuser = NULL;
    using_history();
    read_history(NULL);
    rl_attempted_completion_function = on_complete;
    char *line;
    while(1){
        nodename = get_nodename();
        currentdir = get_currentdir();
        currentuser = get_currentuser();
        PS = malloc(sizeof(char*)*512);
        sprintf(PS, "[nihonsh]%s@%s:%s\n$ ", currentuser, nodename, currentdir);
        line = readline(PS);

        add_history(line);
        Cmd *tokens = tokenize(line);
        exec_cmd(tokens);

        free(line);
        free(tokens);
        free(nodename);
        free(currentdir);
        free(currentuser);
        free(PS);
    }
    exit(EXIT_SUCCESS);
}