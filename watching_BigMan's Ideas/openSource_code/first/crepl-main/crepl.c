// ============================================
// crepl.c - C 语言 REPL(交互式函数调用器)
// ============================================
// 功能: 无需编译,直接从命令行调用共享库(.so)中的函数
// 原理: dlopen(动态加载) + libffi(动态调用) + stb_c_lexer(解析参数)
// ============================================

// ========== 标准库和依赖 ==========
#include <stdio.h>   // printf, fgets 等标准输入输出
#include <dlfcn.h>   // dlopen, dlsym - 动态链接库操作

// ========== nob.h - 构建工具库 ==========
#define NOB_IMPLEMENTATION  // 启用实现代码
#define NOB_STRIP_PREFIX    // 去掉 nob_ 前缀
#include "nob.h"            // 提供: shift, temp_alloc, da_append 等工具

// ========== libffi - 动态函数调用库 ==========
#include <ffi.h>  // ffi_cif, ffi_type, ffi_prep_cif, ffi_call

// ========== stb_c_lexer - C 词法分析器 ==========
#define STB_C_LEXER_IMPLEMENTATION  // 启用实现代码
#include "stb_c_lexer.h"            // 解析用户输入的函数名和参数

// ============================================
// 全局变量
// ============================================

// 用户输入缓冲区(最多 1024 字符)
char line[1024];

// ========== 函数指针类型定义 ==========
// 用于存储 dlsym 返回的函数地址
// void (*fn_t)(void) 表示: 指向"无参数、无返回值函数"的指针
typedef void (*fn_t)(void);

// ============================================
// 数据结构定义
// ============================================

// ========== Types - 参数类型数组 ==========
// 存储函数参数的 ffi_type 类型信息
// 示例: [&ffi_type_sint32, &ffi_type_pointer]
typedef struct {
    ffi_type **items;  // ffi_type 指针数组
    size_t count;      // 当前元素个数
    size_t capacity;   // 数组容量
} Types;

// ========== Values - 参数值数组 ==========
// 存储函数参数的实际值(的地址)
// 示例: [&int_var, &str_ptr]
// 注意: 存储的是"指向值的指针",不是值本身!
typedef struct {
    void **items;     // void 指针数组
    size_t count;     // 当前元素个数
    size_t capacity;  // 数组容量
} Values;

// ============================================
// main() - 主函数
// ============================================
int main(int argc, char **argv)
{
    // ========================================
    // 第 1 步: 解析命令行参数
    // ========================================
    // argc :  命令行参数的数量，它是一个整数
    // shift() 弹出第一个参数(程序名)
    // 示例: ./crepl libraylib.so
    //   执行前: argv = ["crepl", "libraylib.so"]
    //   执行后: argv = ["libraylib.so"], program_name = "crepl"
    const char *program_name = shift(argv, argc);

    // 检查是否提供了动态库路径
    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }

    // 获取动态库路径参数
    // 示例: dll_path = "./raylib-5.5_linux_amd64/lib/libraylib.so"
    const char *dll_path = shift(argv, argc);

    // ========================================
    // 第 2 步: 加载动态库
    // ========================================

    // dlopen() - 加载共享库到内存
    //   参数 1: dll_path - 库文件路径
    //   参数 2: RTLD_NOW - 立即解析所有符号(vs RTLD_LAZY 延迟解析)
    //   返回值: 库句柄(void*),失败返回 NULL
    //
    // 内部流程:
    //   1. 读取 ELF 文件
    //   2. 映射到内存
    //   3. 解析符号表
    //   4. 执行初始化函数(.init)
    void *dll = dlopen(dll_path, RTLD_NOW);
    if (dll == NULL) {
        // dlerror() 返回详细错误信息
        fprintf(stderr, "ERROR: %s\n", dlerror());
        return 1;
    }

    // ========================================
    // 第 3 步: 初始化数据结构
    // ========================================

    // ffi_cif - Call Interface(调用接口描述符)
    //   存储函数签名信息: 参数类型、返回值类型、ABI 等
    ffi_cif cif = {0};

    // args - 参数类型数组
    //   示例: [&ffi_type_sint32, &ffi_type_pointer]
    Types args = {0};

    // values - 参数值数组
    //   示例: [&int_var, &str_ptr]
    Values values = {0};

    // mark - 临时内存池标记
    //   用于每次循环开始时重置内存池
    size_t mark = temp_save();

    // l - 词法分析器状态
    stb_lexer l = {0};

    // string_store - 字符串字面量存储缓冲区
    //   词法分析器将解析出的字符串存储在这里
    static char string_store[1024];

    // ========================================
    // 第 4 步: REPL 主循环
    // ========================================
    // REPL = Read-Eval-Print Loop(读取-执行-打印-循环)
    for (;;) {
next:
        // ========== 重置临时内存池 ==========
        // 每次循环开始时,回收上一次循环分配的所有临时内存
        // 避免内存泄漏,提高性能
        temp_rewind(mark);

        // ========== 读取用户输入 ==========
        printf("> ");  // 打印提示符
        // fgets() 读取一行输入(包括 \n)
        //   参数 1: line - 存储缓冲区
        //   参数 2: sizeof(line) - 最多读取 1024 字节
        //   参数 3: stdin - 从标准输入读取
        //   返回值: NULL 表示 EOF(Ctrl+D)
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        fflush(stdout);  // 刷新输出缓冲区

        // ========== 去除首尾空白 ==========
        // sv_from_cstr() 将 C 字符串转换为 String_View
        // sv_trim() 去除首尾空白字符
        //  和 sv_from_cstr 一样，sv_trim 通常也不会分配新内存或复制数据
        //
        String_View sv = sv_trim(sv_from_cstr(line));

        // ========== 初始化词法分析器 ==========
        // 将用户输入传递给词法分析器
        //   参数 1: &l - 词法分析器状态
        //   参数 2: sv.data - 输入字符串起始位置
        //   参数 3: sv.data + sv.count - 输入字符串结束位置
        //   参数 4: string_store - 字符串字面量存储缓冲区
        //   参数 5: ARRAY_LEN(string_store) - 缓冲区大小

        stb_c_lexer_init(&l, sv.data, sv.data + sv.count, string_store, ARRAY_LEN(string_store));

        // ========== 解析函数名 ==========
        // stb_c_lexer_get_token() 获取下一个 token
        //   返回值: 0 表示没有 token(空行)
        if (!stb_c_lexer_get_token(&l)) continue;

        // 检查第一个 token 是否为标识符(函数名)
        // l.token == CLEX_id 表示标识符
        // 其他可能的 token: CLEX_intlit(整数), CLEX_dqstring(字符串)等
        if (l.token != CLEX_id) {
            printf("ERROR: function name must be an identifier\n");
            continue;
        }

        // ========== 查找函数符号 ==========
        // dlsym() - 根据函数名查找函数地址
        //   参数 1: dll - 库句柄(dlopen 返回的)
        //   参数 2: l.string - 函数名字符串(词法分析器解析出的)
        //   返回值: 函数地址(void*),失败返回 NULL
        //
        // 示例:
        //   输入: InitWindow
        //   返回: libraylib.so 中 InitWindow 函数的地址(如 0x00001234)
        //
        // 原理: 查找动态库的符号表(.dynsym 段)
        void *fn = dlsym(dll, l.string);
        if (fn == NULL) {
            printf("ERROR: no function %s found\n", l.string);
            continue;
        }

        // ========== 清空参数列表 ==========
        // 为什么只设置 count = 0 而不释放内存?
        //   - 保留 capacity,避免重复分配
        //   - 下次 da_append() 会覆盖旧值
        args.count = 0;
        values.count = 0;

        // ========================================
        // 第 5 步: 解析参数
        // ========================================
        // 示例输入: InitWindow 800 600 "Hello"
        //   Token 1: CLEX_id "InitWindow" (已处理)
        //   Token 2: CLEX_intlit 800
        //   Token 3: CLEX_intlit 600
        //   Token 4: CLEX_dqstring "Hello"
        //
        // 目标: 构建两个数组
        //   args:   [&ffi_type_sint32, &ffi_type_sint32, &ffi_type_pointer]
        //   values: [&800,              &600,              &"Hello"         ]

        while (stb_c_lexer_get_token(&l)) {
            switch (l.token) {
            // ========== 处理整数字面量 ==========
            case CLEX_intlit: {
                // 步骤 1: 记录参数类型为 int (ffi_type_sint32)
                da_append(&args, &ffi_type_sint32);

                // 步骤 2: 从临时内存池分配空间存储 int 值
                int *x = temp_alloc(sizeof(int));

                // 步骤 3: 存储实际值(词法分析器解析出的整数)
                *x = l.int_number;

                // 步骤 4: 保存值的地址到 values 数组
                //   注意: 存的是 int* 类型,即 &x
                da_append(&values, x);
            } break;

            // ========== 处理字符串字面量 ==========
            case CLEX_dqstring: {
                // 步骤 1: 记录参数类型为指针 (ffi_type_pointer)
                da_append(&args, &ffi_type_pointer);

                // 步骤 2: 分配存储 char* 的空间
                //   为什么是 char**?
                //   - 因为 ffi_call 需要的是"指向参数值的指针"
                //   - 对于 char* 参数,需要传递 &(char*) = char**
                char **x = temp_alloc(sizeof(char*));

                // 步骤 3: 复制字符串到临时内存池
                //   temp_strdup() 从临时内存池分配并复制字符串
                *x = temp_strdup(l.string);

                // 步骤 4: 保存 char** 的地址到 values 数组
                da_append(&values, x);
            } break;

            // ========== 处理无效 token ==========
            default:
                printf("ERROR: invalid argument token\n");
                // goto next 跳到外层循环的开始(第 138 行)
                goto next;
            }
        }

        // ========================================
        // 第 6 步: 准备 FFI 调用接口
        // ========================================
        // ffi_prep_cif() - 准备调用接口描述符
        //   参数 1: &cif - 输出,准备好的调用接口
        //   参数 2: FFI_DEFAULT_ABI - 使用默认 ABI(应用二进制接口)
        //           在 x86-64 上是 System V ABI
        //   参数 3: args.count - 参数个数(如 3)
        //   参数 4: &ffi_type_void - 返回值类型(void,无返回值)
        //   参数 5: args.items - 参数类型数组
        //
        // ffi_prep_cif() 内部做了什么?
        //   1. 检查 ABI 有效性
        //   2. 计算每个参数的大小和对齐
        //   3. 决定参数传递方式:
        //      x86-64 System V ABI:
        //        参数 1-6 通过寄存器传递: RDI, RSI, RDX, RCX, R8, R9
        //        参数 7+ 通过栈传递
        //   4. 计算栈空间需求
        //   5. 生成调用桩代码
        //
        // 示例(InitWindow 800 600 "Hello"):
        //   参数 1 (char*) → RDI 寄存器
        //   参数 2 (int)   → RSI 寄存器
        //   参数 3 (int)   → RDX 寄存器
        ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.count, &ffi_type_void, args.items);
        if (status != FFI_OK) {
            printf("ERROR: could not create cif\n");
            continue;
        }

        // ========================================
        // 第 7 步: 执行函数调用(魔法发生的地方!)
        // ========================================
        // ffi_call() - 执行动态函数调用
        //   参数 1: &cif - 调用接口描述符(包含参数类型、ABI 等信息)
        //   参数 2: fn - 函数指针(dlsym 返回的地址)
        //   参数 3: NULL - 返回值存储位置(NULL = 忽略返回值)
        //   参数 4: values.items - 参数值数组
        //
        // ffi_call() 内部流程:
        //
        //   1. 加载参数到寄存器/栈(根据 ABI):
        //      mov rdi, [values.items[0]]  ; "Hello"
        //      mov esi, [values.items[1]]  ; 800
        //      mov edx, [values.items[2]]  ; 600
        //
        //   2. 执行函数调用:
        //      call [fn]  ; call 0x00001234 (InitWindow 的地址)
        //
        //   3. 获取返回值(如果需要):
        //      mov [rvalue], rax  ; 从 RAX 获取返回值
        //
        // 等价于:
        //   InitWindow("Hello", 800, 600);
        //
        // 关键点:
        //   - libffi 根据 cif 中的类型信息,动态生成汇编代码
        //   - 无需在编译时知道函数签名
        //   - 运行时构造函数调用,类似于 Python 的反射
        ffi_call(&cif, fn, NULL, values.items);
    }

    // ========================================
    // 退出 REPL
    // ========================================
    printf("Quit\n");

    return 0;
}
// ============================================
// 完整执行流程示例
// ============================================
// 用户输入: InitWindow 800 600 "Hello"
//
// 1. 词法分析:
//    Token 1: CLEX_id "InitWindow"
//    Token 2: CLEX_intlit 800
//    Token 3: CLEX_intlit 600
//    Token 4: CLEX_dqstring "Hello"
//
// 2. 查找函数:
//    fn = dlsym(dll, "InitWindow") = 0x00001234
//
// 3. 构造参数类型:
//    args = [&ffi_type_sint32, &ffi_type_sint32, &ffi_type_pointer]
//
// 4. 构造参数值:
//    values = [&800, &600, &"Hello"]
//
// 5. 准备调用接口:
//    ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 3, &ffi_type_void, args)
//    → 决定: RDI, RSI, RDX
//
// 6. 执行调用:
//    ffi_call(&cif, 0x00001234, NULL, values)
//    → 生成汇编:
//       mov rdi, "Hello"
//       mov esi, 800
//       mov edx, 600
//       call 0x00001234
//
// 7. InitWindow 执行(在 libraylib.so 中)
// ============================================
