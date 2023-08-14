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
    tputs(ps, 1, outc);
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
    term_state.change_startup_mode_();
}