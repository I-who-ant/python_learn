# Crepl - C 函数交互式调用器

一个简单的应用程序,允许你从命令行动态调用共享库(.so/.dll)中的 C 函数,使用 [libffi](https://github.com/libffi/libffi) 实现运行时动态调用。这个项目是为了测试 libffi 而开发的娱乐性项目。

## 核心技术

- **动态链接**: 使用 `dlopen`/`dlsym` 加载共享库并查找函数
- **动态调用**: 使用 `libffi` 在运行时构造函数调用
- **词法分析**: 使用 `stb_c_lexer` 解析用户输入

## 快速开始

```console
# 1. 编译构建工具
$ cc -o nob nob.c

# 2. 使用构建工具编译 crepl
$ ./nob

# 3. 运行 crepl,加载 raylib 库
$ ./crepl ./raylib-5.5_linux_amd64/lib/libraylib.so

# 4. 交互式调用函数
> InitWindow 800 600 "Bajoding"   # 创建窗口
> BeginDrawing                     # 开始绘制
> ClearBackground 0xFF0000FF       # 清空背景(红色)
> EndDrawing                       # 结束绘制
```

## 工作原理

```
用户输入
   ↓
词法分析(stb_c_lexer) → 解析函数名和参数
   ↓
符号查找(dlsym) → 获取函数地址
   ↓
动态调用(libffi) → 构造调用并执行
   ↓
函数执行
```

## 文件说明

- `nob.c` - 构建脚本(使用 nob.h 编写)
- `nob.h` - 单头文件构建工具库
- `crepl.c` - REPL 主程序
- `stb_c_lexer.h` - C 词法分析器(单头文件库)
- `rect.js` - 示例:使用 Node.js 生成 crepl 命令

## 使用示例

### 调用 printf

```console
$ ./crepl /lib/x86_64-linux-gnu/libc.so.6
> printf "Hello, %s!\n" "World"
Hello, World!
> printf "Number: %d\n" 42
Number: 42
```

### 通过管道使用

```console
# 使用 Node.js 生成动画
$ node rect.js | ./crepl libraylib.so
```

## 限制

- 只支持基本类型:`int` 和 `char*`
- 不支持返回值
- 不支持结构体、数组、浮点数

## 学习价值

- 理解动态链接原理
- 理解 FFI(外部函数接口)机制
- 学习运行时类型系统
