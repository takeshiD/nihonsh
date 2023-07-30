#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <term.h>
#include "parse.h"
#include "complete.h"

#define BUFSIZE 2048
struct termios original_attributes;

void enable_shell_mode() {
    struct termios shell_mode;

    tcgetattr(STDIN_FILENO, &original_attributes);
    shell_mode = original_attributes;
    shell_mode.c_iflag &= ~ICRNL;   // CRをNL(\e45)に置き換えない
    shell_mode.c_iflag &= ~INLCR;   // NL(\e45)をCRに置き換えない
    shell_mode.c_iflag &= ~IXON;    // output flow control
    shell_mode.c_iflag &= ~IXOFF;   // input  flow control

    shell_mode.c_lflag &= ~ICANON;  // 非カノニカルモード
    shell_mode.c_lflag &= ~ECHO;    // 入力をエコーしない
    shell_mode.c_iflag &= ~IEXTEN;  // 

    shell_mode.c_cc[VMIN] = 1;
    shell_mode.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &shell_mode);
}

void disable_shell_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_attributes);
}


void prompt(const char* ps)
{
    char* termtype = getenv("TERM");
    setupterm(termtype, 1, NULL);
    enable_shell_mode();
    char c;
    char* buf = new char[BUFSIZE];
    char* cur = buf;
    char* tail = buf;
    size_t length = strlen(ps);
    printf("%s", ps);
    fflush(stdout);
    while(true)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == carriage_return[0]){
            printf("\n");
            fflush(stdout);

            TokenList tknlist = tokenize(buf);
            CommandList cmdlist = parse(tknlist);
            invoke_command(cmdlist);

            memset(buf, 0, BUFSIZE);
            cur = buf;
            tail = buf;
            printf("%s", ps);
            fflush(stdout);
            continue;
        }
        if(c == '\x7f'){ // BS, DEL
            if(cur > buf){
                cur--;
                memmove(cur, cur+1, tail-(cur));
                tail--;
                *tail = '\0';
                printf("\x1b[1D\x1b[0K");
                printf("%s", cur);
                printf("\x1b[%ldG", cur-buf+1+length);
            }
            fflush(stdout);
            continue;
        }
        if(c == '\x09'){ // TAB
            complete(buf, cur, tail, BUFSIZE);
            continue;
        }
        if(c == '\x1b'){ // ESC
            if(read(STDIN_FILENO, &c, 1) == 1 && c == '['){
                if(read(STDIN_FILENO, &c, 1) == 1){
                    if(c == 'C' && (cur < tail)){ // →
                        printf("\x1b[1C");
                        cur++;
                    } 
                    if(c == 'D' && (cur > buf)){ // ←
                        printf("\x1b[1D");
                        cur--;
                    }
                }
            }
        }else{ // 特殊文字以外
            if(cur == tail){
                *cur = c;
            }else{
                memmove(cur+1, cur, tail-cur);
                *cur = c;
            }
            printf("%s", cur);
            tail++;
            cur++;
            printf("\x1b[%ldG", cur-buf+1+length);
        }
        fflush(stdout);        
    }
    disable_shell_mode();
}