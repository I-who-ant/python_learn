# nob.c - 构建系统分析

## 什么是 nob？

**nob** = "Nobody's Build System"，一个单头文件的 C 构建工具，类似 Make/CMake，但用 C 代码编写构建逻辑。

## 核心理念

用 C 代码本身作为构建脚本，而不是学习新的 DSL（领域特定语言）。

## nob.c 代码逐行分析

### 1. 头文件引入（1-3行）
```c
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
```

**解释**：
- `NOB_IMPLEMENTATION` - 告诉 nob.h 展开实现代码（单头文件库的常见模式）
- `NOB_STRIP_PREFIX` - 去掉函数前缀，`nob_cmd_append` → `cmd_append`
- `nob.h` - 包含所有构建工具函数（命令执行、文件操作等）

**类比**：类似 stb 系列库的使用方式（stb_image.h, stb_c_lexer.h）

### 2. 全局命令对象（5行）
```c
Cmd cmd = {0};
```

**解释**：
- `Cmd` 是 nob.h 定义的结构体，用于构建命令行参数
- 零初始化，准备存储编译器命令

**等价于**：
```c
typedef struct {
    char **items;    // 参数数组
    size_t count;    // 参数数量
    size_t capacity; // 容量
} Cmd;
```

### 3. 编译器配置函数（7-14行）
```c
void cc(void)
{
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-Wno-unused-function");
    cmd_append(&cmd, "-ggdb");
}
```

**解释**：
- 封装通用编译器选项
- `-Wall -Wextra` - 启用所有警告
- `-Wno-unused-function` - 忽略未使用函数警告（nob.h 有很多工具函数）
- `-ggdb` - 生成 GDB 调试信息

**执行后 cmd 内容**：
```
["cc", "-Wall", "-Wextra", "-Wno-unused-function", "-ggdb"]
```

### 4. 主构建逻辑（16-27行）

#### 4.1 自动重建（18行）
```c
NOB_GO_REBUILD_URSELF(argc, argv);
```

**解释**：
- 检查 `nob.c` 是否比 `nob` 可执行文件新
- 如果是，自动重新编译 `nob` 并重新执行
- 确保构建工具始终是最新版本

**等价逻辑**：
```c
if (is_file_newer("nob.c", "nob")) {
    system("cc -o nob nob.c");
    execvp("./nob", argv);  // 重新执行自己
}
```

#### 4.2 构建 crepl（20-24行）
```c
cc();
cmd_append(&cmd, "-o", "crepl");
cmd_append(&cmd, "crepl.c");
cmd_append(&cmd, "-lffi");
if (!cmd_run(&cmd)) return 1;
```

**解释**：
1. `cc()` - 添加基础编译选项
2. `-o crepl` - 输出文件名
3. `crepl.c` - 源文件
4. `-lffi` - 链接 libffi 库
5. `cmd_run(&cmd)` - 执行命令

**最终执行的命令**：
```bash
cc -Wall -Wextra -Wno-unused-function -ggdb -o crepl crepl.c -lffi
```

## 关于 raylib 动态链接库

**注意**：这个 `nob.c` 并没有构建 raylib，它只是编译 `crepl`。

raylib 的动态链接库（`libraylib.so`）已经预编译好，存放在：
```
raylib-5.5_linux_amd64/lib/libraylib.so
```

`crepl` 通过 `dlopen` 在运行时加载这个库：
```bash
./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so
```

## nob 的优势

### 对比 Makefile
```makefile
# Makefile 版本
CFLAGS = -Wall -Wextra -ggdb
crepl: crepl.c
    cc $(CFLAGS) -o crepl crepl.c -lffi
```

**nob 的优势**：
- 用 C 语言编写，无需学习 Make 语法
- 可以使用完整的 C 语言特性（条件、循环、函数）
- 跨平台（Make 在 Windows 上需要额外工具）
- 自动重建自己

### 对比 CMake
```cmake
# CMakeLists.txt 版本
cmake_minimum_required(VERSION 3.10)
project(crepl)
add_executable(crepl crepl.c)
target_link_libraries(crepl ffi)
```

**nob 的优势**：
- 零依赖（只需要 C 编译器）
- 单文件（nob.h + nob.c）
- 直接运行，无需生成中间文件

## 使用流程

```bash
# 1. 编译构建工具
cc -o nob nob.c

# 2. 运行构建
./nob

# 3. 使用生成的程序
./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so
```

## 核心 API（来自 nob.h）

```c
// 命令构建
void cmd_append(Cmd *cmd, ...);           // 添加参数
bool cmd_run(Cmd *cmd);                   // 执行命令

// 文件操作
bool is_file_newer(const char *a, const char *b);
bool file_exists(const char *path);

// 路径操作
void path_join(String_Builder *sb, ...);  // 拼接路径

// 进程执行
bool exec(const char *program, ...);      // 执行外部程序
```





## 总结

`nob.c` 是一个**元构建系统**：
1. 它本身是 C 程序
2. 它编译其他 C 程序（crepl）
3. 它可以自动重建自己

**核心思想**：用你已经会的语言（C）来编写构建脚本，而不是学习新工具。











