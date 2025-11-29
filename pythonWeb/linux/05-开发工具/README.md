# 05 - 开发工具

> 编译、调试、版本控制等开发必备工具

## 目录

- [GCC/G++ - 编译器](#gccg---编译器)
- [Make - 构建工具](#make---构建工具)
- [GDB - 调试器](#gdb---调试器)
- [Git - 版本控制](#git---版本控制)
- [其他开发工具](#其他开发工具)

---

## GCC/G++ - 编译器

### 安装

```bash
# Arch Linux 安装
sudo pacman -S base-devel  # 包含 gcc, g++, make 等
```

### 基本用法

```bash
# 编译C程序
gcc hello.c -o hello

# 编译C++程序
g++ hello.cpp -o hello

# 编译多个文件
gcc main.c utils.c -o program

# 只编译不链接
gcc -c file.c  # 生成 file.o

# 链接目标文件
gcc file1.o file2.o -o program
```

### 常用编译选项

```bash
# 优化级别
gcc -O0 file.c -o program  # 无优化（默认）
gcc -O1 file.c -o program  # 基本优化
gcc -O2 file.c -o program  # 推荐优化
gcc -O3 file.c -o program  # 最大优化
gcc -Os file.c -o program  # 优化大小

# 警告选项
gcc -Wall file.c           # 启用所有警告
gcc -Werror file.c         # 警告当错误
gcc -Wextra file.c         # 额外警告
gcc -pedantic file.c       # 严格标准检查

# 调试信息
gcc -g file.c -o program   # 包含调试信息
gcc -g3 file.c             # 最详细调试信息

# C/C++ 标准
gcc -std=c11 file.c        # C11 标准
g++ -std=c++17 file.cpp    # C++17 标准
g++ -std=c++20 file.cpp    # C++20 标准

# 包含头文件路径
gcc -I/path/to/headers file.c

# 链接库
gcc file.c -lm             # 链接数学库
gcc file.c -lpthread       # 链接线程库
gcc file.c -L/path/to/libs -lmylib  # 指定库路径

# 宏定义
gcc -DDEBUG file.c         # 定义 DEBUG 宏
gcc -DVERSION=1.0 file.c   # 定义带值的宏
```

### 实用组合

```bash
# 开发版本（带调试信息和警告）
gcc -Wall -Wextra -g -O0 main.c -o program

# 发布版本（优化和去除调试信息）
gcc -O2 -DNDEBUG main.c -o program

# 严格检查
gcc -Wall -Wextra -Werror -pedantic -std=c11 file.c

# 生成依赖文件
gcc -MM file.c > dependencies.mk
```

### 预处理和汇编

```bash
# 只预处理
gcc -E file.c -o file.i

# 生成汇编代码
gcc -S file.c -o file.s

# 查看汇编（人类可读）
gcc -S -O2 -fverbose-asm file.c
```

---

## Make - 构建工具

### 安装

```bash
sudo pacman -S make
```

### Makefile 基础

**简单示例**:
```makefile
# Makefile

# 变量
CC = gcc
CFLAGS = -Wall -g
TARGET = program
OBJS = main.o utils.o

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 编译
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 重新构建
rebuild: clean all

# 伪目标
.PHONY: all clean rebuild
```

**自动变量**:
```makefile
# $@ - 目标文件名
# $< - 第一个依赖文件
# $^ - 所有依赖文件
# $? - 所有比目标新的依赖

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ -o $@
```

### Make 命令

```bash
# 执行默认目标
make

# 执行指定目标
make clean
make all

# 并行编译（4个任务）
make -j4

# 并行编译（所有核心）
make -j$(nproc)

# 显示执行的命令
make -n

# 继续执行（忽略错误）
make -k

# 指定 Makefile
make -f MyMakefile
```

### 完整示例

```makefile
# Makefile for C++ project

# 编译器和标志
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lpthread

# 目录
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# 文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/myapp

# 默认目标
all: $(TARGET)

# 创建目录
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 链接
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# 编译
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# 清理
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 运行
run: $(TARGET)
	./$(TARGET)

# 依赖
-include $(OBJS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -MM -MT '$(@:.d=.o)' $< > $@

.PHONY: all clean run
```

---

## GDB - 调试器

### 安装

```bash
sudo pacman -S gdb
```

### 编译带调试信息的程序

```bash
gcc -g program.c -o program
```

### 基本命令

```bash
# 启动GDB
gdb ./program
gdb --args ./program arg1 arg2

# 运行程序
(gdb) run
(gdb) r

# 带参数运行
(gdb) run arg1 arg2

# 设置断点
(gdb) break main          # 在函数设置
(gdb) b main              # 简写
(gdb) b file.c:10         # 在文件第10行
(gdb) b function if x==5  # 条件断点

# 查看断点
(gdb) info breakpoints
(gdb) i b

# 删除断点
(gdb) delete 1            # 删除断点1
(gdb) clear main          # 清除函数断点

# 单步执行
(gdb) step                # 进入函数
(gdb) s
(gdb) next                # 跳过函数
(gdb) n
(gdb) finish              # 执行到函数返回

# 继续执行
(gdb) continue
(gdb) c

# 打印变量
(gdb) print variable
(gdb) p variable
(gdb) p *pointer          # 解引用指针
(gdb) p array[0]@10       # 打印数组前10个元素

# 查看变量类型
(gdb) whatis variable
(gdb) ptype variable

# 修改变量
(gdb) set variable x = 10

# 查看调用栈
(gdb) backtrace
(gdb) bt
(gdb) bt full             # 显示局部变量

# 切换栈帧
(gdb) frame 0
(gdb) f 0
(gdb) up                  # 上一个栈帧
(gdb) down                # 下一个栈帧

# 查看源码
(gdb) list
(gdb) l
(gdb) list main
(gdb) list 10             # 第10行附近

# 查看内存
(gdb) x/10x 0x12345678    # 16进制显示10个字
(gdb) x/s string_ptr      # 字符串
(gdb) x/10i $pc           # 反汇编10条指令

# 监视点
(gdb) watch variable      # 变量改变时停止
(gdb) rwatch variable     # 读取时停止
(gdb) awatch variable     # 读写都停止

# 查看线程
(gdb) info threads
(gdb) thread 2            # 切换到线程2
```

### GDB调试示例

```c
// bug.c
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int num = 5;
    int result = factorial(num);
    printf("Factorial of %d is %d\n", num, result);
    return 0;
}
```

```bash
# 编译
gcc -g bug.c -o bug

# 启动GDB
gdb ./bug

# GDB会话
(gdb) break main          # 在main设置断点
(gdb) run                 # 运行
(gdb) next                # 单步执行
(gdb) step                # 进入factorial
(gdb) print n             # 打印n
(gdb) continue            # 继续执行
(gdb) quit                # 退出
```

### TUI模式（文本界面）

```bash
gdb -tui ./program

# TUI命令
Ctrl+X A    # 切换TUI模式
Ctrl+X 2    # 切换窗口布局
Ctrl+L      # 刷新屏幕
Ctrl+P      # 上一条命令
Ctrl+N      # 下一条命令
```

### GDB配置文件

创建 `~/.gdbinit`:
```gdb
# 启用历史
set history save on
set history size 10000
set history filename ~/.gdb_history

# 美化输出
set print pretty on
set print array on
set print array-indexes on

# 自动加载
set auto-load safe-path /
```

---

## Git - 版本控制

### 安装配置

```bash
# 安装
sudo pacman -S git

# 配置
git config --global user.name "Your Name"
git config --global user.email "email@example.com"

# 查看配置
git config --list
```

### 基本命令

```bash
# 初始化仓库
git init

# 克隆仓库
git clone https://github.com/user/repo.git
git clone git@github.com:user/repo.git  # SSH

# 查看状态
git status

# 添加文件
git add file.txt
git add .                # 所有文件
git add -A               # 所有变更
git add -p               # 交互式添加

# 提交
git commit -m "Commit message"
git commit -am "Message"  # 添加并提交已跟踪文件

# 查看历史
git log
git log --oneline        # 简洁模式
git log --graph          # 图形化
git log -p               # 显示差异
git log --author="name"  # 按作者过滤

# 查看差异
git diff                 # 工作目录 vs 暂存区
git diff --staged        # 暂存区 vs 最后提交
git diff HEAD            # 工作目录 vs 最后提交
git diff commit1 commit2 # 两个提交之间

# 分支操作
git branch               # 列出分支
git branch newbranch     # 创建分支
git checkout newbranch   # 切换分支
git checkout -b newbranch # 创建并切换
git switch newbranch     # 新语法切换
git branch -d branchname # 删除分支
git branch -D branchname # 强制删除

# 合并
git merge branchname
git merge --no-ff branchname  # 保留分支历史

# 远程仓库
git remote -v            # 查看远程仓库
git remote add origin url # 添加远程仓库
git push origin main     # 推送到远程
git pull origin main     # 拉取并合并
git fetch origin         # 只拉取不合并

# 标签
git tag v1.0             # 轻量标签
git tag -a v1.0 -m "Version 1.0"  # 附注标签
git push origin v1.0     # 推送标签
git push origin --tags   # 推送所有标签

# 撤销操作
git checkout -- file.txt # 撤销工作目录修改
git restore file.txt     # 新语法
git reset HEAD file.txt  # 撤销暂存
git restore --staged file.txt  # 新语法
git revert commit_hash   # 撤销提交（新建提交）
git reset --soft HEAD~1  # 撤销最后提交（保留更改）
git reset --hard HEAD~1  # 撤销最后提交（丢弃更改）

# 储藏
git stash                # 储藏当前更改
git stash list           # 查看储藏列表
git stash pop            # 应用并删除储藏
git stash apply          # 应用储藏
git stash drop           # 删除储藏
```

### .gitignore 示例

```gitignore
# 编译产物
*.o
*.a
*.so
*.exe

# 编辑器
.vscode/
.idea/
*.swp
*~

# 构建目录
build/
dist/
obj/

# 依赖
node_modules/
__pycache__/

# 日志
*.log

# 系统文件
.DS_Store
Thumbs.db

# 环境文件
.env
.env.local

# 特定文件
config/secrets.yml

# 但包含某些文件
!important.log
```

### Git别名

添加到 `~/.gitconfig`:
```ini
[alias]
    st = status
    co = checkout
    br = branch
    ci = commit
    unstage = reset HEAD --
    last = log -1 HEAD
    visual = log --graph --oneline --all
    amend = commit --amend --no-edit
```

---

## 其他开发工具

### CMake

```bash
# 安装
sudo pacman -S cmake

# 基本CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(myapp main.cpp utils.cpp)

# 使用
mkdir build
cd build
cmake ..
make
```

### Valgrind - 内存检查

```bash
# 安装
sudo pacman -S valgrind

# 检查内存泄漏
valgrind --leak-check=full ./program

# 检查缓存性能
valgrind --tool=cachegrind ./program
```

### strace - 系统调用跟踪

```bash
# 跟踪程序
strace ./program

# 跟踪系统调用
strace -e open,read,write ./program

# 跟踪运行中的进程
strace -p PID
```

### ltrace - 库调用跟踪

```bash
ltrace ./program
```

---

## 相关资源

- [GCC手册](https://gcc.gnu.org/onlinedocs/)
- [GDB教程](https://sourceware.org/gdb/documentation/)
- [Git文档](https://git-scm.com/doc)
- [Pro Git 中文版](https://git-scm.com/book/zh/v2)
- Arch Wiki: https://wiki.archlinux.org/title/Core_utilities
