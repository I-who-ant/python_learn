# C 语言编译完全指南 - 从源码到可执行文件

## 📚 目录

1. [什么是编译?](#什么是编译)
2. [cc 和 gcc 的关系](#cc-和-gcc-的关系)
3. [编译的四个阶段](#编译的四个阶段)
4. [常用编译器选项](#常用编译器选项)
5. [crepl 项目编译实战](#crepl-项目编译实战)
6. [常见错误排查](#常见错误排查)

---

## 什么是编译?

### 核心概念

**编译**是将人类可读的源代码(.c 文件)转换为计算机可执行的机器码的过程。

```
源代码(文本文件)          可执行文件(二进制)
   ↓                         ↓
main.c        →  [编译器]  →  main (可直接运行)
(你可以阅读)                 (计算机可以执行)
```

### 类比理解

```
源代码 = 食谱(人类语言)
  ↓
编译器 = 厨师(翻译和加工)
  ↓
可执行文件 = 成品菜(可直接食用)
```

你不能直接"吃"食谱,必须通过厨师的加工才能得到食物。
同样,你不能直接"运行"源代码,必须通过编译器编译才能得到可执行程序。

---

## cc 和 gcc 的关系

### 简短答案

**cc 不是 gcc 的简写,但在大多数 Linux 系统上,cc 是指向 gcc 的符号链接。**

### 详细解释

#### 历史背景

- **cc** = C Compiler(C 编译器的通用名称)
  - 最早由 Unix 系统使用
  - 是一个**标准名称**,不是特定的编译器

- **gcc** = GNU Compiler Collection(GNU 编译器套件)
  - GNU 项目开发的编译器
  - 支持 C, C++, Objective-C, Fortran 等多种语言
  - 最流行的开源 C 编译器

#### 在 Linux 上的关系

在大多数 Linux 发行版上:

```bash
# cc 是指向 gcc 的符号链接
$ ls -l /usr/bin/cc
lrwxrwxrwx 1 root root 3 Jan 10 2024 /usr/bin/cc -> gcc

# 验证版本(输出相同)
$ cc --version
gcc (GCC) 13.2.1
...

$ gcc --version
gcc (GCC) 13.2.1
...
```

#### 使用建议

```bash
# 以下两个命令在大多数系统上是等价的:
cc -o program program.c
gcc -o program program.c

# 建议:
# - 写脚本时使用 cc (更通用,跨平台)
# - 需要 gcc 特定功能时使用 gcc (明确依赖)
```

---

## 编译的四个阶段

### 完整流程

```
源代码(.c)
    ↓
[1. 预处理] → 处理 #include, #define 等
    ↓
预处理后的代码(.i)
    ↓
[2. 编译] → 转换为汇编代码
    ↓
汇编代码(.s)
    ↓
[3. 汇编] → 转换为机器码
    ↓
目标文件(.o)
    ↓
[4. 链接] → 合并多个目标文件,链接库
    ↓
可执行文件(无扩展名)
```

### 逐步演示

创建一个简单的 `hello.c`:

```c
#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

#### 阶段 1: 预处理(Preprocessing)

```bash
# 生成 hello.i (预处理后的代码)
$ cc -E hello.c -o hello.i

# 查看结果(会展开所有 #include)
$ head -n 20 hello.i
# 1 "hello.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# ... (stdio.h 的所有内容,约 800 行)
```

**作用**:
- 展开 `#include` 文件
- 替换 `#define` 宏
- 处理条件编译 `#ifdef`
- 删除注释

#### 阶段 2: 编译(Compilation)

```bash
# 生成 hello.s (汇编代码)
$ cc -S hello.c -o hello.s

# 查看结果
$ cat hello.s
	.file	"hello.c"
	.text
	.section	.rodata
.LC0:
	.string	"Hello, World!"
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movl	$0, %eax
	popq	%rbp
	ret
```

**作用**:
- 将 C 代码转换为汇编语言
- 进行语法检查
- 优化代码

#### 阶段 3: 汇编(Assembly)

```bash
# 生成 hello.o (目标文件,二进制)
$ cc -c hello.c -o hello.o

# 查看文件类型
$ file hello.o
hello.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped

# 无法直接阅读(二进制格式)
$ cat hello.o
^?ELF^B^A^A^@^@^@^@^@^@... (乱码)
```

**作用**:
- 将汇编代码转换为机器码
- 生成可重定位目标文件(.o)

#### 阶段 4: 链接(Linking)

```bash
# 生成可执行文件 hello
$ cc hello.o -o hello

# 查看文件类型
$ file hello
hello: ELF 64-bit LSB executable, x86-64, dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2

# 运行程序
$ ./hello
Hello, World!
```

**作用**:
- 合并多个 `.o` 文件
- 链接外部库(如 libc.so 中的 `printf`)
- 解析符号引用
- 生成最终可执行文件

### 一步到位

通常我们跳过中间步骤,直接生成可执行文件:

```bash
# 直接从 .c 生成可执行文件(包含所有阶段)
$ cc hello.c -o hello
$ ./hello
Hello, World!
```

#### 参数顺序灵活性 🔄

**好消息:gcc/cc 的参数顺序非常灵活!**

以下所有写法都是**正确且等价**的:

```bash
# 1. 标准顺序(推荐,最易读)
cc test.c -o test

# 2. -o 在最前面
cc -o test test.c

# 3. 选项散布在各处
cc -Wall test.c -ggdb -o test -Wextra

# 4. -o 和文件名连在一起(无空格)
cc test.c -otest

# 5. -o 和文件名分开(有空格)
cc test.c -o test
```

**规则总结**:

| 参数类型 | 顺序要求 | 示例 |
|---------|---------|------|
| **源文件** (.c) | 可以在任何位置 | `cc a.c b.c -o prog` 或 `cc -o prog a.c b.c` |
| **-o 选项** | 可以在任何位置 | `cc -o prog test.c` 或 `cc test.c -o prog` |
| **-o 和文件名** | 可连写或分开 | `-o prog` 或 `-oprog` |
| **其他选项** | 可以在任何位置 | `-Wall -ggdb -Wextra` 顺序任意 |

**⚠️ 唯一的规则**:

带参数的选项(如 `-o`, `-l`, `-I`, `-L`)必须和它的参数**成对出现**:

```bash
# ✅ 正确:-o 后面紧跟输出文件名
cc test.c -o program

# ✅ 正确:-l 后面紧跟库名
cc test.c -o program -lm -lffi

# ❌ 错误:-o 后面没有文件名
cc test.c -o -Wall program
#          ↑ cc 会认为 -Wall 是输出文件名!

# ❌ 错误:-l 后面没有库名
cc test.c -l -o program
#          ↑ cc 会报错:missing argument
```

**最佳实践**:

虽然顺序灵活,但为了代码可读性,建议:

```bash
# 推荐顺序:编译器 + 警告选项 + 调试选项 + 输出 + 源文件 + 库
cc -Wall -Wextra -ggdb -o program main.c utils.c -lm -lffi

# 或者:源文件在前(更常见)
cc main.c utils.c -Wall -Wextra -ggdb -o program -lm -lffi
```

---

## 常用编译器选项

### 1. `-o <文件名>` - 指定输出文件名

```bash
# 默认输出文件名为 a.out
$ cc hello.c
$ ./a.out        # 运行

# 使用 -o 指定输出文件名
$ cc hello.c -o hello
$ ./hello        # 运行

# 解释:
# -o  : output(输出)的缩写
# hello : 输出文件名
```

**示例对比**:

```bash
# 没有 -o
$ cc hello.c       → 生成 a.out
$ cc main.c        → 生成 a.out (覆盖之前的!)

# 有 -o
$ cc hello.c -o hello  → 生成 hello
$ cc main.c -o main    → 生成 main (不会冲突)
```

### 2. `-c` - 只编译不链接(生成 .o 文件)

```bash
# 只编译,生成目标文件
$ cc -c file1.c       → 生成 file1.o
$ cc -c file2.c       → 生成 file2.o

# 后续手动链接
$ cc file1.o file2.o -o program
```

**使用场景**: 大型项目分模块编译,避免重复编译未修改的文件。

### 3. `-Wall -Wextra` - 启用警告

```bash
# -Wall  : 启用常见警告
# -Wextra: 启用额外警告
$ cc -Wall -Wextra hello.c -o hello
```

**示例**:

```c
// test.c
int main() {
    int x;           // 未初始化
    printf("%d", x); // 未使用返回值检查
    // 没有 return 语句
}
```

```bash
# 不加警告选项(编译通过,但有隐患)
$ cc test.c -o test

# 加警告选项(会提示问题)
$ cc -Wall -Wextra test.c -o test
test.c:3:5: warning: 'x' is used uninitialized [-Wuninitialized]
test.c:5:1: warning: control reaches end of non-void function [-Wreturn-type]
```

### 4. `-ggdb` - 生成调试信息

```bash
# 包含调试符号(用于 gdb 调试器)
$ cc -ggdb hello.c -o hello

# 使用 gdb 调试
$ gdb ./hello
(gdb) break main
(gdb) run
(gdb) print x
```

### 5. `-l<库名>` - 链接外部库

```bash
# 链接数学库(libm.so)
$ cc math_program.c -o math_program -lm

# 链接 ffi 库(libffi.so)
$ cc crepl.c -o crepl -lffi

# 解释:
# -lm    → 链接 libm.so (m = math)
# -lffi  → 链接 libffi.so
# -lpthread → 链接 libpthread.so (多线程)
```

**命名规则**:

```
库文件名: libffi.so
          ↑   ↑   ↑
          |   |   └─ 扩展名(.so = shared object)
          |   └───── 库名(ffi)
          └───────── 固定前缀(lib)

使用时: -lffi (去掉 lib 前缀和 .so 后缀)
```

### 6. `-I<目录>` - 指定头文件搜索路径

```bash
# 在 /usr/local/include 搜索头文件
$ cc program.c -I/usr/local/include

# 多个路径
$ cc program.c -I/path1 -I/path2
```

### 7. `-L<目录>` - 指定库文件搜索路径

```bash
# 在 /usr/local/lib 搜索库文件
$ cc program.c -L/usr/local/lib -lffi

# 示例:编译 crepl 时查找 libffi.so
$ cc crepl.c -L/usr/lib/x86_64-linux-gnu -lffi -o crepl
```

---

## crepl 项目编译实战

### 项目结构

```
crepl-main/
├── nob.c              # 构建脚本(源代码)
├── nob.h              # 构建工具库
├── crepl.c            # REPL 主程序(源代码)
├── stb_c_lexer.h      # 词法分析器(头文件)
└── raylib-5.5_linux_amd64/
    └── lib/
        └── libraylib.so  # Raylib 库
```

### 错误示例(你遇到的问题)

```bash
# ❌ 错误:尝试执行源代码文件
$ ./nob.c && crepl.c raylib-5.5_linux_amd64/lib/libraylib.so

# 错误分析:
# 1. ./nob.c
#    - nob.c 是文本文件(源代码)
#    - Shell 尝试将其作为脚本执行
#    - 失败原因:第一行是 "// ...",Shell 不理解 C 语法
#
# 2. crepl.c raylib...
#    - crepl.c 也是文本文件
#    - Shell 尝试将其作为命令执行
#    - 失败原因:crepl.c 不是可执行文件

错误信息:
./nob.c: 行 1: //: 是一个目录
./nob.c: 行 2: 未预期的记号 "(" 附近有语法错误
```

### 正确编译步骤

#### 第 1 步:编译构建工具 nob

```bash
# 编译 nob.c → 生成可执行文件 nob
$ cc -o nob nob.c

# 解释:
# cc       : C 编译器
# -o nob   : 输出文件名为 "nob"
# nob.c    : 源文件

# 现在目录中多了一个文件:
$ ls -lh nob
-rwxr-xr-x 1 user user 50K Jan 10 10:00 nob
#   ↑
#   x = 可执行(executable)
```

**验证可执行权限**:

```bash
# 查看文件类型
$ file nob
nob: ELF 64-bit LSB executable, x86-64, dynamically linked

# 查看权限
$ ls -l nob
-rwxr-xr-x  # rwx = 可读、可写、可执行
```

#### 第 2 步:运行 nob 编译 crepl

```bash
# 运行 nob(注意有 ./ 前缀)
$ ./nob

# nob 内部执行的命令(自动):
# cc -Wall -Wextra -ggdb -o crepl crepl.c -lffi

# 输出:
CMD: cc -Wall -Wextra -ggdb -o crepl crepl.c -lffi
[INFO] Compilation successful

# 现在目录中多了 crepl 可执行文件:
$ ls -lh crepl
-rwxr-xr-x 1 user user 75K Jan 10 10:01 crepl
```

**nob 的作用**:

```c
// nob.c 内部逻辑(简化版)
int main(int argc, char **argv) {
    // 1. 检查自己是否需要重新编译
    NOB_GO_REBUILD_URSELF(argc, argv);

    // 2. 编译 crepl
    cc();
    cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-o", "crepl");
    cmd_append(&cmd, "crepl.c");
    cmd_append(&cmd, "-lffi");  // 链接 libffi 库
    cmd_run(&cmd);              // 执行命令

    return 0;
}
```

#### 第 3 步:运行 crepl

```bash
# 运行 crepl,加载 raylib 动态库
$ ./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so

# 交互式调用函数
> InitWindow 800 600 "Hello"
> BeginDrawing
> ClearBackground 0xFF000000
> DrawRectangle 10 10 100 100 0xFF0000FF
> EndDrawing
> ^D  (按 Ctrl+D 退出)
Quit
```

### 完整流程总结

```bash
# 步骤 1: 编译构建工具
$ cc -o nob nob.c
  源代码: nob.c (文本)
       ↓ 编译
  可执行: nob (二进制)

# 步骤 2: 运行构建工具
$ ./nob
  执行: nob
       ↓ 内部调用 cc
  生成: crepl (二进制)

# 步骤 3: 运行主程序
$ ./crepl libraylib.so
  执行: crepl
       ↓ 加载动态库
  交互: REPL 循环
```

### 为什么需要 ./

```bash
# ❌ 错误:直接运行(Shell 找不到)
$ nob
bash: nob: command not found

# ✅ 正确:使用相对路径
$ ./nob
[OK] 运行成功

# 原因:
# - Shell 只在 PATH 环境变量中搜索命令
# - 当前目录(.)通常不在 PATH 中(安全原因)
# - ./ 明确告诉 Shell:"在当前目录查找"

# 查看 PATH
$ echo $PATH
/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
# ↑ 没有当前目录
```

---

## 常见错误排查

### 错误 1: command not found

```bash
$ nob
bash: nob: command not found
```

**原因**: 当前目录不在 PATH 中

**解决**:
```bash
# 方法 1: 使用 ./
$ ./nob

# 方法 2: 使用绝对路径
$ /home/user/crepl-main/nob

# 方法 3: 添加当前目录到 PATH(不推荐)
$ export PATH=".:$PATH"
$ nob  # 现在可以直接运行
```

### 错误 2: Permission denied

```bash
$ ./nob
bash: ./nob: Permission denied
```

**原因**: 文件没有可执行权限

**解决**:
```bash
# 查看权限
$ ls -l nob
-rw-r--r-- 1 user user 50K Jan 10 10:00 nob
# ↑ 没有 x (可执行)权限

# 添加可执行权限
$ chmod +x nob

# 再次查看
$ ls -l nob
-rwxr-xr-x 1 user user 50K Jan 10 10:00 nob
# ↑ 现在有 x 权限了

# 运行成功
$ ./nob
```

### 错误 3: undefined reference to 'xxx'

```bash
$ cc crepl.c -o crepl
/usr/bin/ld: /tmp/ccXXXX.o: undefined reference to 'ffi_prep_cif'
collect2: error: ld returned 1 exit status
```

**原因**: 缺少库链接

**解决**:
```bash
# 链接 libffi 库
$ cc crepl.c -o crepl -lffi
```

### 错误 4: cannot find -lffi

```bash
$ cc crepl.c -o crepl -lffi
/usr/bin/ld: cannot find -lffi: No such file or directory
```

**原因**: 系统未安装 libffi 开发包

**解决**:
```bash
# Arch Linux
$ sudo pacman -S libffi

# Ubuntu/Debian
$ sudo apt install libffi-dev

# Fedora
$ sudo dnf install libffi-devel
```

### 错误 5: No such file or directory (头文件)

```bash
$ cc crepl.c -o crepl -lffi
crepl.c:18:10: fatal error: ffi.h: No such file or directory
   18 | #include <ffi.h>
      |          ^~~~~~~
```

**原因**: 缺少头文件

**解决**:
```bash
# 方法 1: 安装开发包(推荐)
$ sudo pacman -S libffi

# 方法 2: 手动指定头文件路径
$ cc crepl.c -o crepl -I/usr/local/include -lffi
```

### 错误 6: 语法错误(尝试执行源代码)

```bash
$ ./crepl.c
./crepl.c: 行 1: //: 是一个目录
./crepl.c: 行 2: 未预期的记号 "(" 附近有语法错误
```

**原因**: 尝试执行源代码文件而非可执行文件

**解决**:
```bash
# 1. 先编译
$ cc crepl.c -o crepl -lffi

# 2. 再运行
$ ./crepl libraylib.so
```

---

## 快速参考卡片

### 编译命令模板

```bash
# 基础编译
cc 源文件.c -o 输出文件名

# 带警告和调试信息
cc -Wall -Wextra -ggdb 源文件.c -o 输出文件名

# 链接外部库
cc 源文件.c -o 输出文件名 -l库名

# 多个源文件
cc file1.c file2.c file3.c -o program

# 分步编译
cc -c file1.c         # → file1.o
cc -c file2.c         # → file2.o
cc file1.o file2.o -o program
```

### 文件类型速查

| 扩展名 | 类型 | 说明 | 可读? | 可执行? |
|--------|------|------|-------|---------|
| `.c` | 源代码 | C 语言源文件 | ✅ | ❌ |
| `.h` | 头文件 | 函数声明、宏定义 | ✅ | ❌ |
| `.i` | 预处理代码 | 展开后的源代码 | ✅ | ❌ |
| `.s` | 汇编代码 | 汇编语言 | ✅ | ❌ |
| `.o` | 目标文件 | 机器码(未链接) | ❌ | ❌ |
| 无扩展名 | 可执行文件 | 最终程序 | ❌ | ✅ |
| `.so` | 共享库 | 动态链接库 | ❌ | ⚠️ |

### 编译器选项速查

| 选项 | 作用 | 示例 |
|------|------|------|
| `-o` | 指定输出文件名 | `cc test.c -o test` |
| `-c` | 只编译不链接 | `cc -c file.c` |
| `-Wall` | 启用常见警告 | `cc -Wall test.c` |
| `-Wextra` | 启用额外警告 | `cc -Wextra test.c` |
| `-ggdb` | 生成调试信息 | `cc -ggdb test.c` |
| `-l<name>` | 链接库 | `cc test.c -lm` |
| `-I<dir>` | 头文件搜索路径 | `cc -I/usr/local/include` |
| `-L<dir>` | 库文件搜索路径 | `cc -L/usr/local/lib` |
| `-E` | 只预处理 | `cc -E test.c` |
| `-S` | 只编译到汇编 | `cc -S test.c` |

---

## 总结

1. **cc 和 gcc**: cc 是通用名称,通常指向 gcc
2. **编译流程**: 预处理 → 编译 → 汇编 → 链接
3. **核心选项**:
   - `-o` 指定输出
   - `-c` 只编译
   - `-l` 链接库
4. **源代码 vs 可执行文件**: .c 是文本,需要编译后才能运行
5. **crepl 项目**: 先编译 nob,再用 nob 编译 crepl

现在你应该能理解为什么需要:
```bash
cc -o nob nob.c     # 编译
./nob               # 运行
```

而不是:
```bash
./nob.c             # ❌ 错误!
```
