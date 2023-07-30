#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <vector>

enum class CommandKind
{
    EXECUTE,
    REDIRECT_OUT_NEW,
    REDIRECT_OUT_ADD,
};

struct Command
{
    int argc;
    std::vector<char*> argv;
    int status;
    CommandKind kind;
    pid_t pid;
    Command();
    Command(std::vector<char*> _argv, CommandKind _kind);
};
struct CommandList
{
private:
    std::vector<Command> cmdlist_{};
public:
    typedef std::vector<Command>::iterator iterator;
    typedef std::vector<Command>::const_iterator const_iterator;
    typedef std::vector<Command>::reverse_iterator reverse_iterator;
    typedef std::vector<Command>::const_reverse_iterator const_reverse_iterator;
    CommandList();
    void append(std::vector<char*> cmdchar, CommandKind kind);
    void append(Command&& cmd);
    Command& at(int idx);
    std::size_t size() const;

    // forward iterator
    iterator begin(){ return this->cmdlist_.begin();}
    const_iterator begin() const { return this->cmdlist_.begin();}
    iterator end(){ return this->cmdlist_.end();}
    const_iterator end() const { return this->cmdlist_.end();}
    // reverse iterator
    reverse_iterator rbegin(){ return this->cmdlist_.rbegin();}
    const_reverse_iterator rbegin() const { return this->cmdlist_.rbegin();}
    reverse_iterator rend(){ return this->cmdlist_.rend();}
    const_reverse_iterator rend() const { return this->cmdlist_.rend();}

    bool is_tail(Command& cmd);
    bool is_head(Command& cmd);
    bool is_parent(Command& cmd);
};

enum class TokenKind
{
    NONE,
    ID,
    PIPE,
    REDIRECT_IN,
    REDIRECT_OUT_NEW,
    REDIRECT_OUT_ADD,
};

struct Token
{
    char* str;
    TokenKind kind;
    Token(char* _str, TokenKind _kind):str(_str),kind(_kind){}
};
class TokenList
{
private:
    std::vector<Token> tknlist_{};
public:
    TokenList();
    void append(Token&& tkn);
    Token& at(int idx);
    std::size_t size() const;
    typedef std::vector<Token>::iterator iterator;
    typedef std::vector<Token>::const_iterator const_iterator;
    typedef std::vector<Token>::reverse_iterator reverse_iterator;
    typedef std::vector<Token>::const_reverse_iterator const_reverse_iterator;
    // forward iterator
    iterator begin(){ return this->tknlist_.begin();}
    const_iterator begin() const { return this->tknlist_.begin();}
    iterator end(){ return this->tknlist_.end();}
    const_iterator end() const { return this->tknlist_.end();}
    // reverse iterator
    reverse_iterator rbegin(){ return this->tknlist_.rbegin();}
    const_reverse_iterator rbegin() const { return this->tknlist_.rbegin();}
    reverse_iterator rend(){ return this->tknlist_.rend();}
    const_reverse_iterator rend() const { return this->tknlist_.rend();}
};

std::ostream& operator<<(std::ostream& stream, const Command& cmd);
std::ostream& operator<<(std::ostream& stream, const CommandList& cmdlist);
std::ostream& operator<<(std::ostream& stream, const Token& tkn);
std::ostream& operator<<(std::ostream& stream, const TokenList& tknlist);

bool is_space(char* c);

TokenList tokenize(char* line);
CommandList parse(TokenList tknlist);
void invoke_command(CommandList& cmdlist);
#endif // PARSE_H