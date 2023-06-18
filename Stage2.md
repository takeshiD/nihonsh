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

# 解析 parsing
```sh
cmd1 | cmd2 | cmd3  # [cmd1, cmd2, cmd3]
cmd1; cmd2;         # [cmd1]->exec, [cmd2]->exec
cmd1 < file         # 
cmd1 > file         #
cmd1 >> file        #
```