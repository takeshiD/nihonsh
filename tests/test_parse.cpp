#include <stdexcept>
#include <iostream>
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"
#include "parse.h"

/*********   Success Case  *********/
TEST(COMMAND, CONSRUCTOR)
{
    Command cmd1;
    EXPECT_EQ(cmd1.argc, 0);
    EXPECT_TRUE(cmd1.argv.empty());
    EXPECT_EQ(cmd1.status, -1);
    EXPECT_EQ(cmd1.pid, -1);

    Command cmd2({"ls", "-la"}, CommandKind::EXECUTE);
    EXPECT_EQ(cmd2.argc, 2);
    EXPECT_STREQ(cmd2.argv[0], "ls");
    EXPECT_STREQ(cmd2.argv[1], "-la");
    EXPECT_EQ(cmd2.status, -1);
    EXPECT_EQ(cmd2.pid, -1);
}
TEST(COMMANDLIST, CONSRUCTOR)
{
    CommandList cmdlist;
    cmdlist.append({"ls", "-la"}, CommandKind::EXECUTE);
    cmdlist.append({"grep", "-e", "pipe"}, CommandKind::EXECUTE);
    cmdlist.append({"head", "-n", "2"}, CommandKind::EXECUTE);
    EXPECT_STREQ(cmdlist.at(0).argv[0], "ls");
    EXPECT_STREQ(cmdlist.at(1).argv[0], "grep");
    EXPECT_STREQ(cmdlist.at(2).argv[0], "head");
}

TEST(TOKENIZE, NO_INPUT)
{
    char line[256] = "";
    TokenList tknlist = tokenize(line);
    EXPECT_EQ(tknlist.size(), 0);
    EXPECT_THROW(tknlist.at(0), std::out_of_range);
}

TEST(TOKENIZE, SINGLE)
{
    char line[256] = "ls";
    TokenList tknlist = tokenize(line);
    EXPECT_EQ(tknlist.size(), 1);
    EXPECT_STREQ(tknlist.at(0).str, "ls");
    EXPECT_EQ(tknlist.at(0).kind, TokenKind::ID);
}


TEST(TOKENIZE, PIPE_REDIRECT)
{
    char line[256] = "ls -la | grep -e pipe > result.txt";
    TokenList tknlist = tokenize(line);
    EXPECT_EQ(tknlist.size(), 8);

    EXPECT_STREQ(tknlist.at(0).str, "ls");
    EXPECT_STREQ(tknlist.at(1).str, "-la");
    EXPECT_STREQ(tknlist.at(2).str, "|");
    EXPECT_STREQ(tknlist.at(3).str, "grep");
    EXPECT_STREQ(tknlist.at(4).str, "-e");
    EXPECT_STREQ(tknlist.at(5).str, "pipe");
    EXPECT_STREQ(tknlist.at(6).str, ">");
    EXPECT_STREQ(tknlist.at(7).str, "result.txt");

    EXPECT_EQ(tknlist.at(0).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(1).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(2).kind, TokenKind::PIPE);
    EXPECT_EQ(tknlist.at(3).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(4).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(5).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(6).kind, TokenKind::REDIRECT_OUT_NEW);
    EXPECT_EQ(tknlist.at(7).kind, TokenKind::ID);
}
TEST(TOKENIZE, PIPE_PIPE)
{
    char line[256] = "| ls -la          |";
    TokenList tknlist = tokenize(line);
    EXPECT_EQ(tknlist.size(), 4);

    EXPECT_STREQ(tknlist.at(0).str, "|");
    EXPECT_STREQ(tknlist.at(1).str, "ls");
    EXPECT_STREQ(tknlist.at(2).str, "-la");
    EXPECT_STREQ(tknlist.at(3).str, "|");
    
    EXPECT_EQ(tknlist.at(0).kind, TokenKind::PIPE);
    EXPECT_EQ(tknlist.at(1).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(2).kind, TokenKind::ID);
    EXPECT_EQ(tknlist.at(3).kind, TokenKind::PIPE);
}

TEST(PARSE, CASE1)
{
    TokenList tknlist;
    tknlist.append(Token("|", TokenKind::PIPE));
    tknlist.append(Token("ls", TokenKind::ID));
    tknlist.append(Token("-la", TokenKind::ID));
    tknlist.append(Token("|", TokenKind::ID));
    CommandList cmdlist = parse(tknlist);
    EXPECT_STREQ(cmdlist.at(0).argv[0], "ls");
    EXPECT_STREQ(cmdlist.at(0).argv[1], "-la");
}

TEST(PARSE, CASE2)
{
    char line[256] = "ls -la | grep -e pipe";
    TokenList tknlist;
    tknlist.append(Token("ls",  TokenKind::ID));
    tknlist.append(Token("-la", TokenKind::ID));
    tknlist.append(Token("|",   TokenKind::PIPE));
    tknlist.append(Token("grep",TokenKind::ID));
    tknlist.append(Token("-e",  TokenKind::ID));
    tknlist.append(Token("pipe",TokenKind::ID));
    CommandList cmdlist = parse(tknlist);
    EXPECT_STREQ(cmdlist.at(0).argv[0], "ls");
    EXPECT_STREQ(cmdlist.at(0).argv[1], "-la");
}
TEST(INVOKE, STDOUT)
{
    CommandList cmdlist;
    cmdlist.append({"seq", "-w", "10"}, CommandKind::EXECUTE);
    cmdlist.append({"grep", "02"}, CommandKind::EXECUTE);
    testing::internal::CaptureStdout();
    invoke_command(cmdlist);
    EXPECT_STREQ("02\n", testing::internal::GetCapturedStdout().c_str());
}