# パイプライン, リダイレクト

## dup
![](/images/dup2.svg)

## pipe
![](/images/pipe.svg)

## パイプライン

```sh
cmd1 | cmd2
```

![](/images/pipeline.svg)


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

![](/images/redirect.svg)


# 設計
1. プロンプトからの入力: char*
2. 字句解析
    - TokenList(Token(), Token(), ...);
3. 構文解析
    - CommandList(Command(), Command(), ...);
4. 実行
    - invoke_command(CommandList);


## 構文解析
### トークン
* '|': PIPE
* '<': REDIRECT_IN
* '>': REDIRECT_OUT
* ' ','\t': WSP
* 上記以外の表示文字: ID

### 構文規則(EBNF)
expr := cmd (ctrl expr)?
cmd  := ID cmd*
ctrl := PIPE | REDIRECT_IN | REDIRECT_OUT

### 構文規則(bash base)
inputunit := simple_list '\n'
    | '\n'
    | EOF
    ;
simple_list := simple_list1
    | simple_list1 '&'
    | simple_list1 ';'
    ;
simple_list1 := simple_list1 AND_AND newlines simple_list1
    | simple_list1 OR_OR newlines simple_list1
    | simple_list1 '&' simple_list1
    | simple_list1 ';' simple_list1
    | pipeline
    | BANG pipeline
    ;

(確認)
"ls -la | grep"
-> "ls":ID, "-la":ID, "|":PIPE ,"grep":ID
-> ID ID PIPE ID
-> cmd ID PIPE ID
-> cmd PIPE ID
-> cmd ctrl ID
-> cmd ctrl cmd
-> cmd (ctrl expr)
-> expr

# termios
| c_lflag | Decimal | Binary  | Description |
|:-------:|:-------:|:-------:|:---:|
| ISIG    | 0000001 | 00000001 | Enable Signals  |
| ICANO   | 0000002 | 00000010 | Canonical input |
| XCASE   | 0000004 | 00000100 |             |
| ECHO    | 0000010 | 00001010 | Enable echo |
| ECHOE   | 0000020 | 00010100 | Echo erase character as error-correcting backspace |
| ECHOK   | 0000040 | 00101000 | Echo KILL   |
| ECHONL  | 0000100 | 00110100 | Echo NL     |
| ECHOCTL | 0001000 | 11101000 | ECHO is also set, terminal special characters other than TAB,NL,START,STOP are echoed as ^X, where X is the character with ASCII code 0x40 greater than the special character(not in POSIX)|
| ECHOPRT | 0002000 | 11010000 | If ICANON and ECHO are alse set, characters are printed as they are being erased |
| ECHOKE  | 0004000 | 10100000 | If ICANON is alse set, KILL is echoed by erasing each character on the line, as specified by ECHOE and ECHOPRT |

