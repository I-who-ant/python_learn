# dlfcn.h - 动态链接库接口

## 概述
`<dlfcn.h>` 是 Linux/POSIX 系统中用于动态加载共享对象（Shared Objects，.so 文件）的标准头文件。它提供了在运行时加载和使用动态链接库的能力。

## 核心函数

### 1. dlopen - 打开动态链接库

```c
void *dlopen(const char *filename, int flag);
```

**功能**：打开一个动态链接库，并返回一个句柄（handle）。

**参数**：
- `filename`: 动态库的文件路径
  - 可以是完整路径：`"/path/to/libfoo.so"`
  - 可以是库名称：`"libfoo.so"`（在标准路径中搜索）
  - 可以是 `NULL`：表示打开当前程序本身

- `flag`: 加载模式标志
  - `RTLD_NOW`: 立即解析所有符号（推荐）
  - `RTLD_LAZY`: 延迟解析符号（按需解析）
  - `RTLD_GLOBAL`: 使符号对后续加载的库可用
  - `RTLD_LOCAL`: 符号仅对当前库可用（默认）

**返回值**：
- 成功：返回库的句柄（void *指针）
- 失败：返回 `NULL`

**示例**：


```c

// 打开 raylib 库
void *lib = dlopen("raylib-5.5_linux_amd64/lib/libraylib.so", RTLD_NOW);

// 打开当前程序本身
void *lib = dlopen(NULL, RTLD_NOW);
```

---

### 2. dlsym - 查询符号地址

```c
void *dlsym(void *handle, const char *symbol);
```

**功能**：在动态库中查找指定符号（函数或变量）的地址。

**参数**：
- `handle`: 由 `dlopen` 返回的库句柄
- `symbol`: 要查找的符号名称（字符串）

**返回值**：
- 成功：返回符号的地址
- 失败：返回 `NULL`

**使用技巧**：
1. **查询函数**：需要使用函数指针来接收地址
2. **类型转换**：需要将 `void*` 转换为正确的函数指针类型

**示例**：

```c
// 基本用法 - 查询 malloc 函数
void *malloc_lib = dlsym(lib, "malloc");
printf("malloc_lib = %p\n", malloc_lib);

// 函数指针用法 - 查询 InitWindow 函数
void (*InitWindow)(int width, int height, const char *title);
InitWindow = dlsym(lib, "InitWindow");

// 现在可以像普通函数一样调用
if (InitWindow) {
    InitWindow(800, 600, "My Window");
}
```

---

### 3. dlclose - 关闭动态链接库

```c
int dlclose(void *handle);
```

**功能**：关闭由 `dlopen` 打开的动态库，释放资源。

**参数**：
- `handle`: 要关闭的库句柄

**返回值**：
- 成功：返回 0
- 失败：返回非零值



**示例**：

```c
dlclose(lib);
```

---

### 4. dlerror - 获取错误信息

```c
char *dlerror(void);
```

**功能**：返回最近一次 dl* 函数调用失败的错误信息。

**返回值**：
- 有错误：返回错误描述字符串
- 无错误：返回 `NULL`

**示例**：
```c
void *lib = dlopen("nonexistent.so", RTLD_NOW);
if (!lib) {
    fprintf(stderr, "Error: %s\n", dlerror());
}
```

---

## 完整示例

```c
#include <stdio.h>
#include <dlfcn.h>

int main() {
    // 1. 打开动态库
    void *lib = dlopen(NULL, RTLD_NOW);
    if (!lib) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    // 2. 查询 malloc 符号
    void *malloc_lib = dlsym(lib, "malloc");
    if (!malloc_lib) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(lib);
        return 1;
    }

    // 3. 比较地址
    printf("malloc_lib = %p\n", malloc_lib);
    printf("&malloc    = %p\n", &malloc);

    // 4. 关闭库
    dlclose(lib);

    return 0;
}
```

---

## 编译说明

使用 `dlfcn.h` 时需要链接 `dl` 库：

```bash
# 基本编译
gcc -o program program.c -ldl

# 或者使用 cc
cc -o program program.c -ldl
```

---

## 常见用途

1. **插件系统**：动态加载插件模块
2. **版本检测**：运行时检测库的可用性
3. **热更新**：无需重启程序即可更新功能模块
4. **依赖延迟加载**：按需加载可选依赖
5. **程序自省**：程序查询自身的符号表

---

## 注意事项

⚠️ **符号可见性**：
- 默认情况下，主程序中的符号（如 `main` 函数）不会导出到动态符号表
- 需要使用 `-rdynamic` 编译选项来导出符号

⚠️ **线程安全**：
- `dlerror()` 不是线程安全的

⚠️ **命名约定**：
- Linux 下动态库命名通常为 `libXXX.so`
- 加载时可以使用完整名称或简短名称
