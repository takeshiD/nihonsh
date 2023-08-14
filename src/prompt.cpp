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
class terminal_state_t
{
private:
    struct termios startup_mode_;
    struct termios shell_mode_;
    struct termios external_commands_mode_;
    std::string state_;
public:
    terminal_state_t(): state_("startup"){
        tcgetattr(STDIN_FILENO, &startup_mode_);
        memcpy(&external_commands_mode_, &startup_mode_, sizeof(external_commands_mode_));
        memcpy(&shell_mode_, &startup_mode_, sizeof(shell_mode_));
        /** external commands mode **/
        external_commands_mode_.c_iflag &= ~IXON;  // disable output flow control
        external_commands_mode_.c_iflag &= ~IXOFF; // disable input flow control

        /** shell mode **/
        shell_mode_.c_iflag &= ~ICRNL;   // CRをNL(\e45)に置き換えない
        shell_mode_.c_iflag &= ~INLCR;   // NL(\e45)をCRに置き換えない
        shell_mode_.c_iflag &= ~IXON;    // disable output flow control
        shell_mode_.c_iflag &= ~IXOFF;   // disable input flow control
        shell_mode_.c_lflag &= ~ICANON;  // 非カノニカルモード
        shell_mode_.c_lflag &= ~ECHO;    // 入力をエコーしない
        shell_mode_.c_iflag &= ~IEXTEN;  // 
        shell_mode_.c_cc[VMIN] = 1;
        shell_mode_.c_cc[VTIME] = 0;
    }
    ~terminal_state_t(){
        change_startup_mode_();
    }
    bool change_startup_mode_(){
        int success = tcsetattr(STDIN_FILENO, TCSANOW, &startup_mode_);
        if(success == 0){
            state_ = "startup";
            return true;
        }
        return false;
    }
    bool change_shell_mode_(){
        int success = tcsetattr(STDIN_FILENO, TCSANOW, &shell_mode_);
        if(success == 0){
            state_ = "shell";
            return true;
        }
        return false;
    }
    bool change_external_commands_mode_(){
        int success = tcsetattr(STDIN_FILENO, TCSANOW, &external_commands_mode_);
        if(success == 0){
            state_ = "external";
            return true;
        }
        return false;
    }
};

// struct termios original_attributes;

// void enable_shell_mode() {
//     struct termios shell_mode;

//     tcgetattr(STDIN_FILENO, &original_attributes);
//     shell_mode = original_attributes;
//     shell_mode.c_iflag &= ~ICRNL;   // CRをNL(\e45)に置き換えない
//     shell_mode.c_iflag &= ~INLCR;   // NL(\e45)をCRに置き換えない
//     shell_mode.c_iflag &= ~IXON;    // output flow control
//     shell_mode.c_iflag &= ~IXOFF;   // input  flow control

//     shell_mode.c_lflag &= ~ICANON;  // 非カノニカルモード
//     shell_mode.c_lflag &= ~ECHO;    // 入力をエコーしない
//     shell_mode.c_iflag &= ~IEXTEN;  // 

//     shell_mode.c_cc[VMIN] = 1;
//     shell_mode.c_cc[VTIME] = 0;
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &shell_mode);
// }
// void disable_shell_mode() {
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_attributes);
// }

static int outc(int c)
{
    write(1, &c, 1);
    return c;
}

void prompt(const char* _ps)
{
    char* termtype = getenv("TERM");
    setupterm(termtype, 1, NULL);
    terminal_state_t term_state;
    // enable_shell_mode();
    term_state.change_shell_mode_();
    char c;
    char* buf = new char[BUFSIZE];
    char* cur = buf;
    char* tail = buf;
    char cwd[1024];
    char ps[2048];
    char hostname[128];
    memset(cwd, 0, 1024);
    memset(ps, 0, 2048);
    memset(hostname, 0, 128);
    getcwd(cwd, 1024);
    gethostname(hostname, 128);
    sprintf(ps, "%s@%s:%s$ ", _ps, hostname, cwd);
    size_t length = strlen(ps);
    printf("%s", ps);
    fflush(stdout);
    // std::cout << std::hex << static_cast<int>(carriage_return[0]) << std::endl;
    while(true)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == '\x0d'){ // CR
            tputs(cursor_down, 1, outc);

            term_state.change_external_commands_mode_();
            TokenList tknlist = tokenize(buf);
            CommandList cmdlist = parse(tknlist);
            invoke_command(cmdlist);
            term_state.change_shell_mode_();

            memset(buf, 0, BUFSIZE);
            cur = buf;
            tail = buf;
            memset(cwd, 0, 1024);
            memset(ps, 0, 2048);
            getcwd(cwd, 1024);
            sprintf(ps, "%s@%s:%s$ ", _ps, hostname, cwd);
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
            // fflush(stdout);
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
                        printf("\x1b[1C");
                        cur++;
                    } 
                    if(c == 'D' && (cur > buf)){ // ←
                        printf("\x1b[1D");
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
            printf("%s", cur);
            tail++;
            cur++;
            printf("\x1b[%ldG", cur-buf+1+length);
        }
        fflush(stdout);        
    }
    term_state.change_startup_mode_();
    // disable_shell_mode();
}