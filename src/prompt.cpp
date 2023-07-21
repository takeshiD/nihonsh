#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "parse.h"

#define BUFSIZE 2048
struct termios original_attributes;

void enable_raw_mode() {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &original_attributes);
    raw = original_attributes;
    raw.c_lflag &= ~(ECHO | ICANON);
    // raw.c_lflag |= (ECHOE | ECHOK | ECHOKE | ECHONL);
    // raw.c_lflag |= (ECHOE | ECHOK | ECHONL);
    raw.c_iflag |= IEXTEN;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_attributes);
}


void prompt(const char* ps)
{
    enable_raw_mode();
    std::cout << original_attributes.c_lflag << std::endl;
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
        if(c == '\x0a'){ // LF
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
        if(c == 127){ // BS
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
    disable_raw_mode();
}