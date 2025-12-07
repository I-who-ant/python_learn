# crepl.c - C 语言 REPL 实现原理详解

## 项目概述

**crepl** = C REPL（Read-Eval-Print Loop），一个交互式 C 函数调用器。

**核心能力**：无需编译，直接调用共享库中的 C 函数。

## 代码结构分析

### 1. 头文件引入（1-8行）

```c
#include <stdio.h>
#include <dlfcn.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <ffi.h>
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
```

**依赖库解析**：

| 库 | 用途 | 关键函数 |
|---|---|---|
| `stdio.h` | 标准输入输出 | `printf`, `fgets` |
| `dlfcn.h` | 动态链接库操作 | `dlopen`, `dlsym` |
| `nob.h` | 构建工具库 | `shift`, `temp_alloc` |
| `ffi.h` | 外部函数接口 | `ffi_call`, `ffi_prep_cif` |
| `stb_c_lexer.h` | C 语法词法分析 | `stb_c_lexer_get_token` |

### 2. 全局变量和类型定义（10-24行）

```c
char line[1024];

typedef void (*fn_t)(void);

typedef struct {
    ffi_type **items;
    size_t count;z
    size_t capacity;
} Types;

typedef struct {
    void **items;
    size_t count;
    size_t capacity;
} Values;
```

**数据结构说明**：

- `line[1024]` - 用户输入缓冲区
- `fn_t` - 函数指针类型（用于 dlsym 返回值）
- `Types` - 动态数组，存储参数类型（`ffi_type*`）
- `Values` - 动态数组，存储参数值（`void*`）

**为什么需要两个数组？**
```
输入：InitWindow 800 600 "Hello"
Types:  [ffi_type_sint32, ffi_type_sint32, ffi_type_pointer]
Values: [&800,            &600,            &"Hello"         ]
```

### 3. 主函数 - 参数解析（26-42行）

```c
int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }

    const char *dll_path = shift(argv, argc);

    void *dll = dlopen(dll_path, RTLD_NOW);
    if (dll == NULL) {
        fprintf(stderr, "ERROR: %s\n", dlerror());
        return 1;
    }
```

**执行流程**：

1. `shift(argv, argc)` - 从 nob.h，弹出第一个参数
   ```c
   // 输入：./crepl libraylib.so
   program_name = "crepl"
   dll_path = "libraylib.so"
   ```

2. `dlopen(dll_path, RTLD_NOW)` - 加载共享库
   - `RTLD_NOW` - 立即解析所有符号（vs `RTLD_LAZY` 延迟解析）
   - 返回库句柄（`void*`），失败返回 `NULL`

**关键点**：`dlopen` 不需要头文件，只需要 `.so` 文件路径。

### 4. 初始化数据结构（44-49行）

```c
ffi_cif cif = {0};
Types args = {0};
Values values = {0};
size_t mark = temp_save();
stb_lexer l = {0};
static char string_store[1024];
```

**变量说明**：

- `ffi_cif` - Call Interface（调用接口描述符），存储函数签名信息
- `args` / `values` - 参数类型和值的动态数组
- `mark` - 临时内存标记（nob.h 的内存池机制）
- `l` - 词法分析器状态
- `string_store` - 字符串字面量存储区

### 5. 主循环 - REPL 核心（50-104行）

#### 5.1 读取用户输入（50-59行）

```c
for (;;) {
next:
    temp_rewind(mark);

    printf("> ");
    if (fgets(line, sizeof(line), stdin) == NULL) break;
    fflush(stdout);
    String_View sv = sv_trim(sv_from_cstr(line));

    stb_c_lexer_init(&l, sv.data, sv.data + sv.count, string_store, ARRAY_LEN(string_store));
```

**执行流程**：

1. `temp_rewind(mark)` - 重置临时内存池（每次循环复用内存）
2. `fgets` - 读取一行输入
3. `sv_trim` - 去除首尾空白（nob.h 提供）
4. `stb_c_lexer_init` - 初始化词法分析器

**示例输入**：
```
> InitWindow 800 600 "Hello"
```

#### 5.2 解析函数名（61-72行）

```c
if (!stb_c_lexer_get_token(&l)) continue;

if (l.token != CLEX_id) {
    printf("ERROR: function name must be an identifier\n");
    continue;
}

void *fn = dlsym(dll, l.string);
if (fn == NULL) {
    printf("ERROR: no function %s found\n", l.string);
    continue;
}
```

**执行流程**：

1. `stb_c_lexer_get_token(&l)` - 获取第一个 token
2. 检查是否为标识符（`CLEX_id`）
3. `dlsym(dll, l.string)` - 根据函数名查找函数地址
   ```c
   // 输入：InitWindow
   // 返回：libraylib.so 中 InitWindow 函数的地址
   void *fn = dlsym(dll, "InitWindow");
   ```

**关键点**：`dlsym` 通过字符串查找符号，这是动态调用的核心。

#### 5.3 解析参数（74-95行）

```c
args.count = 0;
values.count = 0;

while (stb_c_lexer_get_token(&l)) {
    switch (l.token) {
    case CLEX_intlit: {
        da_append(&args, &ffi_type_sint32);
        int *x = temp_alloc(sizeof(int));
        *x = l.int_number;
        da_append(&values, x);
    } break;
    case CLEX_dqstring: {
        da_append(&args, &ffi_type_pointer);
        char **x = temp_alloc(sizeof(char*));
        *x = temp_strdup(l.string);
        da_append(&values, x);
    } break;
    default:
        printf("ERROR: invalid argument token\n");
        goto next;
    }
}
```

**执行流程**：

**输入**：`800 600 "Hello"`

**循环 1**：
- Token: `CLEX_intlit`，值: `800`
- 添加类型：`ffi_type_sint32`
- 分配内存：`int *x = temp_alloc(sizeof(int))`
- 存储值：`*x = 800`
- 添加到 values：`da_append(&values, x)`

**循环 2**：
- Token: `CLEX_intlit`，值: `600`
- 同上

**循环 3**：
- Token: `CLEX_dqstring`，值: `"Hello"`
- 添加类型：`ffi_type_pointer`
- 分配内存：`char **x = temp_alloc(sizeof(char*))`
- 复制字符串：`*x = temp_strdup("Hello")`
- 添加到 values：`da_append(&values, x)`

**结果**：
```
args:   [&ffi_type_sint32, &ffi_type_sint32, &ffi_type_pointer]
values: [&800,              &600,              &"Hello"         ]
```

**关键点**：
- `temp_alloc` - 从临时内存池分配（下次循环自动释放）
- `da_append` - 动态数组追加（nob.h 提供）

#### 5.4 准备调用接口（97-101行）

```c
ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.count, &ffi_type_void, args.items);
if (status != FFI_OK) {
    printf("ERROR: could not create cif\n");
    continue;
}
```

**参数说明**：

```c
ffi_prep_cif(
    &cif,              // 调用接口描述符（输出）
    FFI_DEFAULT_ABI,   // 调用约定（x86_64 上是 System V ABI）
    args.count,        // 参数数量（3）
    &ffi_type_void,    // 返回值类型（void）
    args.items         // 参数类型数组
);
```

**作用**：告诉 libffi 如何调用这个函数。

**等价于声明**：
```c
void InitWindow(int, int, char*);
```

#### 5.5 执行函数调用（103行）

```c
ffi_call(&cif, fn, NULL, values.items);
```

**参数说明**：

```c
ffi_call(
    &cif,          // 调用接口描述符
    fn,            // 函数指针（dlsym 返回的地址）
    NULL,          // 返回值存储位置（void 函数不需要）
    values.items   // 参数值数组
);
```

**等价于**：
```c
InitWindow(800, 600, "Hello");
```

**魔法发生的地方**：libffi 根据 `cif` 中的类型信息，动态构造函数调用栈帧。

## 完整执行流程示例

### 输入
```bash
./crepl ./libraylib.so
> InitWindow 800 600 "Hello"
```

### 执行步骤

1. **加载库**
   ```c
   void *dll = dlopen("./libraylib.so", RTLD_NOW);
   ```

2. **词法分析**
   ```
   Token 1: CLEX_id       → "InitWindow"
   Token 2: CLEX_intlit   → 800
   Token 3: CLEX_intlit   → 600
   Token 4: CLEX_dqstring → "Hello"
   ```

3. **查找函数**
   ```c
   void *fn = dlsym(dll, "InitWindow");
   // fn = 0x7f1234567890 (函数地址)
   ```

4. **构造参数**
   ```
   args:   [sint32, sint32, pointer]
   values: [&800,   &600,   &"Hello"]
   ```

5. **准备调用**
   ```c
   ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 3, &ffi_type_void, args.items);
   ```

6. **执行调用**
   ```c
   ffi_call(&cif, fn, NULL, values.items);
   // 等价于：InitWindow(800, 600, "Hello");
   ```

## 技术亮点

### 1. 零编译开销
不需要 `#include <raylib.h>`，不需要重新编译，直接调用。

### 2. 类型推断
通过词法分析自动识别参数类型：
- 数字 → `int`
- 字符串 → `char*`

### 3. 内存管理
使用临时内存池（`temp_alloc` / `temp_rewind`），每次循环自动释放。

### 4. 动态绑定
运行时解析函数符号，类似 Python 的 `getattr`。

## 局限性分析

### 1. 类型支持有限
```c
case CLEX_intlit:   // 只支持 int
case CLEX_dqstring: // 只支持 char*
```

**不支持**：
- `float` / `double`
- 结构体
- 数组
- 函数指针

### 2. 无返回值处理
```c
ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.count, &ffi_type_void, args.items);
//                                               ^^^^^^^^^^^^^^
//                                               硬编码为 void
```

**无法获取返回值**：
```c
> int result = GetScreenWidth()  // 不支持
```

### 3. 无类型检查
```c
> InitWindow "abc" "def" 123  // 类型错误，但不会报错
```

依赖用户正确输入参数类型。

### 4. 无函数签名验证
```c
> InitWindow 800  // 参数不足，可能崩溃
```

## 扩展思路

### 支持返回值
```c
int result;
ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.count, &ffi_type_sint, args.items);
ffi_call(&cif, fn, &result, values.items);
printf("Result: %d\n", result);
```

### 支持浮点数
```c
case CLEX_floatlit:
    da_append(&args, &ffi_type_float);
    float *x = temp_alloc(sizeof(float));
    *x = l.real_number;
    da_append(&values, x);
```

### 支持函数签名查询
```c
> :type InitWindow
InitWindow(int width, int height, const char *title) -> void
```

## 总结

**crepl 的核心技术栈**：

```
用户输入
   ↓
stb_c_lexer（词法分析）
   ↓
dlsym（符号查找）
   ↓
libffi（动态调用）
   ↓
函数执行
```

**关键技术**：
1. `dlopen/dlsym` - 动态加载和符号解析
2. `libffi` - 动态构造函数调用
3. `stb_c_lexer` - C 语法解析
4. `nob.h` - 内存管理和工具函数

**实用价值**：
- 快速测试 C 库 API
- 学习动态链接原理
- 理解 FFI 机制
