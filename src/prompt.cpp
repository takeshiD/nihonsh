#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <term.h>

#include <iostream>
#include <signal.h>
#include "parse.h"
#include "complete.h"
#include "jobs.h"

#define BUFSIZE 2048

static int outc(int c)
{
    write(1, &c, 1);
    return c;
}

JobList joblist;

void prompt(const char* _ps)
{
    char* termtype = getenv("TERM");
    setupterm(termtype, 1, NULL);
    joblist.term_state_.change_shell_mode_();
    char c;
    char* buf = new char[BUFSIZE];
    char* cur = buf;
    char* tail = buf;
    char cwd[1024];
    char ps[2048];
    char hostname[128];
    char home[128];
    char name[128];
    memset(buf, 0, BUFSIZE);
    memset(cwd, 0, 1024);
    memset(ps, 0, 2048);
    memset(hostname, 0, 128);
    memset(home, 0, 128);
    memset(name, 0, 128);
    getcwd(cwd, 1024);
    gethostname(hostname, 128);
    struct passwd *pw = getpwuid(getuid());
    memcpy(name, pw->pw_name, sizeof(name));
    char* tmphome = getenv("HOME");
    if(tmphome == NULL){
        memcpy(home, pw->pw_dir, sizeof(home));
        setenv("HOME", home, 0);
    }else{
        memcpy(home, tmphome, strlen(tmphome));
    }
    if(strncmp(cwd, home, strlen(home)) == 0){
        memmove(cwd, cwd+strlen(home)-1, strlen(cwd));
        cwd[0] = '~';
    }
    sprintf(ps, "%s@%s:%s$ ", name, hostname, cwd);
    size_t length = strlen(ps);
    tputs(ps, 1, outc);
    while(true)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == '\x0d'){ // CR
            tputs(cursor_down, 1, outc);
            joblist.term_state_.change_external_commands_mode_();
            TokenList tknlist = tokenize(buf);
            CommandList cmdlist = parse(tknlist);
            if(cmdlist.size() > 0){
                joblist.launch_job_(cmdlist, cmdlist.foreground);
            }
            joblist.term_state_.change_shell_mode_();
            memset(buf, 0, BUFSIZE);
            cur = buf;
            tail = buf;
            memset(cwd, 0, 1024);
            memset(ps, 0, 2048);
            getcwd(cwd, 1024);
            if(strncmp(cwd, home, strlen(home)) == 0){
                memmove(cwd, cwd+strlen(home)-1, strlen(cwd));
                cwd[0] = '~';
            }
            sprintf(ps, "%s@%s:%s$ ", name, hostname, cwd);
            length = strlen(ps);
            tputs(ps, 1, outc);
            continue;
        }
        if(c == '\x7f'){ // BS, DEL
            if(cur > buf){
                cur--;
                memmove(cur, cur+1, tail-(cur));
                tail--;
                *tail = '\0';
                tputs(cursor_left, 1, outc);
                tputs(delete_character, 1, outc);
            }
            continue;
        }
        if(c == '\x09'){ // TAB
            // complete(buf, cur, tail, BUFSIZE);
            continue;
        }
        if(c == '\x1b'){ // ESC
            if(read(STDIN_FILENO, &c, 1) == 1 && c == '['){
                if(read(STDIN_FILENO, &c, 1) == 1){
                    if(c == 'C' && (cur < tail)){ // →
                        tputs(cursor_right, 1, outc);
                        cur++;
                    } 
                    if(c == 'D' && (cur > buf)){ // ←
                        tputs(cursor_left, 1, outc);
                        cur--;
                    }
                    if(c == 'A'){ // ↑
                        //
                    }
                    if(c == 'B'){ // ↓
                        //
                    } 
                }
            }
            continue;
        }
        if(32 <= c && c < 127){ // 印字可能文字
            if(cur == tail){
                *cur = c;
            }else{
                memmove(cur+1, cur, tail-cur);
                *cur = c;
            }
            tputs(cur, 1, outc); // 挿入位置から最後尾まで文字表示
            tail++;
            cur++;
            char* ystep = tiparm(column_address, cur-buf+length);
            tputs(ystep, 1, outc); // カーソルを絶対位置移動
        }
    }
    joblist.term_state_.change_startup_mode_();
}