// ============================================// nob.c - 构建脚本(用 C 代码编写的构建系统)
// ============================================

// 1. 启用 nob.h 的实现代码(单头文件库模式)
#define NOB_IMPLEMENTATION

// 2. 去掉函数前缀(nob_cmd_append → cmd_append)
#define NOB_STRIP_PREFIX

// 3. 包含 nob.h 工具库(提供命令执行、文件操作、内存管理等功能)
#include "nob.h"

// ============================================
// 全局变量
// ============================================

// 命令对象,用于构建命令行参数
// 类似于数组: cmd = ["cc", "-Wall", "-o", "crepl", "crepl.c", "-lffi"]
Cmd cmd = {0};  // 零初始化

// ============================================
// cc() - 添加通用编译器选项
// ============================================
void cc(void)
{
    // 添加编译器名称
    cmd_append(&cmd, "cc");

    // 启用所有警告
    cmd_append(&cmd, "-Wall");     // 启用大部分警告
    cmd_append(&cmd, "-Wextra");   // 启用额外警告

    // 忽略"未使用函数"警告(因为 nob.h 有很多工具函数)
    cmd_append(&cmd, "-Wno-unused-function");

    // 生成 GDB 调试信息
    cmd_append(&cmd, "-ggdb");
}

// ============================================
// main() - 主函数
// ============================================
int main(int argc, char **argv)
{
    // ========================================
    // Go Rebuild Urself™ 技术
    // ========================================
    // 自动检测 nob.c 是否被修改:
    //   - 如果 nob.c 比 nob 可执行文件新 → 重新编译 nob 并重新执行
    //   - 确保构建工具始终是最新版本
    NOB_GO_REBUILD_URSELF(argc, argv);

    // ========================================
    // 构建 crepl
    // ========================================

    // 1. 添加基础编译选项(cc -Wall -Wextra -Wno-unused-function -ggdb)
    cc();

    // 2. 指定输出文件名: -o crepl
    cmd_append(&cmd, "-o", "crepl");

    // 3. 指定源文件: crepl.c
    cmd_append(&cmd, "crepl.c");

    // 4. 链接 libffi 库: -lffi
    //    libffi 提供动态函数调用能力
    cmd_append(&cmd, "-lffi");

    // 5. 执行编译命令
    //    最终命令: cc -Wall -Wextra -Wno-unused-function -ggdb -o crepl crepl.c -lffi
    if (!cmd_run(&cmd)) return 1;  // 如果失败,返回错误码 1

    // 构建成功
    return 0;
}
