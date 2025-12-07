# rlwrap 使用指南

## 基本介绍

`rlwrap` (readline wrapper) 是一个为交互式程序添加命令行编辑功能的工具。

## 核心功能

- **命令历史**: 上下箭头浏览历史命令
- **行编辑**: 左右移动光标、删除、复制粘贴
- **Tab 补全**: 支持自动补全
- **持久化历史**: 历史记录保存到文件

## 安装

```bash
# Ubuntu/Debian
sudo apt install rlwrap

# macOS
brew install rlwrap

# Arch Linux
sudo pacman -S rlwrap
```

## 基本用法

### 1. 包装简单的交互式程序

```bash
# Python REPL
rlwrap python

# SQLite
rlwrap sqlite3 database.db

# Netcat
rlwrap nc localhost 8080
```

### 2. 配合 C REPL (crepl) 使用

`crepl` 是一个 C 语言的交互式解释器,可以动态加载共享库。

```bash
# 基本用法
rlwrap ./crepl

# 加载动态链接库
rlwrap ./crepl ./path/to/library.so

# 实际示例:加载 raylib 图形库
rlwrap ./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so
```

**使用场景说明:**
- `crepl`: C 语言 REPL 程序
- `libraylib.so`: raylib 游戏开发库的动态链接库
- `rlwrap` 提供命令历史和编辑功能

在 crepl 中可以直接调用 raylib 的函数:

```c
// 在 crepl 交互环境中
InitWindow(800, 600, "Hello");
BeginDrawing();
ClearBackground(RAYWHITE);
EndDrawing();
CloseWindow();
```

### 3. 常用选项

```bash
# 指定历史文件位置
rlwrap -H ~/.myapp_history myapp

# 设置历史记录大小
rlwrap -s 1000 myapp

# 启用文件名补全
rlwrap -f myapp

# 多行输入支持
rlwrap -m myapp

# 忽略重复的历史记录
rlwrap -D 2 myapp
```

### 4. 高级用法

```bash
# 自定义补全列表
echo -e "function1\nfunction2\nfunction3" > completions.txt
rlwrap -f completions.txt ./myprogram

# 设置提示符颜色
rlwrap -pRed ./myprogram

# 组合多个选项
rlwrap -H ~/.crepl_history -s 5000 -m ./crepl ./lib.so
```

## 实战示例

### 示例 1: C 动态库交互式测试

```bash
# 编译动态库
gcc -shared -fPIC -o libmath.so math.c

# 使用 rlwrap + crepl 交互式测试
rlwrap ./crepl ./libmath.so

# 在 crepl 中调用函数
> int result = add(5, 3);
> printf("Result: %d\n", result);
```

### 示例 2: 游戏开发调试 (raylib)

```bash
# 启动带历史记录的 raylib REPL
rlwrap -H ~/.raylib_history ./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so

# 交互式测试 raylib API
> InitWindow(800, 600, "Test");
> SetTargetFPS(60);
> // 测试各种函数...
> CloseWindow();
```

### 示例 3: 网络协议调试

```bash
# 使用 rlwrap 包装 telnet
rlwrap telnet example.com 80

# 手动输入 HTTP 请求,支持历史记录和编辑
GET / HTTP/1.1
Host: example.com
```

## 快捷键

- `Ctrl+R`: 反向搜索历史
- `Ctrl+P` / `↑`: 上一条命令
- `Ctrl+N` / `↓`: 下一条命令
- `Ctrl+A`: 行首
- `Ctrl+E`: 行尾
- `Ctrl+K`: 删除到行尾
- `Ctrl+U`: 删除到行首
- `Ctrl+W`: 删除前一个单词

## 历史文件位置

默认历史文件保存在:
```
~/.local/share/rlwrap/
```

可以通过 `-H` 选项自定义位置。

## 注意事项

1. **性能影响**: rlwrap 对性能影响极小
2. **兼容性**: 几乎支持所有交互式程序
3. **历史安全**: 敏感命令会被记录,注意清理历史文件
4. **终端要求**: 需要支持 ANSI 转义序列的终端

## 相关工具

- `readline`: GNU 命令行编辑库
- `ledit`: 另一个类似的包装工具
- `socat`: 更强大的网络工具,也支持 readline
