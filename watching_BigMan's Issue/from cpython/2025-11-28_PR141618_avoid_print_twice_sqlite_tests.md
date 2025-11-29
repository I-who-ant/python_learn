# PR #141618 - 避免 SQLite 测试重复打印版本信息

**日期**: 2025-11-28
**PR 链接**: https://github.com/python/cpython/pull/141618
**Issue 链接**: https://github.com/python/cpython/issues/139743
**作者**: yihong0618
**状态**: Open (awaiting merge)

---

## 📋 基本信息

- **标题**: gh-139743: avoid print twice verbose version for sqlite tests
- **创建时间**: 2025-11-16
- **改动规模**: +8 行, -3 行
- **修改文件**: `Lib/test/test_sqlite3/__init__.py`
- **审查者**: berkerpeksag, erlend-aasland

---

## 🎯 问题描述

### 原始 Issue (#139743)

在 Python 3.15 中使用 pyrepl 时,当执行 `help()` 然后输入 `modules` 列出所有模块时,会出现一个意外的打印语句:

```
test_sqlite3: testing with SQLite version 3.51.0
```

**复现步骤**:
```sh
>>> help()
help> modules

Please wait a moment while I gather a list of all available modules...

test_sqlite3: testing with SQLite version 3.51.0  # ← 意外出现的输出
__future__          _sre                ensurepip           quopri
__hello__           _ssl                enum                random
...
```

这条消息会在模块列表中随机出现,影响用户体验和输出的清晰度。

---

## 🔍 根本原因

在 `Lib/test/test_sqlite3/__init__.py` 文件中,`load_tests()` 函数实现了 unittest 的 "load tests" 协议。在 verbose 模式下,该函数会打印 SQLite 版本信息:

```python
def load_tests(*args):
    if verbose:
        print(f"test_sqlite3: testing with SQLite version {sqlite3.sqlite_version}")
    pkg_dir = os.path.dirname(__file__)
    return load_package_tests(pkg_dir, *args)
```

**问题**: `load_tests()` 函数会被**多次调用**,导致版本信息被重复打印。

---

## 💡 解决方案演进

### 第一次尝试 (Commit 1)

尝试通过判断 `pattern` 参数是否为 `None` 来检测是否是顶层调用:

```python
def load_tests(loader, tests, pattern):
    # Only print on the top-level call
    if verbose and pattern is None:
        print(f"test_sqlite3: testing with SQLite version {sqlite3.sqlite_version}")
    pkg_dir = os.path.dirname(__file__)
    return load_package_tests(pkg_dir, loader, tests, pattern)
```

**问题**: 这个方法不够可靠,`pattern` 参数可能在多次调用中都为 `None`。

### 最终方案 (Commit 2)

使用**全局标志变量**来确保只打印一次:

```python
# make sure only print once
_printed_version = False

# Implement the unittest "load tests" protocol.
def load_tests(loader, tests, pattern):
    global _printed_version
    if verbose and not _printed_version:
        print(f"test_sqlite3: testing with SQLite version {sqlite3.sqlite_version}")
        _printed_version = True
    pkg_dir = os.path.dirname(__file__)
    return load_package_tests(pkg_dir, loader, tests, pattern)
```

---

## 🔑 技术要点

### 优点

1. **简单有效**: 使用全局标志是最直接的解决方案
2. **保持向后兼容**: 仍然在 verbose 模式下打印版本信息
3. **最小改动**: 只修改了必要的代码,没有引入复杂逻辑

### 考虑因素

1. **线程安全**: 全局变量 `_printed_version` 不是线程安全的
   - 但在测试框架的上下文中,通常是单线程执行
   - 这个权衡在当前场景下是合理的

2. **命名规范**: 使用 `_printed_version` (下划线前缀)表示这是模块私有变量

---

## 📊 效果对比

### 修复前

```bash
➜  cpython git:(main) ./python.exe -m test test_sqlite3 -v
...
test_sqlite3: testing with SQLite version 3.43.2
...
test_sqlite3: testing with SQLite version 3.43.2  # 重复出现!
...
```

### 修复后

```bash
➜  cpython git:(main) ./python.exe -m test test_sqlite3 -v
== CPython 3.15.0a1+ (heads/main:ed73c909f2, Nov 16 2025, 15:29:51) [Clang 17.0.0 (clang-1700.0.13.5)]
== macOS-15.3.2-arm64-arm-64bit-Mach-O little-endian
== Python build: release
== cwd: /Users/yihong/repos/cpython/build/test_python_worker_37275
== CPU count: 10
== encodings: locale=UTF-8 FS=utf-8
== resources: all test resources are disabled, use -u option to unskip tests

Using random seed: 643038146
0:00:00 load avg: 1.46 Run 1 test sequentially in a single process
0:00:00 load avg: 1.46 [1/1] test_sqlite3
test_sqlite3: testing with SQLite version 3.43.2  # 只出现一次!
test_bad_source_closed_connection (test.test_sqlite3.test_backup.BackupTests.test_bad_source_closed_connection) ... ok
```

---

## 📝 学习点

1. **测试框架协议**: unittest 的 `load_tests` 协议允许自定义测试加载行为
2. **副作用控制**: 在可能被多次调用的函数中打印信息需要谨慎处理
3. **迭代优化**: 作者从参数判断到全局标志,体现了良好的问题解决思路
4. **小改动大影响**: 虽然只改了几行代码,但显著改善了用户体验

---

## 🔗 相关链接

- **PR**: https://github.com/python/cpython/pull/141618
- **Issue**: https://github.com/python/cpython/issues/139743
- **Patch**: https://github.com/python/cpython/pull/141618.patch
- **Diff**: https://github.com/python/cpython/pull/141618.diff

---

## 📌 当前状态

- ✅ 代码审查中
- 🏷️ 标签: `awaiting merge`
- 👥 等待 berkerpeksag 和 erlend-aasland 的最终审查
- 📅 预计很快会被合并到 Python 3.15

---

**备注**: 这是一个典型的小型但重要的 bug 修复,展示了开源协作中发现问题、定位原因、提出方案、优化实现的完整流程。
