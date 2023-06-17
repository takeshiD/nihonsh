# パイプライン, リダイレクト
## パイプライン
```sh
cmd1 | cmd2 | cmd3
# [cmd1, cmd2, cmd3]
```

## リダイレクト
```sh
# 入力
cmd < file          # fileの内容をcmdのstdinに渡す

# 出力 0:stdin, 1:stdout, 2:stderr
cmd >&2             # stdoutをstderrに入れる
cmd > file          # cmdのstdoutをfile.txtに入れて、上書きする。ファイルが無ければ作成。
cmd >> file         # cmdのstdoutをfile.txtに入れて、追加する
cmd 2> file         # stderrをfileに入れる

cmd &> file         # stdout,stderrをfileに入れて上書き。ファイルが無ければ作成
cmd > file 2>&1     # 同上

cmd &>> file        # stdout,stderrをfileに入れて追加書き込み。
cmd >> file 2>&1    # 同上
```

# 解析 parsing
```sh
cmd1 | cmd2 | cmd3  # [cmd1, cmd2, cmd3]
cmd1; cmd2;         # [cmd1]->exec, [cmd2]->exec
cmd1 < file         # 
cmd1 > file         #
cmd1 >> file        #
```

# Command構造体

```cpp
enum class CommandType{
    HEAD,
    TAIL,
    REDIRECT,
    PIPE
};

class Command {
    int capa = 1024;
public:
    int argc;
    char** argv;
    CommandType cmdtype;
    Command* next;
    Command(): arg(0), argv(NULL), cmdtype(CommandHEAD){}
    Command(int _argc, char** _argv, CommandType _cmdtype, Command* _next): argc(argc), argv(_argv), cmdtype(_cmdtype), next(next) {}
};

class CommandList {
public:
    Command* head;
};
```