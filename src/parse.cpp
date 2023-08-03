#include "parse.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <string.h>
#include <functional>
#include "builtin.h"

Command::Command(): argc(0), argv(), status(-1), pid(-1), kind(CommandKind::EXECUTE){}

Command::Command(std::vector<char*> _argv, CommandKind _kind): status(-1), pid(-1), kind(_kind)
{
    this->argc = _argv.size();
    std::copy(_argv.begin(), _argv.end(), std::back_inserter(this->argv));
    if(this->argv.back() != NULL){
        this->argv.push_back(NULL);
    }
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

std::ostream& operator<<(std::ostream& stream, const Command& cmd)
{
    stream << "Command(";
    stream << "argc=" << cmd.argc << ", ";
    if(cmd.argc > 0){
        stream << "argv={";
        for(int i=0; i<cmd.argc-1; i++){
            stream << "\"" << cmd.argv[i] << "\"" << ",";
        }
        if(cmd.argv[cmd.argc-1] == NULL){
            stream << "NULL";
        }
        stream << "}, ";
    }else{
        stream << "argv=null, ";
    }
    stream << "pid=" << cmd.pid;
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
            cmdlist.append(Command({}, CommandKind::REDIRECT_OUT_ADD));
            i++;
            continue;
        }
        if(tknlist.at(i).kind == TokenKind::REDIRECT_OUT_NEW){
            cmdlist.append(Command({}, CommandKind::REDIRECT_OUT_NEW));
            i++;
            continue;
        }
        if(tknlist.at(i).kind == TokenKind::REDIRECT_IN){
            i++;
            continue;
        }
        while(i < tknlist.size() && tknlist.at(i).kind == TokenKind::ID){
            tmp.emplace_back(tknlist.at(i).str);
            i++;
        }
        if(!tmp.empty()){
            tmp.push_back(NULL);
            cmdlist.append(Command(tmp, CommandKind::EXECUTE));
            tmp.clear();
        }
    }
    return cmdlist;
}

void execute_pipeline(CommandList& cmdlist)
{
    int fds1[2] = {-1, -1};
    int fds2[2] = {-1, -1};
    for(Command& cmd : cmdlist)
    {
        builtin_t* blt = lookup_builtin(cmd.argv[0]);
        if(blt != nullptr){
            blt->func(cmd.argc, cmd.argv.data());
        }
        fds1[0] = fds2[0];
        fds1[1] = fds2[1];
        if(!cmdlist.is_tail(cmd)){
            pipe(fds2);
        }
        cmd.pid = fork();
        if(cmdlist.is_parent(cmd)){
            if(fds1[0] != -1) close(fds1[0]);
            if(fds1[1] != -1) close(fds1[1]);
            continue;
        }
        if(!cmdlist.is_head(cmd)){
            close(0); dup2(fds1[0], 0); close(fds1[0]);
            close(fds1[1]);
        }
        if(!cmdlist.is_tail(cmd)){
            close(1); dup2(fds2[1], 1); close(fds2[1]);
            close(fds2[0]);
            
        }
        if()
        execvp(cmd.argv[0], cmd.argv.data());
        std::cerr << "execvp error(" << errno << "): " << cmd << std::endl;
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