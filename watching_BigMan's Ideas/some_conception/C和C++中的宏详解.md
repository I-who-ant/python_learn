# C和C++中的宏详解

## 目录
- [什么是宏](#什么是宏)
- [宏的基本类型](#宏的基本类型)
- [宏的工作原理](#宏的工作原理)
- [常用宏示例](#常用宏示例)
- [宏的高级用法](#宏的高级用法)
- [宏的陷阱与注意事项](#宏的陷阱与注意事项)
- [宏与内联函数对比](#宏与内联函数对比)
- [实战应用场景](#实战应用场景)

## 什么是宏

宏(Macro)是C/C++预处理器在编译前进行文本替换的机制。它不是C/C++语言本身的一部分,而是**预处理器指令**。

### 核心特点
1. **编译前展开**: 在编译阶段之前,预处理器会将宏替换为实际代码
2. **文本替换**: 纯粹的字符串替换,不进行类型检查
3. **无作用域**: 宏定义从定义处到文件结束(或`#undef`)都有效
4. **零运行时开销**: 因为是编译期替换,运行时无额外开销

### 预处理流程
```
源代码(.c/.cpp) → 预处理器(处理宏/include) → 编译器 → 汇编器 → 链接器 → 可执行文件
```

## 宏的基本类型

### 1. 对象宏(Object-like Macro)
定义常量或简单替换:

```c
#define PI 3.14159
#define MAX_SIZE 1024
#define BUFFER_SIZE (MAX_SIZE * 2)  // 可以使用其他宏

// 使用
double area = PI * r * r;
char buffer[MAX_SIZE];
```

### 2. 函数宏(Function-like Macro)
带参数的宏,类似函数:

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// 使用
int max = MAX(10, 20);           // 展开为: ((10) > (20) ? (10) : (20))
int sq = SQUARE(5);              // 展开为: ((5) * (5))
```

### 3. 预定义宏
编译器自动定义的宏:

```c
__FILE__      // 当前文件名
__LINE__      // 当前行号
__DATE__      // 编译日期 "Mmm dd yyyy"
__TIME__      // 编译时间 "hh:mm:ss"
__FUNCTION__  // 当前函数名(C99/C++)
__STDC__      // 是否符合标准C

// 调试示例
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

DEBUG_PRINT("Error: value = %d", x);
// 输出: [test.c:42] Error: value = 10
```

## 宏的工作原理

### 展开过程

```c
#define DOUBLE(x) ((x) + (x))
int y = DOUBLE(5);

// 预处理后变成:
int y = ((5) + (5));
```

### 多级展开

```c
#define A 2
#define B A * 3
#define C B + 1

int x = C;  // 展开为: 2 * 3 + 1 = 7
```

### 字符串化操作符 `#`

将宏参数转换为字符串:

```c
#define STR(x) #x

printf("%s\n", STR(hello));   // 输出: hello
printf("%s\n", STR(123));     // 输出: 123
printf("%s\n", STR(a + b));   // 输出: a + b
```

### 连接操作符 `##`

连接两个token:

```c
#define CONCAT(a, b) a##b

int xy = 10;
int result = CONCAT(x, y);  // 展开为: xy,即访问变量xy

// 生成变量名
#define MAKE_VAR(name, num) name##num
int MAKE_VAR(var, 1) = 100;  // 定义 var1 = 100
int MAKE_VAR(var, 2) = 200;  // 定义 var2 = 200
```

## 常用宏示例

### 1. 数学运算宏

```c
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define CLAMP(x, min, max) (MIN(MAX(x, min), max))

// 交换两个值(注意:要求相同类型)
#define SWAP(a, b) do { \
    typeof(a) _tmp = (a); \
    (a) = (b); \
    (b) = _tmp; \
} while(0)
```

### 2. 数组操作宏

```c
// 获取数组元素个数
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int nums[] = {1, 2, 3, 4, 5};
size_t count = ARRAY_SIZE(nums);  // 5

// 遍历数组
#define FOR_EACH(type, item, array) \
    for (type *item = (array); \
         item < (array) + ARRAY_SIZE(array); \
         ++item)

FOR_EACH(int, p, nums) {
    printf("%d ", *p);
}
```

### 3. 内存管理宏

```c
// 安全的malloc
#define MALLOC(type, count) ((type*)malloc(sizeof(type) * (count)))

int *arr = MALLOC(int, 100);  // 分配100个int

// 安全的free
#define SAFE_FREE(ptr) do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while(0)
```

### 4. 调试宏

```c
#ifdef DEBUG
    #define DBG(fmt, ...) \
        fprintf(stderr, "[DEBUG %s:%d] " fmt "\n", \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DBG(fmt, ...) ((void)0)  // Release版本不输出
#endif

// 断言宏
#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "Assertion failed: %s\n  File: %s, Line: %d\n", \
                msg, __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

DBG("x = %d", x);
ASSERT(x > 0, "x must be positive");
```

### 5. 位操作宏

```c
#define BIT(n) (1U << (n))                    // 获取第n位的掩码
#define SET_BIT(x, n) ((x) |= BIT(n))        // 设置第n位为1
#define CLEAR_BIT(x, n) ((x) &= ~BIT(n))     // 清除第n位
#define TOGGLE_BIT(x, n) ((x) ^= BIT(n))     // 翻转第n位
#define CHECK_BIT(x, n) (((x) & BIT(n)) != 0) // 检查第n位

unsigned int flags = 0;
SET_BIT(flags, 3);      // flags = 0b1000
TOGGLE_BIT(flags, 1);   // flags = 0b1010
```

## 宏的高级用法

### 1. 可变参数宏

```c
// C99标准
#define LOG(fmt, ...) printf("[LOG] " fmt "\n", ##__VA_ARGS__)

LOG("Hello");              // [LOG] Hello
LOG("x = %d", 10);         // [LOG] x = 10

// 带等级的日志
#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    fprintf(stderr, "[WARN] " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    printf("[INFO] " fmt "\n", ##__VA_ARGS__)
```

### 2. do-while(0) 技巧

让宏像函数一样使用,支持分号结尾:

```c
// 错误示例
#define BAD_MACRO(x) \
    statement1; \
    statement2

if (condition)
    BAD_MACRO(x);  // 只有statement1在if内!
else
    something;

// 正确示例
#define GOOD_MACRO(x) do { \
    statement1; \
    statement2; \
} while(0)

if (condition)
    GOOD_MACRO(x);  // 现在两个语句都在if内
else
    something;
```

### 3. X-Macro 技术

用于减少重复代码:

```c
// 定义颜色列表
#define COLOR_LIST \
    X(RED, 0xFF0000) \
    X(GREEN, 0x00FF00) \
    X(BLUE, 0x0000FF) \
    X(YELLOW, 0xFFFF00)

// 生成枚举
#define X(name, value) COLOR_##name,
enum Color {
    COLOR_LIST
};
#undef X

// 生成字符串数组
#define X(name, value) #name,
const char *color_names[] = {
    COLOR_LIST
};
#undef X

// 生成值数组
#define X(name, value) value,
unsigned int color_values[] = {
    COLOR_LIST
};
#undef X
```

### 4. 条件编译

```c
#define PLATFORM_LINUX

#ifdef PLATFORM_LINUX
    #define PATH_SEP '/'
    #include <unistd.h>
#elif defined(PLATFORM_WINDOWS)
    #define PATH_SEP '\\'
    #include <windows.h>
#else
    #error "Unsupported platform"
#endif

// 版本控制
#define VERSION_MAJOR 2
#define VERSION_MINOR 3
#define VERSION_PATCH 1

#if VERSION_MAJOR >= 2
    // 使用新API
#else
    // 使用旧API
#endif
```

### 5. 类型安全宏(C++11)

```cpp
// C++11 的类型安全宏
#define TYPE_SAFE_MAX(a, b) ({ \
    auto _a = (a); \
    auto _b = (b); \
    _a > _b ? _a : _b; \
})

// 或使用constexpr
#define CONSTEXPR_MAX(a, b) \
    (((a) > (b)) ? (a) : (b))
```

## 宏的陷阱与注意事项

### 1. 参数重复求值

```c
#define SQUARE(x) ((x) * (x))

int a = 5;
int result = SQUARE(a++);  // 展开为: ((a++) * (a++))
// a会被递增两次!结果可能不是预期的

// 修复:使用临时变量(但失去宏的优势)
#define SAFE_SQUARE(x) ({ \
    typeof(x) _x = (x); \
    _x * _x; \
})
```

### 2. 运算符优先级问题

```c
// 错误示例
#define MULTIPLY(a, b) a * b

int result = MULTIPLY(2 + 3, 4 + 5);
// 展开为: 2 + 3 * 4 + 5 = 2 + 12 + 5 = 19 (不是25!)

// 正确:总是加括号
#define MULTIPLY(a, b) ((a) * (b))
// 展开为: ((2 + 3) * (4 + 5)) = 25
```

### 3. 副作用问题

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int max = MAX(func1(), func2());
// func1或func2可能被调用两次!

// 如果函数有副作用(如修改全局变量、I/O操作),结果不可预测
```

### 4. 类型检查缺失

```c
#define ADD(a, b) ((a) + (b))

int x = ADD(10, 20);      // OK
int y = ADD("hello", 5);  // 编译期不报错,运行时崩溃!

// 内联函数提供类型检查:
static inline int add(int a, int b) {
    return a + b;
}
```

### 5. 调试困难

```c
#define COMPLEX_MACRO(x) \
    do_something(x); \
    do_another(x); \
    final_step(x)

// 调试时无法单步进入宏内部
// 错误信息指向展开后的代码,难以定位
```

### 6. 作用域问题

```c
#define TEMP_VAR tmp

void func1() {
    int TEMP_VAR = 10;  // 定义 tmp
}

void func2() {
    int TEMP_VAR = 20;  // 又定义了 tmp,可能冲突
}
```

## 宏与内联函数对比

| 特性 | 宏 | 内联函数(C++/C99) |
|------|-----|------------------|
| **类型检查** | 无 | 有 |
| **调试** | 困难 | 容易 |
| **副作用** | 可能重复求值 | 参数只求值一次 |
| **递归** | 不支持 | 支持 |
| **命名空间** | 无作用域 | 遵循作用域规则 |
| **编译期常量** | 是 | 不一定 |
| **代码大小** | 可能膨胀 | 编译器优化 |
| **泛型** | 天然支持 | C++需要模板 |

### 示例对比

```c
// 宏版本
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// 内联函数版本(C99)
static inline int max_int(int a, int b) {
    return a > b ? a : b;
}

// C++模板版本
template<typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}
```

### 选择建议

**使用宏的场景**:
- 需要编译期常量(数组大小等)
- 条件编译
- 字符串化/token连接
- 需要泛型但不想用模板(C语言)

**使用内联函数的场景**:
- 需要类型检查
- 需要调试
- 复杂逻辑
- 避免副作用

## 实战应用场景

### 1. Linux内核风格宏

```c
// 容器获取宏(container_of)
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
})

// 链表操作
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

// 错误处理
#define IS_ERR(ptr) ((unsigned long)(ptr) > (unsigned long)(-1000))
```

### 2. 嵌入式系统宏

```c
// 寄存器操作
#define REG32(addr) (*(volatile unsigned int *)(addr))
#define REG16(addr) (*(volatile unsigned short *)(addr))
#define REG8(addr) (*(volatile unsigned char *)(addr))

// 使用
REG32(0x40000000) = 0xFF;  // 写寄存器

// 位域定义
#define GPIO_PIN_0  BIT(0)
#define GPIO_PIN_1  BIT(1)

// 外设控制
#define ENABLE_UART()  SET_BIT(RCC_APB1ENR, RCC_UART_EN)
```

### 3. 性能测量宏

```c
#include <time.h>

#define MEASURE_TIME(code) do { \
    clock_t start = clock(); \
    code; \
    clock_t end = clock(); \
    printf("Time: %f seconds\n", \
           (double)(end - start) / CLOCKS_PER_SEC); \
} while(0)

// 使用
MEASURE_TIME({
    for (int i = 0; i < 1000000; i++) {
        expensive_function();
    }
});
```

### 4. 单元测试宏

```c
#define TEST(name) \
    void test_##name(); \
    void test_##name()

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s:%d: %s != %s\n", \
               __FILE__, __LINE__, #a, #b); \
        exit(1); \
    } \
} while(0)

// 使用
TEST(addition) {
    ASSERT_EQ(add(2, 3), 5);
    ASSERT_EQ(add(-1, 1), 0);
}
```

### 5. 智能指针模拟(C语言)

```c
#define SMART_PTR(type) \
    typedef struct { \
        type *ptr; \
        size_t *ref_count; \
    } smart_##type

#define SMART_NEW(type, var) \
    smart_##type var = { \
        .ptr = malloc(sizeof(type)), \
        .ref_count = malloc(sizeof(size_t)) \
    }; \
    *(var.ref_count) = 1

#define SMART_FREE(var) do { \
    if (--(*var.ref_count) == 0) { \
        free(var.ptr); \
        free(var.ref_count); \
    } \
} while(0)
```

## 最佳实践

1. **总是使用括号**保护参数和整个表达式
2. **宏名使用大写**,与普通标识符区分
3. **复杂逻辑使用do-while(0)**包装
4. **避免在宏中使用return/break/continue**
5. **文档化副作用**,警告参数可能被多次求值
6. **优先考虑const/inline**,宏是最后选择
7. **使用`#undef`**及时清理临时宏
8. **避免嵌套宏定义**,降低复杂度

## 总结

宏是C/C++的强大工具,但也是双刃剑:
- ✅ 优势: 零开销、编译期计算、代码生成、条件编译
- ❌ 劣势: 无类型检查、调试困难、易出错、代码膨胀

现代C++倾向于用**constexpr**、**template**、**inline**替代宏,但在C语言和底层编程中,宏仍是不可或缺的利器。

理解宏的本质——**预处理期的文本替换**,是掌握它的关键。
