# nob.h 对于 nob.c 的作用

## 核心概念：单头文件库（Single-Header Library）

**nob.h** 是一个 **单头文件库**，包含了完整的构建工具实现（2616 行代码）。

## 文件结构

```
nob.h
├── 头部（1-785行）：函数声明、类型定义、宏定义
└── 实现（786-2616行）：函数实现（需要 #define NOB_IMPLEMENTATION）
```

## 对 nob.c 的作用

### 1. 提供构建工具 API

nob.c 中使用的所有函数都来自 nob.h：

```c
// nob.c 中使用的函数
shift()           // 弹出命令行参数
cmd_append()      // 添加命令参数
cmd_run()         // 执行命令
temp_alloc()      // 临时内存分配
temp_save()       // 保存内存标记
temp_rewind()     // 重置内存池
```

### 2. 单头文件库模式

**使用方式**：
```c
// nob.c
#define NOB_IMPLEMENTATION  // 展开函数实现
#define NOB_STRIP_PREFIX    // 去掉 nob_ 前缀
#include "nob.h"
```

**工作原理**：

#### 不定义 NOB_IMPLEMENTATION（默认）
```c
#include "nob.h"
// 只包含声明：
void nob_cmd_append(Nob_Cmd *cmd, ...);
bool nob_cmd_run(Nob_Cmd *cmd);
```

#### 定义 NOB_IMPLEMENTATION
```c
#define NOB_IMPLEMENTATION
#include "nob.h"
// 包含声明 + 实现：
void nob_cmd_append(Nob_Cmd *cmd, ...) {
    // 实际实现代码（786行之后）
}
```

**类比**：类似 stb_image.h、stb_c_lexer.h 的使用方式。

### 3. 提供的核心功能

#### 3.1 命令执行（Cmd）
```c
typedef struct {
    char **items;    // 命令参数数组
    size_t count;
    size_t capacity;
} Nob_Cmd;

void nob_cmd_append(Nob_Cmd *cmd, ...);  // 添加参数
bool nob_cmd_run(Nob_Cmd *cmd);          // 执行命令
```

**nob.c 中的使用**：
```c
Cmd cmd = {0};
cmd_append(&cmd, "cc", "-Wall", "-o", "crepl", "crepl.c", "-lffi");
cmd_run(&cmd);  // 执行：cc -Wall -o crepl crepl.c -lffi
```

#### 3.2 临时内存池（Temp Allocator）
```c
void *nob_temp_alloc(size_t size);      // 分配临时内存
size_t nob_temp_save(void);             // 保存当前位置
void nob_temp_rewind(size_t mark);      // 回退到标记位置
char *nob_temp_strdup(const char *s);   // 临时复制字符串
```

**工作原理**：
```
内存池：[8MB 固定大小缓冲区]
         ↑
       mark（标记位置）

temp_alloc()  → 向前移动指针
temp_rewind() → 回退到 mark，释放所有后续内存
```

**优势**：
- 无需手动 free
- 每次循环自动回收
- 避免内存碎片

**crepl.c 中的使用**：
```c
size_t mark = temp_save();
for (;;) {
    temp_rewind(mark);  // 每次循环重置内存
    int *x = temp_alloc(sizeof(int));  // 分配
    // 下次循环自动释放
}
```

#### 3.3 动态数组（Dynamic Array）
```c
#define da_append(da, item) /* 宏实现 */
```

**nob.c 中的使用**：
```c
Types args = {0};
da_append(&args, &ffi_type_sint32);  // 自动扩容
```

#### 3.4 文件操作
```c
bool nob_file_exists(const char *path);
bool nob_needs_rebuild(const char *output, const char **inputs, size_t count);
bool nob_copy_file(const char *src, const char *dst);
bool nob_mkdir_if_not_exists(const char *path);
```

#### 3.5 字符串工具
```c
typedef struct {
    const char *data;
    size_t count;
} String_View;

String_View sv_from_cstr(const char *s);
String_View sv_trim(String_View sv);
```

**crepl.c 中的使用**：
```c
String_View sv = sv_trim(sv_from_cstr(line));
```

#### 3.6 自动重建宏
```c
#define NOB_GO_REBUILD_URSELF(argc, argv) \
    nob__go_rebuild_urself(argc, argv, __FILE__, NULL)
```

**工作原理**：
```c
if (nob_needs_rebuild("nob", (const char*[]){"nob.c"}, 1)) {
    // nob.c 比 nob 新，重新编译
    system("cc -o nob nob.c");
    execvp("./nob", argv);  // 重新执行
}
```

### 4. NOB_STRIP_PREFIX 的作用

**不启用**（默认）：
```c
#include "nob.h"
nob_cmd_append(&cmd, "cc");  // 需要 nob_ 前缀
```

**启用**：
```c
#define NOB_STRIP_PREFIX
#include "nob.h"
cmd_append(&cmd, "cc");  // 去掉前缀
```

**实现原理**（预处理器宏）：
```c
#ifdef NOB_STRIP_PREFIX
#define cmd_append nob_cmd_append
#define cmd_run nob_cmd_run
#define temp_alloc nob_temp_alloc
// ...
#endif
```

**注意**：
- 只是宏替换，编译后的符号仍是 `nob_cmd_append`
- 避免命名冲突（如 `log()` 与 `math.h` 冲突）

## 完整依赖关系

```
nob.c（27行）
   ↓ #include "nob.h"
nob.h（2616行）
   ├── 声明（1-785行）
   │   ├── Cmd 结构体
   │   ├── String_View 结构体
   │   ├── 函数声明
   │   └── 宏定义
   └── 实现（786-2616行，需要 NOB_IMPLEMENTATION）
       ├── cmd_append() 实现
       ├── cmd_run() 实现
       ├── temp_alloc() 实现
       └── 其他函数实现
```

## 为什么使用单头文件库？

### 优势
1. **零依赖** - 只需要一个 .h 文件
2. **易于集成** - 复制粘贴即可
3. **跨平台** - 内部处理 Windows/Linux 差异
4. **自包含** - 不需要额外的 .c 文件

### 对比传统库
```
传统库：
  libfoo.h（声明）
  libfoo.c（实现）
  libfoo.a（编译后）
  需要链接：-lfoo

单头文件库：
  foo.h（声明 + 实现）
  直接 #include
```

## 类似的单头文件库

| 库 | 用途 | 行数 |
|---|---|---|
| stb_image.h | 图像加载 | ~7000 |
| stb_c_lexer.h | C 词法分析 | ~1000 |
| nob.h | 构建工具 | ~2600 |

## 总结

**nob.h 对 nob.c 的作用**：

1. **提供完整的构建工具库** - 命令执行、文件操作、内存管理
2. **单头文件模式** - 通过 `NOB_IMPLEMENTATION` 控制是否展开实现
3. **简化 API** - 通过 `NOB_STRIP_PREFIX` 去掉前缀
4. **零依赖** - 不需要额外的库文件

**本质**：nob.h 是一个 **2600 行的构建工具库**，nob.c 只是一个 **27 行的配置文件**。
