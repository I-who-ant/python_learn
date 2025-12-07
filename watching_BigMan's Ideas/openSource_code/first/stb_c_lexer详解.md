# stb_c_lexer.h 详解 - C 语言词法分析器

## 📚 什么是词法分析器?

词法分析器(Lexer)是编译器/解释器的第一步,负责将源代码字符串分解成 **token(词法单元)**。

### 类比理解

就像阅读句子时,大脑会自动将文字分解成单词:

```
句子: "我爱编程"
↓ 分词
单词: ["我", "爱", "编程"]
```

词法分析器做的是同样的事:

```
代码: "InitWindow 800 600 "Hello""
↓ 词法分析
Token: [标识符:InitWindow, 整数:800, 整数:600, 字符串:"Hello"]
```

---

## 🏗️ stb_c_lexer.h 核心结构

### 1. `stb_lexer` 结构体 (第 113-132 行)

这是词法分析器的**状态机**,存储了解析过程中的所有信息:

```c
typedef struct
{
   // ========== 输入流信息 ==========
   char *input_stream;      // 输入字符串的起始位置
   char *eof;               // 输入字符串的结束位置(或 NULL)
   char *parse_point;       // 当前解析到的位置(像一个移动的指针)

   // ========== 字符串存储 ==========
   char *string_storage;    // 用于存储解析出的字符串字面量的缓冲区
   int   string_storage_len;// 缓冲区长度

   // ========== 错误定位 ==========
   char *where_firstchar;   // 当前 token 的第一个字符位置
   char *where_lastchar;    // 当前 token 的最后一个字符位置

   // ========== 解析结果(最重要!) ==========
   long token;              // Token 类型 (CLEX_id, CLEX_intlit 等)
   double real_number;      // 如果是浮点数,值存这里
   long   int_number;       // 如果是整数,值存这里
   char *string;            // 如果是字符串/标识符,指向 string_storage
   int string_len;          // 字符串长度
} stb_lexer;
```

### 📖 工作流程示例

假设输入: `"printf 42 \"hello\""`

```
初始化后:
  input_stream = "printf 42 \"hello\""
  parse_point  = "printf 42 \"hello\""  (指向开头)
  eof          = NULL (或指向末尾)

第 1 次调用 stb_c_lexer_get_token():
  ↓ 解析 "printf"
  token        = CLEX_id
  string       = "printf" (复制到 string_storage)
  parse_point  = " 42 \"hello\""  (移动到空格后)

第 2 次调用 stb_c_lexer_get_token():
  ↓ 跳过空格,解析 "42"
  token        = CLEX_intlit
  int_number   = 42
  parse_point  = " \"hello\""

第 3 次调用 stb_c_lexer_get_token():
  ↓ 跳过空格,解析 "\"hello\""
  token        = CLEX_dqstring
  string       = "hello" (去掉引号,复制到 string_storage)
  parse_point  = ""  (到达末尾)

第 4 次调用 stb_c_lexer_get_token():
  ↓ 没有更多 token
  返回 0 (表示结束)
```

---

## 🎯 Token 类型枚举 (第 177-211 行)

```c
enum
{
   CLEX_eof = 256,         // 文件结束
   CLEX_parse_error,       // 解析错误

   // ========== 字面量 ==========
   CLEX_intlit,            // 整数字面量: 42, 0xFF, 0777
   CLEX_floatlit,          // 浮点数字面量: 3.14, 1e-5
   CLEX_id,                // 标识符: printf, InitWindow
   CLEX_dqstring,          // 双引号字符串: "hello"
   CLEX_sqstring,          // 单引号字符串: 'hello' (可选)
   CLEX_charlit,           // 字符字面量: 'A'

   // ========== 运算符 ==========
   CLEX_eq,                // ==
   CLEX_noteq,             // !=
   CLEX_lesseq,            // <=
   CLEX_greatereq,         // >=
   CLEX_andand,            // &&
   CLEX_oror,              // ||
   CLEX_shl,               // <<
   CLEX_shr,               // >>
   CLEX_plusplus,          // ++
   CLEX_minusminus,        // --
   CLEX_arrow,             // ->
   // ... 更多运算符

   // ========== 单字符 Token ==========
   // 其他字符(如 +, -, *, / 等)直接返回字符的 ASCII 值
};
```

### 特殊规则:单字符 Token

对于单字符运算符(如 `+`, `(`, `;` 等),`token` 字段直接存储字符的 **ASCII 值**:

```c
输入: "x + 3"

Token 1:
  token = CLEX_id
  string = "x"

Token 2:
  token = 43  (ASCII 码 '+')

Token 3:
  token = CLEX_intlit
  int_number = 3
```

---

## 🔧 核心 API 函数

### 1. `stb_c_lexer_init()` - 初始化词法分析器 (第 273 行)

```c
void stb_c_lexer_init(
    stb_lexer *lexer,               // 词法分析器状态
    const char *input_stream,       // 输入字符串起始位置
    const char *input_stream_end,   // 输入字符串结束位置(或 NULL)
    char *string_store,             // 字符串存储缓冲区
    int store_length                // 缓冲区大小
);
```

**作用**: 设置初始状态,准备开始解析

**示例**:
```c
char input[] = "InitWindow 800 600";
char buffer[256];  // 用于存储解析出的字符串
stb_lexer lex;

stb_c_lexer_init(&lex,
                 input,              // 输入起始
                 input + strlen(input), // 输入结束
                 buffer,             // 字符串缓冲区
                 sizeof(buffer));    // 缓冲区大小
```

### 2. `stb_c_lexer_get_token()` - 获取下一个 Token (第 498 行)

```c
int stb_c_lexer_get_token(stb_lexer *lexer);
```

**返回值**:
- `1` = 成功解析到一个 token
- `0` = 到达末尾,没有更多 token

**副作用**: 更新 `lexer` 的以下字段:
- `token` - Token 类型
- `int_number` / `real_number` / `string` - Token 的值

**示例**:
```c
while (stb_c_lexer_get_token(&lex)) {
    switch (lex.token) {
        case CLEX_id:
            printf("标识符: %s\n", lex.string);
            break;
        case CLEX_intlit:
            printf("整数: %ld\n", lex.int_number);
            break;
        case CLEX_dqstring:
            printf("字符串: \"%s\"\n", lex.string);
            break;
        default:
            if (lex.token < 256) {
                printf("字符: %c\n", (char)lex.token);
            }
    }
}
```

---

## 🎬 完整示例:解析 crepl 输入

### 场景:用户输入 `InitWindow 800 600 "Hello"`

```c
// ========== 第 1 步:准备数据 ==========
char line[] = "InitWindow 800 600 \"Hello\"";
char string_store[1024];
stb_lexer l;

// ========== 第 2 步:初始化词法分析器 ==========
stb_c_lexer_init(&l,
                 line,
                 line + strlen(line),
                 string_store,
                 sizeof(string_store));

// ========== 第 3 步:解析 Token 1 - 函数名 ==========
stb_c_lexer_get_token(&l);  // 返回 1(成功)

// 检查结果:
l.token == CLEX_id           // ✅ 是标识符
strcmp(l.string, "InitWindow") == 0  // ✅ 函数名是 "InitWindow"

// 内部状态:
// parse_point 现在指向: " 800 600 \"Hello\""
//                        ↑ 这里

// ========== 第 4 步:解析 Token 2 - 第一个参数 ==========
stb_c_lexer_get_token(&l);  // 返回 1

// 检查结果:
l.token == CLEX_intlit       // ✅ 是整数
l.int_number == 800          // ✅ 值是 800

// 内部状态:
// parse_point 现在指向: " 600 \"Hello\""
//                        ↑ 这里

// ========== 第 5 步:解析 Token 3 - 第二个参数 ==========
stb_c_lexer_get_token(&l);  // 返回 1

l.token == CLEX_intlit       // ✅ 是整数
l.int_number == 600          // ✅ 值是 600

// ========== 第 6 步:解析 Token 4 - 字符串参数 ==========
stb_c_lexer_get_token(&l);  // 返回 1

l.token == CLEX_dqstring     // ✅ 是双引号字符串
strcmp(l.string, "Hello") == 0  // ✅ 值是 "Hello"(引号已去除)

// ========== 第 7 步:检查是否还有更多 Token ==========
stb_c_lexer_get_token(&l);  // 返回 0(到达末尾)
```

---

## 🔍 crepl.c 中的实际使用

让我们看看 `crepl.c` 如何使用这个词法分析器:

### 代码片段 1:初始化 (crepl.c:166 行)

```c
// 用户输入: "InitWindow 800 600 \"Hello\"\n"
String_View sv = sv_trim(sv_from_cstr(line));  // 去除首尾空白

// 初始化词法分析器
stb_c_lexer_init(&l,
                 sv.data,              // 输入起始
                 sv.data + sv.count,   // 输入结束
                 string_store,         // 字符串缓冲区
                 ARRAY_LEN(string_store));
```

**为什么需要 `string_store`?**
- 词法分析器需要复制字符串字面量和标识符
- 例如:`"Hello"` → 去除引号 → 复制 `Hello` 到 `string_store`
- `l.string` 会指向 `string_store` 中的位置

### 代码片段 2:解析函数名 (crepl.c:171 行)

```c
if (!stb_c_lexer_get_token(&l)) continue;  // 获取第一个 token

if (l.token != CLEX_id) {  // 检查是否为标识符
    printf("ERROR: function name must be an identifier\n");
    continue;
}

// 此时:
// l.string = "InitWindow" (存储在 string_store 中)
// 可以直接用于 dlsym 查找函数
void *fn = dlsym(dll, l.string);
```

### 代码片段 3:解析参数 (crepl.c:218 行)

```c
while (stb_c_lexer_get_token(&l)) {  // 循环获取剩余 token
    switch (l.token) {
    case CLEX_intlit: {  // 整数参数
        // l.int_number 包含解析出的整数值
        da_append(&args, &ffi_type_sint32);  // 记录类型
        int *x = temp_alloc(sizeof(int));
        *x = l.int_number;  // 存储值
        da_append(&values, x);
    } break;

    case CLEX_dqstring: {  // 字符串参数
        // l.string 指向解析出的字符串(引号已去除)
        da_append(&args, &ffi_type_pointer);  // 记录类型
        char **x = temp_alloc(sizeof(char*));
        *x = temp_strdup(l.string);  // 复制字符串
        da_append(&values, x);
    } break;
    }
}
```

---

## 🧪 实验:自己动手测试

你可以创建一个简单的测试程序:

```c
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
#include <stdio.h>
#include <string.h>

int main() {
    char input[] = "printf 42 \"hello\" 3.14";
    char buffer[256];
    stb_lexer lex;

    stb_c_lexer_init(&lex, input, input + strlen(input), buffer, sizeof(buffer));

    printf("解析结果:\n");
    while (stb_c_lexer_get_token(&lex)) {
        printf("Token: ");
        switch (lex.token) {
            case CLEX_id:
                printf("标识符 '%s'\n", lex.string);
                break;
            case CLEX_intlit:
                printf("整数 %ld\n", lex.int_number);
                break;
            case CLEX_dqstring:
                printf("字符串 \"%s\"\n", lex.string);
                break;
            case CLEX_floatlit:
                printf("浮点数 %g\n", lex.real_number);
                break;
            default:
                if (lex.token < 256)
                    printf("字符 '%c'\n", (char)lex.token);
        }
    }
    return 0;
}
```

**预期输出**:
```
解析结果:
Token: 标识符 'printf'
Token: 整数 42
Token: 字符串 "hello"
Token: 浮点数 3.14
```

---

## 🎓 关键概念总结

### 1. 词法分析器 = 状态机

```
输入: "a + 3"
      ↓
   [状态机]
   parse_point 不断向右移动
   识别模式:字母、数字、运算符
      ↓
输出: [CLEX_id "a"] ['+'] [CLEX_intlit 3]
```

### 2. Token 存储策略

| Token 类型 | 存储位置 | 示例 |
|-----------|---------|------|
| 整数 | `int_number` | `42` → `int_number = 42` |
| 浮点数 | `real_number` | `3.14` → `real_number = 3.14` |
| 字符串 | `string` → `string_storage` | `"hello"` → `string = "hello"` |
| 标识符 | `string` → `string_storage` | `printf` → `string = "printf"` |
| 单字符 | `token` (ASCII) | `+` → `token = 43` |

### 3. 为什么需要 `string_storage`?

词法分析器不会修改原始输入字符串,而是将解析出的字符串**复制**到缓冲区:

```c
输入字符串: "\"hello world\""
                ↓ 词法分析
去除引号并复制: "hello world" → string_storage[0..10]
                ↓
l.string 指向 string_storage
```

### 4. 调用流程

```
初始化:
  stb_c_lexer_init(&lex, input, end, buffer, size)
  设置 parse_point = input
     ↓
循环:
  while (stb_c_lexer_get_token(&lex)) {
      1. 跳过空白字符
      2. 识别下一个 token 的模式
      3. 解析并填充 lex.token, lex.int_number 等
      4. 移动 parse_point
      5. 返回 1
  }
     ↓
结束:
  parse_point 到达末尾,返回 0
```

---

## 📚 扩展阅读

### stb 系列库

`stb_c_lexer.h` 是 [stb](https://github.com/nothings/stb) 项目的一部分,这是一个著名的**单头文件库**集合:

- `stb_image.h` - 图像加载
- `stb_truetype.h` - TrueType 字体渲染
- `stb_ds.h` - 数据结构(动态数组、哈希表)
- `stb_c_lexer.h` - C 词法分析器

**单头文件库的优点**:
- 无需复杂的构建系统
- 只需 `#include` 即可使用
- 使用 `#define XXX_IMPLEMENTATION` 控制实现代码的包含

### 词法分析 vs 语法分析

```
源代码: "x = 3 + 5;"
   ↓
词法分析(Lexer)
   ↓
Token 流: [CLEX_id "x"] ['='] [CLEX_intlit 3] ['+'] [CLEX_intlit 5] [';']
   ↓
语法分析(Parser)
   ↓
语法树:
    =
   / \
  x   +
     / \
    3   5
```

`stb_c_lexer.h` 只做**词法分析**,不做语法分析。

---

## ❓ 常见问题

### Q1: 为什么 `parse_point` 是 `char*` 而不是索引?

**A**: 使用指针更高效,可以直接传递给其他函数,避免重复的数组索引计算。

### Q2: `string_storage` 会被覆盖吗?

**A**: 会!每次调用 `stb_c_lexer_get_token()` 都可能覆盖 `string_storage`。如果需要保留字符串,必须**复制**:

```c
stb_c_lexer_get_token(&lex);
char *saved = strdup(lex.string);  // 复制到堆
// 或
char *saved = temp_strdup(lex.string);  // 复制到临时内存池
```

### Q3: 如何支持更多 token 类型?

**A**: `stb_c_lexer.h` 通过配置宏控制:

```c
// 启用十六进制浮点数
#define STB_C_LEX_C99_HEX_FLOATS Y
#include "stb_c_lexer.h"
```

查看文件顶部(第 54-106 行)了解所有配置选项。

---

## 🎯 总结

`stb_c_lexer.h` 是一个:
- ✅ **轻量级**的 C 词法分析器
- ✅ **单头文件**,易于集成
- ✅ **状态机**实现,高效解析
- ✅ 支持 C 语言的常见 token 类型
- ✅ 适合嵌入到自定义解释器/编译器中

在 `crepl.c` 中,它负责将用户输入字符串转换为结构化的 token 流,为后续的函数查找和动态调用提供基础。
