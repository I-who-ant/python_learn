# crepl.c 完整源码解析与 FFI 实现原理

## 目录
1. [crepl 项目概述](#crepl-项目概述)
2. [依赖库分析](#依赖库分析)
3. [核心数据结构](#核心数据结构)
4. [源码逐行解析](#源码逐行解析)
5. [FFI 调用流程详解](#ffi-调用流程详解)
6. [内存管理机制](#内存管理机制)
7. [实战示例](#实战示例)
8. [常见问题](#常见问题)

---

## crepl 项目概述

### 项目定位
**crepl** = **C REPL** (Read-Eval-Print Loop)

一个交互式的 C 函数调用器,可以动态加载 `.so` 动态库并调用其中的函数。

### 核心功能

```bash
$ ./crepl libraylib.so
> DrawText "Hello World" 100 200 20
> DrawCircle 300 300 50
> CloseWindow
> ^D
Quit
```

### 技术亮点

1. **运行时动态加载函数** (dlopen/dlsym)
2. **运行时构造函数调用** (libffi)
3. **词法分析解析参数** (stb_c_lexer)
4. **高效内存管理** (temp allocator)

---

## 依赖库分析

### 1. dlfcn.h - 动态链接库接口

```c
#include <dlfcn.h>
```

**提供:**
- `dlopen()` - 加载动态库
- `dlsym()` - 查找符号地址
- `dlerror()` - 获取错误信息

**作用:** 让 crepl 可以在运行时加载任意 `.so` 文件

### 2. nob.h - 构建系统与工具库

```c
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
```

**提供:**
- `shift()` - 解析命令行参数
- `temp_alloc()` - 临时内存分配
- `temp_save()`/`temp_rewind()` - 内存池管理
- `da_append()` - 动态数组追加

**详见:** `nob.h作用说明.md`

### 3. ffi.h - 外部函数接口库

```c
#include <ffi.h>
```

**提供:**
- `ffi_cif` - 函数调用接口描述符
- `ffi_type` - 类型描述符
- `ffi_prep_cif()` - 准备调用接口
- `ffi_call()` - 执行动态调用

**作用:** 让 crepl 可以调用未知签名的函数

### 4. stb_c_lexer.h - C 语言词法分析器

```c
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
```

**提供:**
- `stb_c_lexer_init()` - 初始化词法分析器
- `stb_c_lexer_get_token()` - 获取下一个 token
- Token 类型:`CLEX_id`, `CLEX_intlit`, `CLEX_dqstring`

**作用:** 解析用户输入的函数名和参数

---

## 核心数据结构

### 1. Types - 参数类型列表

```c
typedef struct {
    ffi_type **items;   // ffi_type 指针数组
    size_t count;       // 当前元素个数
    size_t capacity;    // 数组容量
} Types;
```

**用途:** 存储函数参数的类型信息

**示例:**
```c
Types args;
da_append(&args, &ffi_type_pointer);  // 第 1 个参数:char*
da_append(&args, &ffi_type_sint32);   // 第 2 个参数:int
// args.items[0] = &ffi_type_pointer
// args.items[1] = &ffi_type_sint32
// args.count = 2

//Types args; 声明了一个动态数组变量 
//args da_append(&args, &ffi_type_pointer); 将&ffi_type_pointer 追加到 args 数组中。
```

### 2. Values - 参数值列表

```c
typedef struct {
    void **items;      // void 指针数组
    size_t count;      // 当前元素个数
    size_t capacity;   // 数组容量
} Values;
```

**用途:** 存储函数参数的实际值(的指针)

**示例:**
```c
Values values;
char *str = "Hello";
int num = 100;
da_append(&values, &str);   // values.items[0] = &str
da_append(&values, &num);   // values.items[1] = &num
```

**关键点:** 存储的是**指向值的指针**,不是值本身!

### 3. ffi_cif - 函数调用接口

```c
ffi_cif cif = {0};
```

**包含信息:**
- ABI 类型 (FFI_DEFAULT_ABI)
- 参数个数
- 参数类型数组
- 返回值类型
- 内部准备的调用信息

**生命周期:**
1. 初始化为 0
2. 通过 `ffi_prep_cif()` 填充
3. 传递给 `ffi_call()` 执行调用

---

## 源码逐行解析

### main() 函数结构

```c
int main(int argc, char **argv)
{
    // === 第 1 部分:初始化 ===
    const char *program_name = shift(argv, argc);
    const char *dll_path = shift(argv, argc);
    void *dll = dlopen(dll_path, RTLD_NOW);

    // === 第 2 部分:准备数据结构 ===
    ffi_cif cif = {0};
    Types args = {0};
    Values values = {0};
    size_t mark = temp_save();
    stb_lexer l = {0};

    // === 第 3 部分:REPL 循环 ===
    for (;;) {
        // 读取输入
        // 解析 token
        // 查找函数
        // 构建参数
        // 调用函数
    }

    return 0;
}
```

---

### 第 1 部分:初始化 (27-42 行)

#### 1.1 解析命令行参数

```c
const char *program_name = shift(argv, argc);
```

**shift() 作用:**

```c
// 之前:argv = ["crepl", "libraylib.so"]
const char *first = shift(argv, argc);
// 返回:first = "crepl"
// 之后:argv = ["libraylib.so"], argc = 1
```

#### 1.2 检查参数

```c
if (argc <= 0) {
    fprintf(stderr, "Usage: %s <input>\n", program_name);
    fprintf(stderr, "ERROR: no input is provided\n");
    return 1;
}
```

#### 1.3 加载动态库

```c
const char *dll_path = shift(argv, argc);  // "libraylib.so"

void *dll = dlopen(dll_path, RTLD_NOW);
//                            ^^^^^^^^^
//                            立即解析所有符号
```

**dlopen() 内部流程:**
```
1. 读取 ELF 文件
2. 映射到内存
3. 解析符号表
4. 执行初始化函数(.init)
5. 返回句柄
```

**错误处理:**
```c
if (dll == NULL) {
    fprintf(stderr, "ERROR: %s\n", dlerror());
    //                              ^^^^^^^^
    //                              获取详细错误信息
    return 1;
}
```

---

### 第 2 部分:准备数据结构 (44-49 行)

```c
ffi_cif cif = {0};           // FFI 调用接口
Types args = {0};            // 参数类型列表
Values values = {0};         // 参数值列表
size_t mark = temp_save();   // 内存池标记
stb_lexer l = {0};           // 词法分析器
static char string_store[1024];  // 字符串存储缓冲区
```

**temp_save() 作用:**
```
临时内存池:
┌────────────────────────────────┐
│  已分配  │     空闲             │
└──────────┴─────────────────────┘
           ↑
          mark

每次循环开始时通过 temp_rewind(mark) 回收内存
```

---

### 第 3 部分:REPL 循环 (50-104 行)

#### 3.1 读取用户输入 (54-56 行)

```c
printf("> ");
if (fgets(line, sizeof(line), stdin) == NULL) break;
//         ^     ^              ^
//         |     |              └─ 从标准输入读取
//         |     └─ 最多读取 1024 字节
//         └─ 存储到全局缓冲区
fflush(stdout);
```

**fgets() 特点:**
- 读取一整行(包括 `\n`)
- 遇到 EOF 返回 NULL (Ctrl+D)
- 自动添加 `\0` 结尾

#### 3.2 初始化词法分析器 (57-59 行)

```c
String_View sv = sv_trim(sv_from_cstr(line));
//               ^^^^^^^ 去除首尾空白
//                       ^^^^^^^^^^^^^ 转换为 String_View

stb_c_lexer_init(&l,
    sv.data,                    // 输入字符串起始
    sv.data + sv.count,         // 输入字符串结束
    string_store,               // 字符串字面量存储缓冲区
    ARRAY_LEN(string_store)     // 缓冲区大小
);
```

**String_View 结构:**
```c
typedef struct {
    const char *data;
    size_t count;
} String_View;
```

#### 3.3 解析函数名 (61-72 行)

```c
if (!stb_c_lexer_get_token(&l)) continue;
//  ^^^^^^^^^^^^^^^^^^^^^^^^
//  获取下一个 token,如果没有则跳过本次循环

if (l.token != CLEX_id) {
    //         ^^^^^^^^
    //         CLEX_id = 标识符(函数名)
    printf("ERROR: function name must be an identifier\n");
    continue;
}
```

**l.token 可能的值:**
- `CLEX_id` - 标识符 (DrawText, printf)
- `CLEX_intlit` - 整数字面量 (100, -42)
- `CLEX_dqstring` - 双引号字符串 ("Hello")
- `CLEX_floatlit` - 浮点数 (3.14)
- 等等...

#### 3.4 查找函数符号 (68-72 行)

```c
void *fn = dlsym(dll, l.string);
//                    ^^^^^^^^
//                    词法分析器解析出的函数名

if (fn == NULL) {
    printf("ERROR: no function %s found\n", l.string);
    continue;
}
```

**dlsym() 工作原理:**
```
libraylib.so 符号表:
┌──────────────┬──────────┐
│   符号名      │   地址    │
├──────────────┼──────────┤
│ DrawText     │ 0x1234   │
│ DrawCircle   │ 0x5678   │
│ CloseWindow  │ 0xabcd   │
└──────────────┴──────────┘

dlsym(dll, "DrawText") → 0x1234
```

#### 3.5 清空参数列表 (74-75 行)

```c
args.count = 0;
values.count = 0;
```

**为什么只设置 count = 0?**
- 数组内存保留(capacity 不变)
- 避免重复分配
- 下次 `da_append()` 会覆盖旧值

---

### 3.6 解析参数 (77-95 行) - 核心逻辑!

```c
while (stb_c_lexer_get_token(&l)) {
    //    ^^^^^^^^^^^^^^^^^^^^^^^^
    //    继续获取 token 直到结束

    switch (l.token) {
```

#### Case 1: 整数字面量 (79-84 行)

```c
case CLEX_intlit: {
    // 步骤 1:记录类型
    da_append(&args, &ffi_type_sint32);
    //               ^^^^^^^^^^^^^^^^
    //               告诉 libffi 这是 int 类型

    // 步骤 2:分配内存存储值
    int *x = temp_alloc(sizeof(int));
    //       ^^^^^^^^^^
    //       从临时内存池分配

    // 步骤 3:存储实际值
    *x = l.int_number;
    //   ^^^^^^^^^^^^
    //   词法分析器解析出的整数值

    // 步骤 4:保存值的地址
    da_append(&values, x);
    //                 ^
    //                 注意:存的是指针!
} break;
```

**示例:**
```
输入: 100

Token: CLEX_intlit, l.int_number = 100

执行后:
args.items[n] = &ffi_type_sint32
values.items[n] = 0x7fff1234  ──→  [100]
                                    内存中的 int
```

#### Case 2: 字符串字面量 (85-90 行)

```c
case CLEX_dqstring: {
    // 步骤 1:记录类型
    da_append(&args, &ffi_type_pointer);
    //               ^^^^^^^^^^^^^^^^^
    //               告诉 libffi 这是指针类型

    // 步骤 2:分配存储 char* 的空间
    char **x = temp_alloc(sizeof(char*));
    //    ^^
    //    注意:这是指向指针的指针!

    // 步骤 3:复制字符串
    *x = temp_strdup(l.string);
    //   ^^^^^^^^^^^
    //   从 string_store 复制到临时内存池

    // 步骤 4:保存 char** 的地址
    da_append(&values, x);
} break;
```

**示例:**
```
输入: "Hello"

Token: CLEX_dqstring, l.string = "Hello"

执行后:
args.items[n] = &ffi_type_pointer

values.items[n] = 0x7fff5678  ──→  [0x7fff9abc] ──→ "Hello\0"
                  char**             char*          字符串数据
```

**为什么需要 char**?**
```c
// ffi_call 的接口:
void ffi_call(..., void **avalue);
//                 ^^^^^^^ 指向参数值的指针数组

// 对于 char* 参数:
char *str = "Hello";
void *array[1] = {&str};  // 需要 &str,不是 str!
//                ^^^^
//                char** 类型

ffi_call(..., array);
```

#### 错误处理 (91-94 行)

```c
default:
    printf("ERROR: invalid argument token\n");
    goto next;
    //   ^^^^
    //   跳到外层循环的开始(第 51 行)
}
```

---

### 3.7 准备 FFI 调用接口 (97-101 行)

```c
ffi_status status = ffi_prep_cif(
    &cif,                   // 输出:准备好的接口
    FFI_DEFAULT_ABI,        // 使用默认 ABI
    args.count,             // 参数个数
    &ffi_type_void,         // 返回值类型(void)
    args.items              // 参数类型数组
);
```

**ffi_prep_cif() 内部做了什么?**

1. **检查 ABI 有效性**
2. **计算每个参数的大小和对齐**
3. **决定参数传递方式:**
   ```
   x86-64 System V ABI:
   参数 1-6: RDI, RSI, RDX, RCX, R8, R9
   参数 7+:  栈
   ```
4. **计算栈空间需求**
5. **生成调用桩代码(stub)**

**示例:**
```c
// DrawText "Hello" 100 200 20
ffi_prep_cif(&cif,
    FFI_DEFAULT_ABI,
    4,                      // 4 个参数
    &ffi_type_void,
    [&ffi_type_pointer, &ffi_type_sint32,
     &ffi_type_sint32, &ffi_type_sint32]
);

// 内部决定:
// 参数 1 (char*) → RDI
// 参数 2 (int)   → RSI
// 参数 3 (int)   → RDX
// 参数 4 (int)   → RCX
```

**错误检查:**
```c
if (status != FFI_OK) {
    printf("ERROR: could not create cif\n");
    continue;
}
```

---

### 3.8 执行函数调用 (103 行)

```c
ffi_call(&cif, fn, NULL, values.items);
//       ^     ^   ^     ^
//       |     |   |     └─ 参数值数组
//       |     |   └─ 返回值存储位置(NULL = 忽略)
//       |     └─ 函数指针
//       └─ 调用接口描述
```

**ffi_call() 内部流程:**

#### 步骤 1:加载参数到寄存器/栈

```asm
; 假设 DrawText(char *text, int x, int y, int size)
mov rdi, [values.items[0]]  ; rdi = "Hello"
mov esi, [values.items[1]]  ; rsi = 100
mov edx, [values.items[2]]  ; rdx = 200
mov ecx, [values.items[3]]  ; rcx = 20
```

**注意:** 实际通过 cif 的信息动态生成上述代码!

#### 步骤 2:执行调用

```asm
call [fn]  ; call 0x1234 (DrawText 的地址)
```

#### 步骤 3:获取返回值

```asm
; 如果有返回值
mov [rvalue], rax  ; 从 RAX 获取返回值
```

**完整示例:**
```c
// 用户输入: DrawText "Hello" 100 200 20

// ffi_call 生成的汇编(伪代码):
void ffi_call_stub() {
    __asm__ (
        "mov rdi, %0\n"    // "Hello"
        "mov esi, %1\n"    // 100
        "mov edx, %2\n"    // 200
        "mov ecx, %3\n"    // 20
        "call *%4\n"       // DrawText
        :
        : "r"(values[0]), "r"(values[1]),
          "r"(values[2]), "r"(values[3]),
          "r"(fn)
    );
}
```

---

### 3.9 内存回收 (52 行)

```c
next:
    temp_rewind(mark);
    //          ^^^^
    //          回收本次循环分配的所有临时内存
```

**内存池状态:**
```
循环开始:
┌────────────────────────────────┐
│  已分配  │     空闲             │
└──────────┴─────────────────────┘
           ↑
          mark

循环中分配:
┌────────────────────────────────┐
│  已分配  │ int, char*, ...│空闲│
└──────────┴────────────────┴────┘
           ↑                ↑
          mark            current

temp_rewind(mark):
┌────────────────────────────────┐
│  已分配  │     空闲             │
└──────────┴─────────────────────┘
           ↑
        mark = current
```

---

## FFI 调用流程详解

### 完整流程图

```
用户输入: DrawText "Hello" 100 200 20
    │
    ↓
┌──────────────────────────────────────┐
│  1. 词法分析 (stb_c_lexer)             │
│     Token 1: CLEX_id "DrawText"      │
│     Token 2: CLEX_dqstring "Hello"   │
│     Token 3: CLEX_intlit 100         │
│     Token 4: CLEX_intlit 200         │
│     Token 5: CLEX_intlit 20          │
└──────────────┬───────────────────────┘
               ↓
┌──────────────────────────────────────┐
│  2. 查找函数 (dlsym)                   │
│     fn = dlsym(dll, "DrawText")      │
│     fn = 0x00001234                  │
└──────────────┬───────────────────────┘
               ↓
┌──────────────────────────────────────┐
│  3. 构建参数类型 (args)                │
│     args.items[0] = &ffi_type_pointer│
│     args.items[1] = &ffi_type_sint32 │
│     args.items[2] = &ffi_type_sint32 │
│     args.items[3] = &ffi_type_sint32 │
└──────────────┬───────────────────────┘
               ↓
┌──────────────────────────────────────┐
│  4. 构建参数值 (values)                │
│     values.items[0] = &str ──→ "Hello"│
│     values.items[1] = &int1 ──→ 100  │
│     values.items[2] = &int2 ──→ 200  │
│     values.items[3] = &int3 ──→ 20   │
└──────────────┬───────────────────────┘
               ↓
┌──────────────────────────────────────┐
│  5. 准备调用接口 (ffi_prep_cif)        │
│     分析 ABI,决定参数传递方式           │
│     参数 1 → RDI                      │
│     参数 2 → RSI                      │
│     参数 3 → RDX                      │
│     参数 4 → RCX                      │
└──────────────┬───────────────────────┘
               ↓
┌──────────────────────────────────────┐
│  6. 执行调用 (ffi_call)                │
│     mov rdi, "Hello"                 │
│     mov esi, 100                     │
│     mov edx, 200                     │
│     mov ecx, 20                      │
│     call 0x00001234                  │
└──────────────────────────────────────┘
```

### 关键点:类型与值的对应

```c
// 参数类型数组
ffi_type **args.items = {
    &ffi_type_pointer,  // ─┐
    &ffi_type_sint32,   //  │
    &ffi_type_sint32,   //  │ 告诉 libffi 参数类型
    &ffi_type_sint32    //  │
};                      // ─┘

// 参数值数组
void **values.items = {
    &ptr,   // ─┐ ptr   = "Hello\0"
    &int1,  //  │ int1  = 100
    &int2,  //  │ int2  = 200
    &int3   //  │ int3  = 20
};          // ─┘

// libffi 组合两者生成调用
ffi_call() 内部:
    for (i = 0; i < nargs; i++) {
        根据 args.items[i] 决定如何加载 values.items[i]
        if (args.items[i] == &ffi_type_pointer) {
            加载指针到寄存器: mov rdi, [values.items[i]]
        } else if (args.items[i] == &ffi_type_sint32) {
            加载整数到寄存器: mov esi, [values.items[i]]
        }
    }
```

---

## 内存管理机制

### temp_alloc 临时内存池

#### 原理

```c
// nob.h 内部实现(简化版)
static char temp_pool[1024 * 1024];  // 1MB 内存池
static size_t temp_pos = 0;           // 当前位置

void *temp_alloc(size_t size) {
    void *ptr = &temp_pool[temp_pos];
    temp_pos += size;
    return ptr;
}

size_t temp_save() {
    return temp_pos;  // 保存当前位置
}

void temp_rewind(size_t mark) {
    temp_pos = mark;  // 恢复到之前的位置
}
```

#### 优势

1. **无需手动 free**
   ```c
   for (;;) {
       size_t mark = temp_save();
       int *x = temp_alloc(sizeof(int));
       char *s = temp_alloc(100);
       // 使用 x, s...
       temp_rewind(mark);  // 一次性回收
   }
   ```

2. **避免内存碎片**
   - 线性分配,无碎片

3. **高性能**
   - 分配:只是指针移动
   - 回收:只是指针重置

#### 限制

1. **生命周期固定**
   - 只能在单次循环内使用
   - 不能跨循环返回

2. **容量有限**
   - 如果内存池用完会出错

---

## 实战示例

### 示例 1:调用 printf

```bash
$ ./crepl /lib/x86_64-linux-gnu/libc.so.6
> printf "Hello, %s!\n" "World"
Hello, World!
> printf "Number: %d\n" 42
Number: 42
```

**执行流程:**
```c
// 输入: printf "Hello, %s!\n" "World"

// 步骤 1:解析
fn = dlsym(dll, "printf")  // 找到 printf
args = [&ffi_type_pointer, &ffi_type_pointer]
values = [&fmt, &arg]      // fmt="Hello, %s!\n", arg="World"

// 步骤 2:调用
ffi_call(&cif, fn, NULL, values)

// 步骤 3:执行
mov rdi, "Hello, %s!\n"
mov rsi, "World"
call printf
```

### 示例 2:调用 raylib 函数

```bash
$ ./crepl /usr/lib/libraylib.so
> InitWindow 800 600 "My Game"
> SetTargetFPS 60
> BeginDrawing
> ClearBackground 0
> DrawText "Hello Raylib!" 10 10 20
> EndDrawing
> CloseWindow
```

### 示例 3:错误情况

#### 情况 1:函数不存在

```bash
> NonExistentFunc 123
ERROR: no function NonExistentFunc found
```

#### 情况 2:无效参数

```bash
> printf [123]  # [ 不是有效 token
ERROR: invalid argument token
```

#### 情况 3:缺少库

```bash
$ ./crepl /path/to/nonexistent.so
ERROR: /path/to/nonexistent.so: cannot open shared object file: No such file or directory
```

---

## 常见问题

### Q1: 为什么 values 存储的是指针?

**A:** libffi 的接口要求:
```c
void ffi_call(..., void **avalue);
//                 ^^^^^^^ 指向参数值的指针数组

// 对于 int 参数:
int x = 100;
void *array[1] = {&x};  // 需要 &x
```

### Q2: 为什么字符串需要 char**?

**A:** 因为参数本身是 `char*`,而 `avalue` 需要指向参数的指针:
```c
char *str = "Hello";    // 参数类型是 char*
void **avalue = {&str}; // 需要指向 char* 的指针,即 char**
```

### Q3: crepl 支持哪些参数类型?

**A:** 当前只支持:
- 整数字面量 (`CLEX_intlit`) → `ffi_type_sint32`
- 字符串字面量 (`CLEX_dqstring`) → `ffi_type_pointer`

**扩展方法:**
```c
case CLEX_floatlit: {
    da_append(&args, &ffi_type_double);
    double *x = temp_alloc(sizeof(double));
    *x = l.real_number;
    da_append(&values, x);
} break;
```

### Q4: 如何处理返回值?

**A:** 当前版本忽略返回值(`NULL`),可以修改:
```c
int retval;
ffi_call(&cif, fn, &retval, values.items);
printf("Return value: %d\n", retval);
```

### Q5: 能否调用需要结构体参数的函数?

**A:** 需要扩展:定义 `ffi_type` 的结构体布局
```c
// 示例:struct Point { int x; int y; };
ffi_type *point_fields[] = {
    &ffi_type_sint32,
    &ffi_type_sint32,
    NULL  // 结束标记
};
ffi_type point_type = {
    .size = 0,
    .alignment = 0,
    .type = FFI_TYPE_STRUCT,
    .elements = point_fields
};
```

### Q6: 为什么使用 temp_alloc 而不是 malloc?

**A:** 优势:
1. 无需手动 free
2. 批量回收(`temp_rewind`)
3. 高性能(无系统调用)
4. 避免内存泄漏

---

## 总结

### crepl 的技术栈

```
┌────────────────────────────────┐
│  stb_c_lexer.h                 │  词法分析
│  (解析用户输入)                 │
└────────────┬───────────────────┘
             ↓
┌────────────────────────────────┐
│  dlfcn.h                       │  动态链接
│  (加载库、查找符号)             │
└────────────┬───────────────────┘
             ↓
┌────────────────────────────────┐
│  libffi                        │  动态调用
│  (构造调用、执行)               │
└────────────┬───────────────────┘
             ↓
┌────────────────────────────────┐
│  nob.h                         │  工具库
│  (内存管理、数组、字符串)        │
└────────────────────────────────┘
```

### 核心思想

> **运行时构造函数调用**

传统方式:
```c
DrawText("Hello", 100, 200, 20);  // 编译时确定
```

crepl 方式:
```c
void *fn = dlsym(dll, "DrawText");         // 运行时查找
ffi_call(&cif, fn, NULL, values.items);    // 运行时调用
```

### 学习价值

1. **理解 FFI 原理**
2. **掌握动态链接机制**
3. **学习内存管理技巧**
4. **理解 ABI 和调用约定**

### 延伸阅读

- `08-FFI与ABI底层编程详解.md` - FFI/ABI 概念
- `01-dlfcn.h动态链接库接口.md` - 动态链接详解
- `nob.h作用说明.md` - nob.h 工具库

### 实验建议

1. **添加浮点数支持**
2. **实现返回值打印**
3. **支持结构体参数**
4. **添加错误恢复机制**
5. **实现函数签名提示**

---

## 附录:完整流程示例

### 输入
```
DrawText "Hello" 100 200 20
```

### 执行过程

```c
// 1. 词法分析
stb_c_lexer_get_token(&l) → CLEX_id "DrawText"
stb_c_lexer_get_token(&l) → CLEX_dqstring "Hello"
stb_c_lexer_get_token(&l) → CLEX_intlit 100
stb_c_lexer_get_token(&l) → CLEX_intlit 200
stb_c_lexer_get_token(&l) → CLEX_intlit 20

// 2. 查找函数
fn = dlsym(dll, "DrawText") = 0x00001234

// 3. 构建类型
args.items[0] = &ffi_type_pointer
args.items[1] = &ffi_type_sint32
args.items[2] = &ffi_type_sint32
args.items[3] = &ffi_type_sint32

// 4. 构建值
values.items[0] = &str   (str = "Hello")
values.items[1] = &int1  (int1 = 100)
values.items[2] = &int2  (int2 = 200)
values.items[3] = &int3  (int3 = 20)

// 5. 准备接口
ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 4, &ffi_type_void, args.items)
→ 决定:RDI, RSI, RDX, RCX

// 6. 执行调用
ffi_call(&cif, 0x00001234, NULL, values.items)
→ 生成汇编:
    mov rdi, [values.items[0]]  ; "Hello"
    mov esi, [values.items[1]]  ; 100
    mov edx, [values.items[2]]  ; 200
    mov ecx, [values.items[3]]  ; 20
    call 0x00001234

// 7. DrawText 执行
// (在屏幕上绘制文字)
```

完成!
