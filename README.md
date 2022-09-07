# nihonsh
酔っ払ってても扱えるシェルを目指します。
umeshと迷いました。

# 開発方針
1. 酒を飲みながら開発をすること
2. 週1日以上の休肝日を設けること
3. 呑んでるときのミスを書き残しておくこと

# 機能
* execute command
```sh
$ cd ./programs
```

* pipeline
```sh
$ ls -la | grep -e "kern" | 
```

* redirect
```
$ ls -la > ./output.txt
$ command < ./input.txt
```

* complement
```sh
$ systemc # tab-complement
$ systemctl 
```

# 呑んでるときのミス集
1. `rm -rf /`やっちゃった
2. `sudo apt remove libreadline8`で依存関係全部消しちゃった