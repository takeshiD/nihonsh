#include "jobs.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <algorithm>
#include <vector>
#include <iomanip>


extern JobList joblist;

void do_job_notification(int signum)
{
    std::vector<int> remove_elements;
    for(int i=0; i<joblist.size_(); i++)
    {
        if(joblist.at_(i).is_completed())
        {
            // std::cout << "[" << joblist.at_(i).pgid << "] ";
            // std::cout << std::left << std::setw(13) << "completed";
            // std::cout << joblist.at_(i).cmdlist_;
            // std::cout << std::endl;
            remove_elements.push_back(i);
        }
        else if(!joblist.at_(i).notified && joblist.at_(i).is_stopped())
        {
            // std::cout << "[" << joblist.at_(i).pgid << "] ";
            // std::cout << std::left << std::setw(13) << "stopped";
            // std::cout << joblist.at_(i).cmdlist_;
            // std::cout << std::endl;
            joblist.at_(i).notified = true;
        }
        else{
            // running: 何も表示しない
            // std::cout << "[" << joblist.at_(i).pgid << "] ";
            // std::cout << std::left << std::setw(13) << "running";
            // std::cout << joblist.at_(i).cmdlist_;
            // std::cout << std::endl;
        }
    }
    for(int i: remove_elements)
    {
        joblist.erase_(std::cbegin(joblist) + i);
    }
}

bool Job::is_completed()
{
    for(Command& cmd: cmdlist_)
    {
        if(!cmd.completed){ return false;}
    }
    return true;
}

bool Job::is_stopped()
{
    for(Command& cmd: cmdlist_)
    {
        if(!cmd.completed && !cmd.stopped){ return false;}
    }
    return true;
}

JobList::JobList(): shell_terminal_fd_(STDIN_FILENO), shell_pgid_(0)
{
    while(tcgetpgrp(shell_terminal_fd_) != (shell_pgid_ = getpgrp()))
    {
        kill(-shell_pgid_, SIGTTIN);
    }
    sigemptyset(&ignore_.sa_mask);
    ignore_.sa_flags |= SA_RESTART;
    ignore_.sa_handler = SIG_IGN;
    sigemptyset(&default_.sa_mask);
    default_.sa_flags |= SA_RESTART;
    default_.sa_handler = SIG_DFL;
    sigemptyset(&notify_.sa_mask);
    notify_.sa_flags |= SA_RESTART;
    notify_.sa_handler = do_job_notification;
    sigaction(SIGINT,  &ignore_, NULL);
    sigaction(SIGQUIT, &ignore_, NULL);
    sigaction(SIGTSTP, &ignore_, NULL);
    sigaction(SIGTTIN, &ignore_, NULL);
    sigaction(SIGTTOU, &ignore_, NULL);
    sigaction(SIGCHLD, &notify_, NULL);
    shell_pgid_ = getpid();
    setpgid(shell_pgid_, shell_pgid_);
    tcsetpgrp(shell_terminal_fd_, shell_pgid_);
    term_state_.change_shell_mode_();
}

void JobList::set_sigaction()
{
    sigaction(SIGINT,  &ignore_, NULL);
    sigaction(SIGQUIT, &ignore_, NULL);
    sigaction(SIGTSTP, &ignore_, NULL);
    sigaction(SIGTTIN, &ignore_, NULL);
    sigaction(SIGTTOU, &ignore_, NULL);
    sigaction(SIGCHLD, &notify_, NULL);
}

void JobList::append_(Job job)
{
    joblist_.emplace_back(job);
}
Job& JobList::at_(int idx)
{
    return joblist_.at(idx);
}
std::size_t JobList::size_() const
{
    return joblist_.size();
}

JobList::iterator JobList::erase_(JobList::const_iterator position)
{
    return joblist_.erase(position);
}

void JobList::launch_process_(Command& cmd, pid_t pgid, int infile, int outfile, bool foreground)
{
    pid_t pid = getpid();
    if(pgid == 0){ pgid = pid;}
    setpgid(pid, pgid);
    if(foreground){
        tcsetpgrp(shell_terminal_fd_, pgid);
    }
    sigaction(SIGINT,  &default_, NULL);
    sigaction(SIGQUIT, &default_, NULL);
    sigaction(SIGTSTP, &default_, NULL);
    sigaction(SIGTTIN, &default_, NULL);
    sigaction(SIGTTOU, &default_, NULL);
    sigaction(SIGCHLD, &default_, NULL);
    if(infile != STDIN_FILENO){
        dup2(infile, STDIN_FILENO);
        close(infile);
    }
    if(outfile != STDOUT_FILENO){
        dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }
    // std::cerr << "execv: " << cmd.argv[0] << ", in:" << infile << ", out:" << outfile << std::endl;
    execvp(cmd.argv[0], cmd.argv.data());
    perror("execvp");
    exit(1);
}

void JobList::launch_job_(CommandList cmdlist, bool foreground)
{
    Job job(cmdlist);
    int fds[2] = {-1, -1};
    int infile, outfile;
    infile = job.stdin;
    for(int i=0; i < job.cmdlist_.size(); i++)
    {
        builtin_t* blt = lookup_builtin(job.cmdlist_.at(i).argv[0]);
        if(blt != nullptr){
            blt->func(job.cmdlist_.at(i).argc, job.cmdlist_.at(i).argv.data());
            job.cmdlist_.at(i).completed = true;
            continue;
        }
        if(job.cmdlist_.is_redirect_out(job.cmdlist_.at(i))){
            continue;
        }
        if(job.cmdlist_.is_redirect_in(job.cmdlist_.at(i))){
            continue;
        }
        if(!job.cmdlist_.is_tail(job.cmdlist_.at(i))){
            pipe(fds);
            outfile = fds[1];
        }else{
            outfile = job.stdout;
        }
        if(i+1<job.cmdlist_.size()  && job.cmdlist_.is_redirect_out(job.cmdlist_.at(i+1))){
            int fd = -1;
            if(job.cmdlist_.at(i+1).kind == CommandKind::REDIRECT_OUT_NEW){
                fd = open(job.cmdlist_.at(i+1).argv[0], O_WRONLY | O_TRUNC | O_CREAT, 0666);
            }
            if(job.cmdlist_.at(i+1).kind == CommandKind::REDIRECT_OUT_ADD){
                fd = open(job.cmdlist_.at(i+1).argv[0], O_WRONLY | O_APPEND | O_CREAT, 0666);
            }
            if(fd < 0){
                std::cerr << "File Open Failed: " << job.cmdlist_.at(i+1).argv[0] << std::endl;
                exit(1); 
            }
            outfile = fd;
            std::cerr << "REDIRECT_OUT: " << outfile << std::endl;
        }
        if(i+1<job.cmdlist_.size()  && job.cmdlist_.is_redirect_in(job.cmdlist_.at(i+1))){
            int fd = open(job.cmdlist_.at(i+1).argv[0], O_RDONLY);
            if(fd < 0){
                std::cerr << "File Open Failed: " << job.cmdlist_.at(i+1).argv[0] << std::endl;
                exit(1); 
            }
            infile = fd;
            outfile = job.stdout;
            std::cerr << "REDIRECT_IN: " << infile << std::endl;
        }
        job.cmdlist_.at(i).pid = fork(); 
        if(job.cmdlist_.at(i).pid == 0){ // child process
            launch_process_(job.cmdlist_.at(i), job.pgid, infile, outfile, foreground);
        }else if(job.cmdlist_.at(i).pid < 0){
            std::cerr << "Error: fork" << std::endl;
            exit(1);
        }else{ // parent proces
            if(job.pgid <= 0){ job.pgid = job.cmdlist_.at(i).pid;}
            setpgid(job.cmdlist_.at(i).pid, job.pgid);
        }
        if(infile  != job.stdin){  close(infile);}
        if(outfile != job.stdout){ close(outfile);}
        infile = fds[0];
    }
    append_(job);
    if(foreground){
        tcsetpgrp(shell_terminal_fd_, job.pgid);
        wait_job_(joblist.at_(joblist.size_()-1));
        tcsetpgrp(shell_terminal_fd_, shell_pgid_);
    }
}

void JobList::wait_job_(Job& job)
{
    int status = -1;
    pid_t pid;
    do{
        pid = waitpid(-1, &status, WUNTRACED);
        if(pid < 0){
            perror("waitpid");
        }
    }while(search_process_(pid, status) && !job.is_completed() && !job.is_stopped());
}

bool JobList::search_process_(pid_t pid, int status)
{
    if(pid > 0)
    {
        for(Job& j: joblist_)
        {
            for(Command& cmd: j.cmdlist_)
            {
                if(cmd.pid == pid) // 受け取ったpidが管理しているジョブに該当
                {
                    // ステータスの更新
                    cmd.status = status;    
                    if(WIFSTOPPED(status))
                    {
                        // 一時停止
                        cmd.stopped = true;
                    }
                    else
                    {
                        // 完了
                        cmd.completed = true;
                        if(WIFSIGNALED(status))
                        {
                            // シグナルによるkill
                            std::cerr << pid << ": Terminated by signal " << WTERMSIG(cmd.status) << std::endl;
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }
    else if(pid == 0 || errno == ECHILD)
    {
        return false;
    }
    else
    {
        perror("waitpid");
        return false;
    }
}