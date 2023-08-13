#include "parse.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <string.h>
#include <functional>
#include <fcntl.h>
#include "builtin.h"

Command::Command(): argc(0), argv(), status(-1), pid(-1), kind(CommandKind::EXECUTE){}

Command::Command(std::vector<char*> _argv, CommandKind _kind): status(-1), pid(-1), kind(_kind)
{
    std::copy(_argv.begin(), _argv.end(), std::back_inserter(this->argv));
    this->argc = _argv.size();
    this->argv.push_back(NULL);
}

CommandList::CommandList(){}
void CommandList::append(std::vector<char*> cmdchar, CommandKind kind)
{
    this->cmdlist_.emplace_back(cmdchar, kind);
}
void CommandList::append(Command&& cmd)
{
    this->cmdlist_.emplace_back(cmd);
}
Command& CommandList::at(int idx)
{
    return this->cmdlist_.at(idx);
}
std::size_t CommandList::size() const
{
    return this->cmdlist_.size();
}
bool CommandList::is_head(Command& cmd)
{
    return &this->cmdlist_.front() == &cmd;
}
bool CommandList::is_tail(Command& cmd)
{
    return &this->cmdlist_.back() == &cmd;
}
bool CommandList::is_parent(Command& cmd)
{
    return cmd.pid>0;
}
bool CommandList::is_redirect_out(Command& cmd)
{
    bool ret = (cmd.kind == CommandKind::REDIRECT_OUT_NEW) || (cmd.kind == CommandKind::REDIRECT_OUT_ADD);
    return ret;
}
bool CommandList::is_redirect_in(Command& cmd)
{
    return cmd.kind == CommandKind::REDIRECT_IN;
}



std::ostream& operator<<(std::ostream& stream, const Command& cmd)
{
    stream << "Command(";
    stream << "argc=" << cmd.argc << ", ";
    if(cmd.argc > 0){
        stream << "argv={";
        for(int i=0; i<cmd.argc; i++){
            stream << "\"" << cmd.argv[i] << "\"" << ",";
        }
        if(cmd.argv[cmd.argc] == NULL){
            stream << "NULL";
        }
        stream << "}, ";
    }else{
        stream << "argv=null, ";
    }
    stream << "pid=" << cmd.pid << ", ";
    stream << "kind=" << static_cast<int>(cmd.kind);
    stream << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const CommandList& cmdlist)
{
    stream << "[";
    for(auto& cmd: cmdlist){
        stream << cmd << ",";
    }
    stream << "]";
    return stream;
}



TokenList::TokenList(){}
void TokenList::append(Token&& tkn)
{
    this->tknlist_.emplace_back(tkn);
}
Token& TokenList::at(int idx)
{
    return this->tknlist_.at(idx);
}
std::size_t TokenList::size() const
{
    return this->tknlist_.size();
}

std::ostream& operator<<(std::ostream& stream, const Token& tkn)
{
    stream << "Token(";
    stream << "kind=" << static_cast<int>(tkn.kind) << ",";
    stream << "str=" << tkn.str << ")";
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const TokenList& tknlist)
{
    stream << "[";
    for(auto tkn: tknlist){
        stream << tkn << ","; 
    }
    stream << "]";
    return stream;
}

bool is_space(char* c)
{
    return (*c == ' ' || *c == '\t');
}
bool is_pipe(char* c)
{
    return *c == '|';
}
bool is_leftangle_single(char* c)
{
    return *c == '<';
}
bool is_leftangle_double(char* c)
{
    return strncmp(c, "<<", 2) == 0;
}
bool is_rightangle_single(char* c)
{
    return *c == '>';
}
bool is_rightangle_double(char* c)
{
    return strncmp(c, ">>", 2) == 0;
}

bool is_reserved(char* c)
{
    return is_pipe(c) || is_leftangle_double(c) || is_leftangle_single(c) || is_rightangle_double(c) || is_rightangle_single(c);
}
bool is_identifier(char* c)
{
    return !is_space(c) && !is_reserved(c) && (32<=*c && *c<=126);
}

TokenList tokenize(char* line)
{
    TokenList tknlist;
    char* p = line;
    while(*p){
        if(is_space(p)){
            *p ='\0';
            p++;
            continue;
        }
        if(is_pipe(p)){
            tknlist.append(Token(p, TokenKind::PIPE));
            p++;
            continue;
        }
        if(is_rightangle_double(p)){
            tknlist.append(Token(p, TokenKind::REDIRECT_OUT_ADD));
            p += 2;
            continue;
        }
        if(is_rightangle_single(p)){
            tknlist.append(Token(p, TokenKind::REDIRECT_OUT_NEW));
            p++;
            continue;
        }
        if(is_leftangle_single(p)){
            tknlist.append(Token(p, TokenKind::REDIRECT_IN));
            p++;
            continue;
        }
        if(is_identifier(p)){
            tknlist.append(Token(p, TokenKind::ID));
            while(*p && is_identifier(p)) p++;
            continue;
        }
    }
    return tknlist;
}

CommandList parse(TokenList tknlist)
{
    CommandList cmdlist;
    int i = 0;
    std::vector<char*> tmp;
    while(i < tknlist.size())
    {
        if(tknlist.at(i).kind == TokenKind::PIPE){
            i++;
            continue;
        }
        if(tknlist.at(i).kind == TokenKind::REDIRECT_OUT_ADD){
            i++;
            if(i >= tknlist.size()){
                std::cerr << "[Error] redirect_outに続くファイル名がありません" << std::endl;
                return CommandList();
            }
            cmdlist.append({tknlist.at(i++).str}, CommandKind::REDIRECT_OUT_ADD);
            continue;
        }
        if(tknlist.at(i).kind == TokenKind::REDIRECT_OUT_NEW){
            i++;
            if(i >= tknlist.size()){
                std::cerr << "[Error] redirect_outに続くファイル名がありません" << std::endl;
                return CommandList();
            }
            cmdlist.append({tknlist.at(i++).str}, CommandKind::REDIRECT_OUT_NEW);
            continue;
        }
        if(tknlist.at(i).kind == TokenKind::REDIRECT_IN){
            i++;
            if(i >= tknlist.size()){
                std::cerr << "[Error] redirect_inに続くファイル名がありません" << std::endl;
                return CommandList();
            }
            cmdlist.append({tknlist.at(i++).str}, CommandKind::REDIRECT_IN);
            continue;
        }
        while(i < tknlist.size() && tknlist.at(i).kind == TokenKind::ID){
            tmp.emplace_back(tknlist.at(i).str);
            i++;
        }
        if(!tmp.empty()){
            cmdlist.append(tmp, CommandKind::EXECUTE);
            tmp.clear();
        }
    }
    return cmdlist;
}

void execute_pipeline(CommandList& cmdlist)
{
    int fds1[2] = {-1, -1};
    int fds2[2] = {-1, -1};
    // std::cout << "Commands: " << cmdlist << std::endl;
    for(int i=0; i<cmdlist.size(); i++)
    {
        builtin_t* blt = lookup_builtin(cmdlist.at(i).argv[0]);
        if(blt != nullptr){
            blt->func(cmdlist.at(i).argc, cmdlist.at(i).argv.data());
            continue;
        }
        if(cmdlist.is_redirect_out(cmdlist.at(i)) || cmdlist.is_redirect_in(cmdlist.at(i))){
            continue;
        }
        fds1[0] = fds2[0];
        fds1[1] = fds2[1];
        if(!cmdlist.is_tail(cmdlist.at(i))){
            pipe(fds2);
        }
        cmdlist.at(i).pid = fork();
        if(cmdlist.is_parent(cmdlist.at(i))){
            if(fds1[0] != -1) close(fds1[0]);
            if(fds1[1] != -1) close(fds1[1]);
            continue;
        }
        // pipe
        if(!cmdlist.is_head(cmdlist.at(i))){
            close(0); dup2(fds1[0], 0); close(fds1[0]);
            close(fds1[1]);
        }
        if(!cmdlist.is_tail(cmdlist.at(i))){
            close(1); dup2(fds2[1], 1); close(fds2[1]);
            close(fds2[0]);
        }
        if(i+1 < cmdlist.size() && cmdlist.is_redirect_out(cmdlist.at(i+1))){ // redirect_out
            int oflag = 0;
            if(cmdlist.at(i+1).kind == CommandKind::REDIRECT_OUT_NEW){
                oflag = O_WRONLY | O_CREAT | O_TRUNC;
            }
            else if(cmdlist.at(i+1).kind == CommandKind::REDIRECT_OUT_ADD){
                oflag = O_WRONLY | O_CREAT | O_APPEND;
            }
            int fd = open(cmdlist.at(i+1).argv[0], oflag, 0666);
            if(fd < 0){
                std::cerr << "[Error] " << cmdlist.at(i+1).argv[0] << " was not made." << std::endl;
                exit(1);
            }
            close(1);
            dup2(fd, 1);
            close(fd);
        }
        if(i+1 < cmdlist.size() && cmdlist.is_redirect_in(cmdlist.at(i+1))){ // redirect_in
            int fd = open(cmdlist.at(i+1).argv[0], O_RDONLY | O_TRUNC);
            if(fd < 0){
                std::cerr << "[Error] You don't have permission to open " << cmdlist.at(i+1).argv[0] << std::endl;
                exit(1);
            }
            close(0);
            dup2(fd, 0);
            close(fd);
        }
        execvp(cmdlist.at(i).argv[0], cmdlist.at(i).argv.data());
        std::cerr << "execvp error(" << errno << "): " << cmdlist.at(i) << std::endl;
        exit(-1);
    }
}

void wait_pipeline(CommandList& cmdlist)
{
    std::for_each(cmdlist.rbegin(), cmdlist.rend(), [](Command cmd){
        waitpid(cmd.pid, &cmd.status, 0);
    });
}
void invoke_command(CommandList& cmdlist)
{
    execute_pipeline(cmdlist);
    wait_pipeline(cmdlist);
}