# FFI 与 ABI 底层编程详解

## 目录
1. [什么是 FFI](#什么是-ffi)
2. [什么是 ABI](#什么是-abi)
3. [FFI vs ABI 的关系](#ffi-vs-abi-的关系)
4. [libffi 库详解](#libffi-库详解)
5. [从 crepl.c 理解 FFI 实战](#从-creplc-理解-ffi-实战)
6. [底层编程关键概念](#底层编程关键概念)

---

## 什么是 FFI

### FFI (Foreign Function Interface,外部函数接口)

**定义:** 允许一种编程语言调用另一种语言编写的函数的机制。

### 典型场景

```
Python 代码  →  调用  →  C 语言库
Java 代码    →  调用  →  C++ 库
动态加载     →  调用  →  未知函数
```

### 为什么需要 FFI?

1. **性能优化:** Python/Java 调用 C 提升性能
2. **系统集成:** 调用操作系统 API
3. **库复用:** 复用已有的 C/C++ 库
4. **动态性:** 运行时加载函数(如 crepl)

### FFI 的层次

```
高层 FFI:
  - ctypes (Python)
  - JNI (Java)
  - cffi (Python)

中层 FFI:
  - libffi (提供 C 级别的 FFI)

底层:
  - 汇编/机器码调用约定
```

---

## 什么是 ABI

### ABI (Application Binary Interface,应用二进制接口)

**定义:** 定义**二进制层面**的函数调用约定,包括:
- 参数如何传递(寄存器?栈?)
- 返回值如何返回
- 栈帧如何布局
- 寄存器如何使用

### ABI vs API

| 维度 | API (源码级) | ABI (二进制级) |
|------|------------|--------------|
| **定义** | 函数签名、数据结构 | 调用约定、内存布局 |
| **示例** | `int add(int a, int b)` | "参数 1 放 RDI,参数 2 放 RSI" |
| **改变后** | 需要重新编译 | 需要重新链接/加载 |
| **关注者** | 程序员 | 编译器、链接器 |

### 常见 ABI

```c
// x86-64 System V ABI (Linux/macOS)
// 参数传递: RDI, RSI, RDX, RCX, R8, R9, 栈
// 返回值: RAX

// x86-64 Windows ABI
// 参数传递: RCX, RDX, R8, R9, 栈
// 返回值: RAX

// ARM AAPCS
// 参数传递: R0-R3, 栈
// 返回值: R0
```

### FFI_DEFAULT_ABI

```c
ffi_prep_cif(&cif, FFI_DEFAULT_ABI, ...);
//               ^^^^^^^^^^^^^^^^^
//               自动选择当前平台的默认 ABI
//               Linux x86-64: System V ABI
//               Windows x86-64: Microsoft x64 ABI
```

---

## FFI vs ABI 的关系

```
┌─────────────────────────────────────────┐
│         你的代码(C/Python/Java)           │
└────────────────┬────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────┐
│      FFI 层 (libffi/ctypes/JNI)         │  ← 提供高层接口
│   - 参数解析                             │
│   - 类型转换                             │
│   - 调用封装                             │
└────────────────┬────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────┐
│         ABI 层 (调用约定)                │  ← 底层规则
│   - 参数放入寄存器/栈                     │
│   - 执行 CALL 指令                       │
│   - 从 RAX 取返回值                      │
└────────────────┬────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────┐
│         目标函数(机器码)                  │
└─────────────────────────────────────────┘
```

### 类比理解

| 概念 | FFI | ABI |
|------|-----|-----|
| **相当于** | 快递公司 | 快递规则 |
| **职责** | 打包、发送、接收 | 包裹格式、运输路线 |
| **用户** | 程序员写代码 | 编译器遵循规则 |

---

## libffi 库详解

### libffi 是什么?

**libffi** 是一个可移植的 FFI 库,提供 C 语言级别的**动态函数调用**能力。

### 核心功能

1. **描述函数签名** (`ffi_cif`)
2. **调用任意函数** (`ffi_call`)
3. **跨平台** (自动处理不同 ABI)

### 关键数据结构

#### 1. ffi_cif (Call Interface)

```c
ffi_cif cif;  // 描述一次函数调用的所有信息
```

**包含:**
- 参数类型列表
- 返回值类型
- ABI 类型
- 内部准备的调用信息

#### 2. ffi_type (类型描述符)

```c
ffi_type_void       // void
ffi_type_sint32     // int
ffi_type_pointer    // char*, void*, 任何指针
ffi_type_double     // double
```

**结构:**
```c
typedef struct {
    size_t size;        // 类型大小(字节)
    unsigned short alignment;  // 内存对齐
    unsigned short type;       // 类型类别
    struct ffi_type **elements; // 结构体字段(如果是复合类型)
} ffi_type;
```

### libffi 工作流程

```c
// 第 1 步:准备参数类型
ffi_type *args[2] = {&ffi_type_sint32, &ffi_type_pointer};

// 第 2 步:准备参数值
int arg1 = 42;
char *arg2 = "hello";
void *values[2] = {&arg1, &arg2};

// 第 3 步:准备 cif
ffi_cif cif;
ffi_prep_cif(&cif,
    FFI_DEFAULT_ABI,     // ABI 类型
    2,                   // 参数个数
    &ffi_type_void,      // 返回值类型
    args                 // 参数类型数组
);

// 第 4 步:调用函数
void *fn = dlsym(handle, "some_function");
ffi_call(&cif, fn, NULL, values);
```

### ffi_prep_cif 内部做了什么?

```c
ffi_status ffi_prep_cif(
    ffi_cif *cif,           // 输出:准备好的调用接口
    ffi_abi abi,            // 输入:使用的 ABI
    unsigned int nargs,     // 输入:参数个数
    ffi_type *rtype,        // 输入:返回值类型
    ffi_type **atypes       // 输入:参数类型数组
)
```

**内部操作:**
1. 根据 ABI 确定参数传递方式(寄存器 vs 栈)
2. 计算每个参数的内存布局
3. 计算栈空间需求
4. 生成调用桩(stub)代码

### ffi_call 内部做了什么?

```c
void ffi_call(
    ffi_cif *cif,          // 调用接口描述
    void (*fn)(void),      // 要调用的函数指针
    void *rvalue,          // 返回值存储位置(可为NULL)
    void **avalue          // 参数值数组
)
```

**内部操作:**
1. 根据 cif 的信息准备寄存器/栈
2. 将参数值从 `avalue` 加载到寄存器或压栈
3. 执行 `call fn` 汇编指令
4. 从 RAX 获取返回值,存入 `rvalue`

---

## 从 crepl.c 理解 FFI 实战

### crepl 是什么?

**crepl** = C REPL (Read-Eval-Print Loop)

**功能:** 交互式调用动态库中的 C 函数

```bash
$ ./crepl libraylib.so
> DrawText "Hello" 100 100 20
> DrawCircle 200 200 50
```

### crepl.c 完整流程分析

#### 第 1 步:加载动态库 (dlfcn.h)

```c
void *dll = dlopen(dll_path, RTLD_NOW);
//                            ^^^^^^^^^
//                            立即解析所有符号
```

**dlopen 做了什么?**
1. 加载 ELF 文件到内存
2. 解析符号表
3. 执行 `.init` 段代码
4. 返回句柄

#### 第 2 步:解析用户输入 (stb_c_lexer.h)

```c
// 输入: DrawText "Hello" 100 100 20
stb_c_lexer_init(&l, sv.data, sv.data + sv.count, ...);

// Token 1: DrawText  → CLEX_id (标识符)
// Token 2: "Hello"   → CLEX_dqstring (双引号字符串)
// Token 3: 100       → CLEX_intlit (整数字面量)
// Token 4: 100       → CLEX_intlit
// Token 5: 20        → CLEX_intlit
```

#### 第 3 步:查找函数符号

```c
void *fn = dlsym(dll, "DrawText");
//         ^^^^^^ 从符号表查找函数地址
if (fn == NULL) {
    printf("ERROR: no function DrawText found\n");
}
```

**dlsym 原理:**
```
libraylib.so 的符号表:
0x00001234  DrawText
0x00005678  DrawCircle
...

dlsym("DrawText") → 返回 0x00001234
```

#### 第 4 步:构建参数类型和值 (关键!)

```c
Types args = {0};      // ffi_type* 数组
Values values = {0};   // void* 数组

while (stb_c_lexer_get_token(&l)) {
    switch (l.token) {
    case CLEX_intlit: {
        // 1. 记录类型
        da_append(&args, &ffi_type_sint32);

        // 2. 分配内存并存储值
        int *x = temp_alloc(sizeof(int));
        *x = l.int_number;  // 100

        // 3. 保存值的指针
        da_append(&values, x);
    } break;

    case CLEX_dqstring: {
        da_append(&args, &ffi_type_pointer);
        char **x = temp_alloc(sizeof(char*));
        *x = temp_strdup(l.string);  // "Hello"
        da_append(&values, x);
    } break;
    }
}
```

**关键点:**
- `args.items`:存储 `ffi_type*`,告诉 libffi 参数类型
- `values.items`:存储 `void*`,指向实际参数值

**示例:**
```
DrawText "Hello" 100 100 20

args.items[0] = &ffi_type_pointer  →  values.items[0] = &ptr  → ptr = "Hello"
args.items[1] = &ffi_type_sint32   →  values.items[1] = &int1 → int1 = 100
args.items[2] = &ffi_type_sint32   →  values.items[2] = &int2 → int2 = 100
args.items[3] = &ffi_type_sint32   →  values.items[3] = &int3 → int3 = 20
```

#### 第 5 步:准备 FFI 调用接口

```c
ffi_cif cif = {0};
ffi_status status = ffi_prep_cif(
    &cif,                    // 输出:准备好的接口
    FFI_DEFAULT_ABI,         // Linux x86-64: System V ABI
    args.count,              // 4 个参数
    &ffi_type_void,          // DrawText 返回 void
    args.items               // [pointer, sint32, sint32, sint32]
);
```

**ffi_prep_cif 为 DrawText 做的准备:**
```
参数 1 (char*):    放入 RDI 寄存器
参数 2 (int):      放入 RSI 寄存器
参数 3 (int):      放入 RDX 寄存器
参数 4 (int):      放入 RCX 寄存器
```

#### 第 6 步:执行调用

```c
ffi_call(&cif, fn, NULL, values.items);
//         ^    ^   ^     ^
//         |    |   |     └─ 参数值数组
//         |    |   └─ 返回值存储(NULL=忽略)
//         |    └─ 函数指针(0x00001234)
//         └─ 调用描述
```

**ffi_call 生成的汇编(伪代码):**
```asm
mov rdi, [values.items[0]]  ; rdi = "Hello"
mov rsi, [values.items[1]]  ; rsi = 100
mov rdx, [values.items[2]]  ; rdx = 100
mov rcx, [values.items[3]]  ; rcx = 20
call [fn]                   ; call 0x00001234
```

### 为什么 crepl 需要 libffi?

```c
// 问题:我们不知道 DrawText 的签名!
void DrawText(const char *text, int x, int y, int size);

// 传统方式:需要硬编码
DrawText("Hello", 100, 100, 20);

// 动态方式:只知道函数名和参数
void *fn = dlsym(dll, "DrawText");
// 如何调用 fn?参数类型、个数都是运行时决定的!

// libffi 提供了解决方案
ffi_call(&cif, fn, NULL, values.items);
```

---

## 底层编程关键概念

### 1. 动态链接 (dlfcn.h)

| 函数 | 作用 | 示例 |
|------|------|------|
| `dlopen` | 加载共享库 | `dlopen("libffi.so", RTLD_NOW)` |
| `dlsym` | 查找符号 | `dlsym(handle, "ffi_call")` |
| `dlclose` | 卸载库 | `dlclose(handle)` |
| `dlerror` | 获取错误 | `dlerror()` |

**符号表示例:**
```bash
$ nm -D /usr/lib/libffi.so | grep ffi_call
00001a20 T ffi_call
```

### 2. 调用约定 (Calling Convention)

#### x86-64 System V ABI (Linux)

```c
void example(int a, int b, int c, int d, int e, int f, int g);
//           ^    ^    ^    ^    ^    ^    ^
//           RDI  RSI  RDX  RCX  R8   R9   栈
```

**汇编实现:**
```asm
mov edi, a
mov esi, b
mov edx, c
mov ecx, d
mov r8d, e
mov r9d, f
push g       ; 第7个参数压栈
call example
add rsp, 8   ; 清理栈
```

#### 栈帧布局

```
高地址
┌────────────────┐
│   参数 n       │
│   ...          │
│   参数 7       │  ← 超过 6 个的参数
├────────────────┤
│   返回地址     │  ← call 指令压入
├────────────────┤
│   旧 rbp      │  ← push rbp
├────────────────┤
│   局部变量     │  ← 当前函数的栈帧
│   ...          │
└────────────────┘
低地址            ← rsp
```

### 3. 类型布局与对齐

```c
// x86-64 类型大小
sizeof(char)       = 1
sizeof(short)      = 2
sizeof(int)        = 4
sizeof(long)       = 8
sizeof(void*)      = 8
sizeof(double)     = 8

// 对齐要求
_Alignof(char)     = 1
_Alignof(int)      = 4
_Alignof(double)   = 8
```

**结构体对齐:**
```c
struct Example {
    char a;     // offset 0, size 1
    // 填充 3 字节
    int b;      // offset 4, size 4
    char c;     // offset 8, size 1
    // 填充 7 字节
};  // 总大小:16 字节(8 字节对齐)
```

### 4. 内存管理技巧

#### crepl 的 temp_alloc 机制

```c
size_t mark = temp_save();          // 保存当前位置
// ... 分配临时内存
temp_rewind(mark);                   // 一次性回收
```

**原理:**
```
临时内存池:
┌────┬────┬────┬────┬────┬────┐
│ A  │ B  │ C  │    │    │    │
└────┴────┴────┴────┴────┴────┘
     ^           ^
     mark        current

temp_rewind(mark) → current = mark
所有 mark 之后的分配都作废
```

**优点:**
- 无需手动 free
- 避免内存泄漏
- 高效(只移动指针)

### 5. 词法分析

**stb_c_lexer.h 的作用:**

```c
// 输入字符串
"DrawText \"Hello World\" 100 200 20"

// Token 流
CLEX_id        "DrawText"
CLEX_dqstring  "Hello World"
CLEX_intlit    100
CLEX_intlit    200
CLEX_intlit    20
```

**实现原理:**
```c
// 状态机
while (*p) {
    if (isalpha(*p)) return CLEX_id;
    if (isdigit(*p)) return CLEX_intlit;
    if (*p == '"')   return CLEX_dqstring;
    // ...
}
```

---

## 总结

### 技术栈层次

```
┌────────────────────────────────────┐
│  crepl.c (应用层)                    │
│  - 交互式输入                        │
│  - 词法分析                          │
├────────────────────────────────────┤
│  libffi (FFI 层)                    │
│  - ffi_prep_cif                     │
│  - ffi_call                         │
├────────────────────────────────────┤
│  dlfcn (动态链接层)                  │
│  - dlopen                           │
│  - dlsym                            │
├────────────────────────────────────┤
│  ABI (调用约定层)                    │
│  - 寄存器分配                        │
│  - 栈帧布局                          │
├────────────────────────────────────┤
│  CPU (硬件层)                        │
│  - 汇编指令                          │
│  - 寄存器                            │
└────────────────────────────────────┘
```

### 关键点回顾

1. **FFI = 跨语言函数调用**
   - libffi 提供 C 级别的动态调用

2. **ABI = 二进制调用约定**
   - 定义参数传递、栈布局

3. **crepl 的核心技术:**
   - dlopen/dlsym: 动态加载函数
   - libffi: 运行时构造函数调用
   - stb_c_lexer: 解析用户输入

4. **为什么需要这些?**
   - 不知道函数签名(运行时决定)
   - 参数类型、个数动态
   - 跨平台(不同 ABI)

### 延伸阅读

- `01-dlfcn.h动态链接库接口.md` - 深入动态链接
- `04-ELF与动态链接原理深度解析.md` - ELF 文件格式
- `05-从Java角度理解C动态链接.md` - JNI 与 FFI 对比
- `06-从Python角度理解C动态链接.md` - ctypes 与 libffi

---

## 实验:手写简化版 FFI

```c
#include <stdio.h>

// 模拟 ffi_call 的简单版本(只支持 int 参数)
void simple_ffi_call(void *fn, int args[], int nargs) {
    // 注意:这只是演示,实际需要汇编
    switch (nargs) {
    case 0:
        ((void(*)(void))fn)();
        break;
    case 1:
        ((void(*)(int))fn)(args[0]);
        break;
    case 2:
        ((void(*)(int,int))fn)(args[0], args[1]);
        break;
    // ...
    }
}

void test(int a, int b) {
    printf("a=%d, b=%d\n", a, b);
}

int main() {
    int args[] = {10, 20};
    simple_ffi_call(test, args, 2);  // 输出:a=10, b=20
    return 0;
}
```

**限制:** 必须预先知道参数个数和类型

**libffi 的优势:** 完全动态,无需预定义!
