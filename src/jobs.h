#ifndef JOBS_H
#define JOBS_H
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "parse.h"
#include "builtin.h"

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


struct Job
{
    char* command;
    CommandList cmdlist_;
    pid_t pgid;
    struct termios tmodes;
    int stdin;
    int stdout;
    int stderr;
    bool notified;
    Job(CommandList _cmdlist): cmdlist_(_cmdlist), pgid(0), tmodes(), stdin(STDIN_FILENO),stdout(STDOUT_FILENO),stderr(STDERR_FILENO),notified(false){}
    bool is_completed();
    bool is_stopped();
};

struct JobList
{
private:
    std::vector<Job> joblist_{};
    int shell_terminal_fd_;
    pid_t shell_pgid_;
    struct sigaction ignore_;
    struct sigaction default_;
    struct sigaction notify_;
public:
    terminal_state_t term_state_;
    typedef std::vector<Job>::iterator iterator;
    typedef std::vector<Job>::const_iterator const_iterator;
    typedef std::vector<Job>::reverse_iterator reverse_iterator;
    typedef std::vector<Job>::const_reverse_iterator const_reverse_iterator;
    // forward iterator
    iterator begin(){ return this->joblist_.begin();}
    const_iterator begin() const { return this->joblist_.begin();}
    iterator end(){ return this->joblist_.end();}
    const_iterator end() const { return this->joblist_.end();}
    // reverse iterator
    reverse_iterator rbegin(){ return this->joblist_.rbegin();}
    const_reverse_iterator rbegin() const { return this->joblist_.rbegin();}
    reverse_iterator rend(){ return this->joblist_.rend();}
    const_reverse_iterator rend() const { return this->joblist_.rend();}

    JobList();
    void append_(Job job);
    // void append_(Job&& job);
    Job& at_(int idx);
    std::size_t size_() const;
    void set_sigaction();
    void wait_job_(Job& job);
    bool search_process_(pid_t pid, int status);
    bool job_is_stopped_(Job& job);
    bool job_is_completed_(Job& job);
    // void launch_job_(Job job, bool foreground);
    void launch_job_(CommandList cmdlist, bool foreground);
    void launch_process_(Command& cmd, pid_t pgid, int infile, int outfile, bool foreground);
};

#endif