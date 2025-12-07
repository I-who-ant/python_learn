# GCC/CC 编译选项与工具使用指南

## 目录
1. [基础编译](#基础编译)
2. [关键编译选项](#关键编译选项)
3. [调试和分析工具](#调试和分析工具)
4. [动态库相关编译](#动态库相关编译)

---

## 基础编译

### 最简单的编译

```bash
# 编译单个源文件
cc -o main main.c

# 等同于
gcc -o main main.c
```

**说明**：
- `-o main`: 指定输出文件名为 `main`
- `main.c`: 源文件

### 编译并立即运行

```bash
cc -o main main.c && ./main
```

**说明**：
- `&&`: 只有编译成功才会执行后面的命令
- `./main`: 运行编译出的程序

---

## 关键编译选项

### 1. -ldl：链接动态链接库

```bash
cc -o main main.c -ldl
```

**用途**：
- 链接 `libdl.so` 库
- 使用 `<dlfcn.h>` 中的函数（`dlopen`、`dlsym` 等）时必须添加

**不加 -ldl 的后果**：
```
undefined reference to `dlopen'
undefined reference to `dlsym'
undefined reference to `dlclose'
```

### 示例

```c
#include <dlfcn.h>

int main() {
    void *lib = dlopen(NULL, RTLD_NOW);  // 需要 -ldl
    return 0;
}
```

---

### 2. -rdynamic：导出符号到动态符号表

```bash
cc -rdynamic -o main main.c
```

**用途**：
- 将**主程序中的符号**（包括函数和全局变量）导出到动态符号表
- 允许通过 `dlsym` 查询主程序中的符号

**对比**：

| 选项 | 能否查询 main? | 能否查询自定义函数? |
|------|---------------|-------------------|
| 无   | ❌ 不能       | ❌ 不能           |
| -rdynamic | ✅ 能 | ✅ 能 |

### 示例对比

**不使用 -rdynamic**：
```c
void *main_lib = dlsym(lib, "main");
printf("%p\n", main_lib);  // 输出: (nil)
```

**使用 -rdynamic**：
```c
void *main_lib = dlsym(lib, "main");
printf("%p\n", main_lib);  // 输出: 0x5555555551a9 (有效地址)
```

---

### 3. -fPIC：生成位置无关代码

```bash
cc -fPIC -c mylib.c
```

**用途**：
- 生成位置无关代码（Position Independent Code）
- **创建共享库时必须使用**
- 允许库加载到内存的任意位置

### 创建共享库完整流程

```bash
# 1. 编译为位置无关的目标文件
cc -fPIC -c mylib.c -o mylib.o

# 2. 链接为共享库
cc -shared -o libmylib.so mylib.o

# 3. 使用共享库
cc -o main main.c -L. -lmylib
```

---

### 4. -shared：创建共享库

```bash
cc -shared -o libfoo.so foo.o
```

**用途**：
- 将目标文件链接为共享库（.so 文件）
- 通常与 `-fPIC` 一起使用

### 完整示例

**mylib.c**：

```c
#include <stdio.h>

void hello() {
    printf("Hello from shared library!\n");
}
```

**编译**：

```bash
# 方法1：分步编译
cc -fPIC -c mylib.c
cc -shared -o libmylib.so mylib.o

# 方法2：一步到位
cc -fPIC -shared -o libmylib.so mylib.c
```

**使用**：
```bash
# 静态链接方式
cc -o main main.c -L. -lmylib

# 动态加载方式
cc -o main main.c -ldl
# 在代码中使用 dlopen("./libmylib.so", RTLD_NOW)
```

---

### 5. -g：生成调试信息

```bash
cc -g -o main main.c
```

**用途**：
- 在可执行文件中包含调试符号
- 允许使用 `gdb` 调试器

### 调试示例

```bash
# 编译时加上 -g
cc -g -o main main.c

# 使用 gdb 调试
gdb ./main
```

---

### 6. -O 优化选项

```bash
# 不优化（默认，适合调试）
cc -o main main.c

# 优化级别 1（基础优化）
cc -O1 -o main main.c

# 优化级别 2（推荐）
cc -O2 -o main main.c

# 优化级别 3（激进优化）
cc -O3 -o main main.c

# 针对大小优化
cc -Os -o main main.c
```

**选择建议**：
- **开发/调试**: 不加优化或 `-O0`
- **发布版本**: `-O2` 或 `-O3`
- **嵌入式**: `-Os` 优化二进制大小

---

### 7. -Wall -Wextra：警告选项

```bash
cc -Wall -Wextra -o main main.c
```

**用途**：
- `-Wall`: 开启大部分警告
- `-Wextra`: 开启额外的警告
- **强烈推荐始终使用**

### 示例

```c
int main() {
    int x;  // 未初始化
    printf("%d\n", x);  // -Wall 会警告使用未初始化变量
}
```

---

### 8. -D 定义宏

```bash
cc -DDEBUG -o main main.c
```

**用途**：
- 在编译时定义预处理器宏
- 相当于在代码中写 `#define DEBUG`

### 示例

```c
#ifdef DEBUG
    printf("Debug mode enabled\n");
#endif
```

---

## 调试和分析工具

### nm：查看符号表

```bash
# 查看所有符号
nm ./main

# 查看动态符号（导出的符号）
nm -D ./main

# 只看函数（T = text section）
nm -D ./main | grep " T "

# 只看数据符号
nm -D ./main | grep " D "
```

**输出示例**：
```
0000000000001149 T main
                 U dlopen
                 U printf
```

**符号类型**：
- `T`: 文本段中的符号（函数）
- `D`: 已初始化的数据段
- `B`: 未初始化的数据段（BSS）
- `U`: 未定义的符号（需要从其他库链接）
- `W`: 弱符号

---

### readelf：读取 ELF 信息

```bash
# 查看符号表
readelf -s ./main

# 查看程序头
readelf -l ./main

# 查看节（section）头
readelf -S ./main

# 查看动态节
readelf -d ./main
```

**示例：查看导出的函数**：
```bash
readelf -s ./main | grep FUNC
```

---

### ldd：查看动态库依赖

```bash
ldd ./main
```

**输出示例**：
```
linux-vdso.so.1 (0x00007ffed53f9000)
libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f9a8c3f5000)
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f9a8c203000)
/lib64/ld-linux-x86-64.so.2 (0x00007f9a8c5fb000)
```

**用途**：
- 查看程序运行时需要哪些动态库
- 检查库是否能够找到
- 调试 "cannot find library" 错误

---

### objdump：反汇编

```bash
# 反汇编代码段
objdump -d ./main

# 查看所有段
objdump -x ./main

# 只看特定函数
objdump -d ./main | grep -A 20 "<main>:"
```

---

### LD_DEBUG：调试动态链接器

```bash
# 查看库加载过程
LD_DEBUG=libs ./main

# 查看符号查找过程
LD_DEBUG=symbols ./main

# 查看所有调试信息
LD_DEBUG=all ./main
```

**用途**：
- 调试动态库加载问题
- 了解符号解析过程
- 追踪运行时链接行为

---

## 动态库相关编译

### 完整的库创建和使用流程

#### 1. 创建共享库

**mylib.h**：
```c
#ifndef MYLIB_H
#define MYLIB_H

void hello(void);
int add(int a, int b);

#endif
```

**mylib.c**：
```c
#include <stdio.h>
#include "mylib.h"

void hello(void) {
    printf("Hello from mylib!\n");
}

int add(int a, int b) {
    return a + b;
}
```

**编译为共享库**：
```bash
cc -fPIC -shared -o libmylib.so mylib.c
```

---

#### 2. 方式A：编译时链接

**main.c**：
```c
#include "mylib.h"

int main() {
    hello();
    return 0;
}
```

**编译**：
```bash
# 链接到当前目录的库
cc -o main main.c -L. -lmylib

# 运行（需要设置 LD_LIBRARY_PATH）
LD_LIBRARY_PATH=. ./main
```

---

#### 3. 方式B：运行时动态加载

**main.c**：
```c
#include <stdio.h>
#include <dlfcn.h>

int main() {
    void *lib = dlopen("./libmylib.so", RTLD_NOW);
    if (!lib) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    void (*hello)(void) = dlsym(lib, "hello");
    if (hello) {
        hello();
    }

    dlclose(lib);
    return 0;
}
```

**编译**：
```bash
cc -o main main.c -ldl

# 运行（无需 LD_LIBRARY_PATH）
./main
```

---

## 常用编译组合

### 开发模式（带调试，带警告）

```bash
cc -g -Wall -Wextra -o main main.c -ldl
```

### 发布模式（优化）

```bash
cc -O2 -o main main.c -ldl
```

### 创建共享库（标准流程）

```bash
cc -fPIC -shared -o libfoo.so foo.c
```

### 使用动态链接（完整）

```bash
cc -rdynamic -o main main.c -ldl
```

---

## 环境变量

### LD_LIBRARY_PATH

```bash
# 临时设置（仅当前命令）
LD_LIBRARY_PATH=/path/to/libs ./main

# 导出到环境
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH
./main
```

### LD_PRELOAD

```bash
# 强制加载特定库（可用于 hook）
LD_PRELOAD=/path/to/mylib.so ./main
```

---

## 常见问题和解决方案

### 问题 1：undefined reference to dlopen

**错误**：
```
undefined reference to `dlopen'
undefined reference to `dlsym'
```

**解决**：
```bash
# 添加 -ldl
cc -o main main.c -ldl
```

---

### 问题 2：cannot open shared object file

**错误**：
```
./main: error while loading shared libraries: libfoo.so: cannot open shared object file
```

**解决方案 1**：使用 `LD_LIBRARY_PATH`
```bash
LD_LIBRARY_PATH=. ./main
```

**解决方案 2**：安装到系统目录
```bash
sudo cp libfoo.so /usr/local/lib/
sudo ldconfig
```

**解决方案 3**：使用 RPATH（编译时指定）
```bash
cc -o main main.c -L. -lmylib -Wl,-rpath,'$ORIGIN'
```

---

### 问题 3：dlsym 返回 NULL

**原因**：符号未导出

**解决方案**：
1. 编译时使用 `-rdynamic`（对于主程序）
2. 检查符号是否真的存在：`nm -D ./program`
3. 确保函数不是 `static` 的

---

### 问题 4：位置无关代码错误

**错误**：
```
relocation R_X86_64_32 against `.rodata' can not be used when making a shared object
```

**解决**：
```bash
# 添加 -fPIC
cc -fPIC -shared -o libfoo.so foo.c
```

---

## 快速参考表

| 选项 | 用途 | 示例 |
|------|------|------|
| `-o` | 指定输出文件名 | `cc -o main main.c` |
| `-ldl` | 链接 libdl.so | `cc main.c -ldl` |
| `-rdynamic` | 导出符号 | `cc -rdynamic main.c` |
| `-fPIC` | 位置无关代码 | `cc -fPIC -c foo.c` |
| `-shared` | 创建共享库 | `cc -shared -o libfoo.so foo.o` |
| `-g` | 调试信息 | `cc -g main.c` |
| `-O2` | 优化 | `cc -O2 main.c` |
| `-Wall` | 警告 | `cc -Wall main.c` |
| `-D` | 定义宏 | `cc -DDEBUG main.c` |
| `-L` | 库搜索路径 | `cc main.c -L/path -lfoo` |
| `-l` | 链接库 | `cc main.c -lm` |
| `-I` | 头文件路径 | `cc -I/path/include main.c` |

---

## 工具速查表

| 工具 | 用途 | 命令 |
|------|------|------|
| `nm` | 查看符号表 | `nm -D ./main` |
| `readelf` | 读取 ELF 信息 | `readelf -s ./main` |
| `ldd` | 查看库依赖 | `ldd ./main` |
| `objdump` | 反汇编 | `objdump -d ./main` |
| `file` | 查看文件类型 | `file ./main` |
| `strings` | 提取字符串 | `strings ./main` |
| `strip` | 去除符号 | `strip ./main` |
