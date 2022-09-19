# C言語入門書の次に読む本
## dbg_malloc

```gcc --version```
```
Apple clang version 13.1.6 (clang-1316.0.21.2.5)
Target: arm64-apple-darwin21.1.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```

```sample```を実行するとsegmentation faultで落ちる。
lldb(gccと言いつつclangが使われている)で追ってみると下記の場所で止まる。
```
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x0)
    frame #0: 0x0000000100002d88 sample`dbg_malloc(size=10, file="main.c", line=11) at dbg_malloc.c:73:13
   70           goto err;
   71       }
   72  
-> 73       mbp->id = id++;
   74       mbp->size = size;
   75       mbp->t = time(NULL);
   76       mbp->mcp = mcp;
Target 0: (sample) stopped.
```
```p id```してみるとずっとid=1となっている。