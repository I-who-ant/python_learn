# Emacs 模式系统完全解析:Major Mode 与 Minor Mode

## 目录
1. [模式系统概述](#模式系统概述)
2. [Major Mode 详解](#major-mode-详解)
3. [Minor Mode 详解](#minor-mode-详解)
4. [模式的激活与切换](#模式的激活与切换)
5. [常用模式实战](#常用模式实战)
6. [自定义模式](#自定义模式)

---

## 模式系统概述

### 什么是 Mode(模式)?

**Mode** 是 Emacs 的核心概念,定义了编辑器在特定环境下的**行为、快捷键、语法高亮、自动缩进**等特性。

### 类比理解

```
Mode 就像手机的不同"场景模式":
- 静音模式 → Minor Mode(可叠加)
- 驾驶模式 → Minor Mode(可叠加)
- Android系统 → Major Mode(只能选一个)
```

### 模式的两大类

```
Emacs Buffer
    │
    ├─ Major Mode (主模式) ← 必须有且只有一个
    │   ├─ Fundamental Mode (基础模式)
    │   ├─ Text Mode (文本模式)
    │   ├─ Python Mode (Python编程模式)
    │   ├─ C Mode (C编程模式)
    │   └─ Org Mode (组织笔记模式)
    │
    └─ Minor Modes (辅助模式) ← 可以有多个
        ├─ Line Number Mode (显示行号)
        ├─ Auto Fill Mode (自动换行)
        ├─ Highlight Line Mode (高亮当前行)
        └─ ...可以同时开启多个
```

**关键区别:**
| 维度 | Major Mode | Minor Mode |
|------|-----------|------------|
| 数量 | **有且只有 1 个** | **可以有多个** |
| 作用 | 定义编辑环境 | 提供辅助功能 |
| 示例 | Python Mode, Text Mode | 行号显示, 自动补全 |
| 互斥性 | 互斥(切换会替换) | 可叠加 |

---

## Major Mode 详解

### Major Mode 的职责

1. **语法高亮** - 关键字、字符串、注释的颜色
2. **缩进规则** - 按 Tab 时如何缩进
3. **快捷键绑定** - 特定模式的快捷键
4. **注释方式** - `M-;` 时插入什么注释符
5. **自动补全** - 补全规则和候选项

### 常见 Major Modes

#### 1. Fundamental Mode (基础模式)

**描述:** Emacs 的**最基础**模式,几乎没有任何特殊功能。

**何时使用:**
- 临时的 *scratch* buffer
- 不确定文件类型时
- 需要最干净的编辑环境

**实战操作:**
```
1. 创建新buffer
   C-x b test [回车]

2. 查看当前模式
   M-x describe-mode (或 C-h m)
   显示: Fundamental mode

3. 特点:
   - 没有语法高亮
   - 没有自动缩进
   - 只有最基本的编辑功能
```

**适用场景:**
```
✓ 快速记事
✓ 临时粘贴文本
✗ 编写代码
✗ 编写结构化文档
```

---

#### 2. Text Mode (文本模式)

**描述:** 用于编辑**普通文本**的模式,是 Fundamental Mode 的增强版。

**相比 Fundamental Mode 的增强:**
- 自动换行(Auto Fill)
- 段落识别
- 大小写转换快捷键
- 简单的填充命令

**实战操作:**
```
1. 创建文本文件
   C-x C-f ~/test.txt

2. 自动进入 Text Mode
   查看模式行: (Text)

3. 测试段落功能:
   输入长文本...
   M-q (重新填充段落,自动换行到80列)
```

**Text Mode 特有功能:**
```
M-s     居中当前行
M-q     重新填充段落(自动换行)
M-^     合并到上一行
```

**示例对比:**
```
【Fundamental Mode】
This is a very very very very very very long line that will not wrap automatically.

【Text Mode + Auto Fill】
This is a very very very very
very very long line that will
wrap automatically at 80 columns.
```

**适用场景:**
```
✓ 写 README.md
✓ 写文档、笔记
✓ 写邮件内容
✗ 编程
```

---

#### 3. 编程语言模式

##### Python Mode

**特性:**
- Python 语法高亮
- 自动缩进(4空格)
- 代码折叠
- 运行 Python 代码
- 符合 PEP 8 的缩进

**实战操作:**
```
1. 创建 Python 文件
   C-x C-f ~/test.py

2. 自动进入 Python Mode
   模式行显示: (Python)

3. 输入代码测试:
def hello(name):
    print(f"Hello, {name}!")  ← "print" 高亮, 自动缩进

if __name__ == "__main__":
    hello("Emacs")            ← 正确缩进

4. Python Mode 特有快捷键:
   C-c C-c    运行当前文件
   C-c C-z    切换到 Python Shell
   C-c C-l    加载文件到 Shell
```

**语法高亮示例:**
```python
def calculate(x, y):     # "def" 蓝色关键字
    """Docstring"""      # 红色字符串
    # Comment            # 灰色注释
    return x + y         # "return" 蓝色
```

##### C Mode

**特性:**
- C 语法高亮
- K&R / GNU 等缩进风格
- 花括号自动配对
- 编译快捷键

**实战操作:**
```
1. 创建 C 文件
   C-x C-f ~/test.c

2. 输入代码:
#include <stdio.h>

int main() {
    printf("Hello\n");  ← 自动缩进4空格
    return 0;
}

3. C Mode 快捷键:
   C-c C-c    编译(make)
   M-;        插入 /* */ 注释
```

##### Markdown Mode (需安装)

**实战操作:**
```
1. 安装 markdown-mode
   M-x package-install RET markdown-mode

2. 打开 Markdown 文件
   C-x C-f ~/README.md

3. 特性:
   - # 标题高亮
   - **粗体** 显示加粗
   - `代码` 特殊颜色
   - [链接](url) 语法高亮

4. Markdown Mode 快捷键:
   C-c C-c p    预览(在浏览器打开)
   C-c C-c l    插入链接
   C-c C-c i    插入图片
```

---

#### 4. Org Mode (组织模式)

**描述:** Emacs 最强大的模式之一,用于**笔记、任务管理、文档编写**。

**核心功能:**
- 大纲组织(标题折叠)
- TODO 任务管理
- 表格编辑
- 导出(HTML, LaTeX, PDF)
- 代码块执行

**实战操作:**
```
1. 创建 Org 文件
   C-x C-f ~/notes.org

2. 输入大纲结构:
* 一级标题
** 二级标题
*** 三级标题
这是内容

3. 折叠/展开:
   TAB        折叠/展开当前标题
   S-TAB      折叠/展开整个文件

4. TODO 管理:
* TODO 学习 Emacs
  DEADLINE: <2025-12-06 Fri>
** DONE 安装 Emacs
   CLOSED: [2025-12-05 Thu 10:30]

5. Org Mode 快捷键:
   C-c C-t    切换 TODO 状态
   M-RET      新建同级项
   M-→/←      提升/降低层级
```

**Org 表格示例:**
```org
| 姓名 | 年龄 | 城市   |
|------+------+--------|
| 张三 |   25 | 北京   |
| 李四 |   30 | 上海   |

在表格内按 TAB 自动对齐和跳转
```

**适用场景:**
```
✓ 个人知识库
✓ 项目管理
✓ 写作文档
✓ 时间管理(GTD)
✓ 学术论文
```

---

#### 5. Dired Mode (目录编辑模式)

**描述:** 把**文件管理**变成"编辑文本",这是 Emacs 的独特之处!

**实战操作:**
```
1. 打开目录
   C-x d ~/Downloads

2. 进入 Dired Mode:
  /home/user/Downloads:
  drwxr-xr-x  5 user  users   4096 Dec  5 10:00 .
  drwxr-xr-x 30 user  users   4096 Dec  4 15:30 ..
  -rw-r--r--  1 user  users  12345 Dec  3 09:15 file1.txt
  -rw-r--r--  1 user  users  67890 Dec  2 14:20 file2.pdf

3. Dired Mode 操作:
   n/p        下/上移动
   RET        打开文件/进入目录
   d          标记删除
   x          执行删除
   C          复制
   R          重命名/移动
   +          新建目录
   g          刷新

4. 批量操作:
   m          标记文件
   u          取消标记
   U          取消所有标记
   * s        标记所有
```

**实战示例:批量重命名**
```
1. C-x d ~/Photos
2. 标记多个文件: m m m
3. 按 R (Rename)
4. 输入新名字模式
```

---

### Major Mode 的继承关系

```
Fundamental Mode (根模式)
    ↓
Text Mode
    ↓
    ├── Markdown Mode
    ├── Org Mode
    └── LaTeX Mode

Prog Mode (编程模式基类)
    ↓
    ├── Python Mode
    ├── C Mode
    ├── JavaScript Mode
    └── Lisp Mode
        └── Emacs Lisp Mode
```

**查看继承关系:**
```
M-x describe-mode (C-h m)
显示当前模式的父模式
```

---

## Minor Mode 详解

### Minor Mode 的特点

1. **可叠加** - 可以同时开启多个
2. **辅助性** - 不改变主要编辑方式
3. **可切换** - 随时开启/关闭

### 常用 Minor Modes



#### 1. Display 类(显示增强)



##### Line Number Mode (行号显示)


**实战操作:**
```
1. 开启行号
   M-x display-line-numbers-mode

2. 效果:
    1 │ Line 1
    2 │ Line 2
    3 │ Line 3

3. 全局开启(写入配置):
   (global-display-line-numbers-mode 1)
```


##### HL Line Mode (当前行高亮)

**实战操作:**

```
1. 开启
   M-x hl-line-mode

2. 效果:
   Line 1
   Line 2
   ┃Line 3┃  ← 当前行有背景色
   Line 4

3. 全局开启:
   (global-hl-line-mode 1)
```



##### Visual Line Mode (视觉换行)

**对比理解:**
```
【关闭 Visual Line Mode】
This is a very very very very long line that extends beyond→
↓
(需要水平滚动才能看到)

【开启 Visual Line Mode】
This is a very very very very
long line that wraps at window
edge automatically
↓
(自动视觉换行,但文件中仍是一行)
```

**实战操作:**
```
1. 开启
   M-x visual-line-mode

2. 测试:
   输入超长行,会自动换行显示
   但 C-a/C-e 仍然跳整个逻辑行

3. 适用场景:
   ✓ 阅读长文本
   ✓ Markdown 文档
   ✗ 代码编辑
```

---

#### 2. Editing 类(编辑增强)

##### Auto Fill Mode (自动填充/换行)

**实战操作:**
```
1. 开启
   M-x auto-fill-mode

2. 效果:
   输入长文本时,到达80列自动换行

   输入: This is a very long sentence that will automatically wrap when it reaches the fill column limit.

   结果:
   This is a very long sentence that will automatically wrap when
   it reaches the fill column limit.

3. 设置换行列数:
   M-x set-fill-column
   输入:100
```

##### Electric Pair Mode (括号自动配对)

**实战操作:**
```
1. 开启
   M-x electric-pair-mode

2. 测试:
   输入 (     自动变成 (|)  光标在中间
   输入 "     自动变成 "|"
   输入 {     自动变成 {|}

3. 智能特性:
   def hello(name):
       print("|")
   ↑
   输入右括号时不会重复

4. 全局开启:
   (electric-pair-mode 1)
```

##### Delete Selection Mode (选中文本后输入替换)

**实战操作:**
```
1. 开启
   M-x delete-selection-mode

2. 测试:
   文本: Hello World

   - C-SPC 标记 "Hello"
   - 输入 "Hi"
   - 结果: Hi World (自动删除选中内容)

3. 这是现代编辑器的标准行为
```

---

#### 3. Programming 类(编程辅助)

##### Flycheck Mode (语法检查)

**实战操作:**
```
1. 安装
   M-x package-install RET flycheck

2. 开启
   M-x flycheck-mode

3. 效果:
   def hello(name)  ← 红色波浪线(缺少冒号)
       print name   ← 红色波浪线(Python3 需括号)

4. 查看错误:
   C-c ! l    列出所有错误
```

##### Company Mode (自动补全)

**实战操作:**
```
1. 安装
   M-x package-install RET company

2. 开启
   M-x company-mode

3. 效果:
   输入 pri
   弹出补全菜单:
   ┌─────────────┐
   │ print       │
   │ private     │
   │ priority    │
   └─────────────┘

4. 操作:
   C-n/C-p    上下选择
   RET        确认补全
   C-g        取消
```

---

#### 4. Version Control 类

##### Magit (Git 集成)

**实战操作:**
```
1. 安装
   M-x package-install RET magit

2. 打开 Git 状态
   M-x magit-status (或 C-x g)

3. Magit Buffer:
   Untracked files
     test.txt
   Unstaged changes
   M  README.md

4. 操作:
   s      Stage (添加到暂存区)
   c c    Commit (提交)
   P p    Push (推送)
```

---

### Minor Mode 的管理

#### 查看已启用的 Minor Modes

```
方法1:
M-x describe-mode (C-h m)
显示所有已启用的模式

方法2:
看模式行(Mode Line):
-UUU:----F1  test.txt  (Python Line-Num HL-Line Auto-Fill)
                             ↑      ↑        ↑        ↑
                             主模式  辅助模式们
```

#### 开启/关闭 Minor Mode

```
通用方法:
M-x <mode-name>

示例:
M-x line-number-mode       开启行号(再执行一次关闭)
M-x auto-fill-mode         开启自动换行
M-x flycheck-mode          开启语法检查
```

---

## 模式的激活与切换

### 自动激活(根据文件扩展名)

Emacs 通过 `auto-mode-alist` 自动选择模式:

```elisp
;; 内置规则示例
.py     → Python Mode
.c      → C Mode
.txt    → Text Mode
.org    → Org Mode
.md     → Markdown Mode (如果安装了)
```

**实战验证:**
```
1. 创建 test.py
   C-x C-f ~/test.py
   自动进入 Python Mode

2. 创建 notes.org
   C-x C-f ~/notes.org
   自动进入 Org Mode

3. 创建 README.txt
   C-x C-f ~/README.txt
   自动进入 Text Mode
```

### 手动切换 Major Mode

```
M-x <mode-name>

示例:
M-x python-mode        切换到 Python Mode
M-x text-mode          切换到 Text Mode
M-x fundamental-mode   切换到 Fundamental Mode
M-x org-mode           切换到 Org Mode
```

**实战示例:**
```
1. 打开 test.txt (默认 Text Mode)
2. 想写 Python 代码
3. M-x python-mode
4. 现在有 Python 语法高亮和缩进
```

### 临时覆盖文件类型判断

**使用 File Local Variables:**

在文件第一行添加:
```
# -*- mode: python -*-

或

#!/usr/bin/env python

或在文件末尾添加:
# Local Variables:
# mode: python
# End:
```

**实战示例:**
```
创建 myscript (无扩展名)

内容:
#!/usr/bin/env python
# -*- mode: python -*-

def hello():
    print("Hello")

保存后重新打开,自动进入 Python Mode
```

---

## 常用模式实战

### 场景1:写 Python 代码

```
1. 创建文件
   C-x C-f ~/hello.py

2. 自动进入 Python Mode

3. 开启辅助功能:
   M-x company-mode        # 自动补全
   M-x flycheck-mode       # 语法检查
   M-x electric-pair-mode  # 括号配对

4. 编码:
def greet(name):
    print(f"Hello, {name}!")  ← 自动缩进、高亮

5. 运行:
   C-c C-c (运行文件)
   C-c C-z (打开 Python Shell)
```

### 场景2:写笔记(Org Mode)

```
1. 创建文件
   C-x C-f ~/notes.org

2. 自动进入 Org Mode

3. 编写大纲:
* 今日任务
** TODO 学习 Emacs
** DONE 读书
* 学习笔记
** Emacs 模式系统
*** Major Mode
*** Minor Mode

4. 操作:
   TAB        折叠/展开
   M-RET      新建同级项
   C-c C-t    切换 TODO 状态

5. 导出:
   C-c C-e h o  (导出为 HTML 并打开)
```

### 场景3:文件管理(Dired Mode)

```
1. 打开目录
   C-x d ~/Documents

2. 自动进入 Dired Mode

3. 批量重命名:
   m m m      标记3个文件
   R          重命名
   photo_*.jpg (使用通配符)

4. 删除文件:
   d d        标记2个删除
   x          执行删除

5. 打开文件:
   RET        在当前窗口打开
   o          在另一个窗口打开
```

### 场景4:Markdown 编辑

```
1. 安装 Markdown Mode
   M-x package-install RET markdown-mode

2. 创建文件
   C-x C-f ~/README.md

3. 自动进入 Markdown Mode

4. 开启视觉换行:
   M-x visual-line-mode

5. 编写:
# 标题
## 二级标题

这是一段很长的文本会自动换行显示...

6. 预览:
   C-c C-c p  (在浏览器预览)
```

---

## 自定义模式

### 配置默认开启的 Minor Modes

在 `~/.emacs` 或 `~/.emacs.d/init.el` 中:

```elisp
;; 全局开启行号
(global-display-line-numbers-mode 1)

;; 全局高亮当前行
(global-hl-line-mode 1)

;; 全局开启括号配对
(electric-pair-mode 1)

;; 在特定模式下开启 Minor Mode
(add-hook 'python-mode-hook
  (lambda ()
    (company-mode 1)        ; 自动补全
    (flycheck-mode 1)       ; 语法检查
    (setq indent-tabs-mode nil)  ; 使用空格而非Tab
    (setq python-indent-offset 4)))  ; 缩进4空格

;; 在 Text Mode 下自动换行
(add-hook 'text-mode-hook 'auto-fill-mode)

;; 在 Org Mode 下开启视觉换行
(add-hook 'org-mode-hook 'visual-line-mode)
```

### 创建简单的 Major Mode

```elisp
;; 定义一个简单的 TODO 模式
(define-derived-mode todo-mode text-mode "TODO"
  "Major mode for editing TODO lists."
  ;; 语法高亮
  (font-lock-add-keywords nil
   '(("^\\[DONE\\].*$" . font-lock-comment-face)
     ("^\\[TODO\\].*$" . font-lock-warning-face)
     ("^\\[URGENT\\].*$" . font-lock-keyword-face))))

;; 关联文件扩展名
(add-to-list 'auto-mode-alist '("\\.todo\\'" . todo-mode))

;; 使用:创建 tasks.todo 文件
;; [TODO] 学习 Emacs
;; [DONE] 安装 Emacs
;; [URGENT] 完成项目
```

---

## 模式系统总结

### 快速对比表

| 特性 | Fundamental | Text | Python | Org | Dired |
|------|------------|------|--------|-----|-------|
| 语法高亮 | ❌ | ❌ | ✅ | ✅ | ✅ |
| 自动缩进 | ❌ | ✅ | ✅ | ✅ | N/A |
| 大纲折叠 | ❌ | ❌ | ❌ | ✅ | N/A |
| 运行代码 | ❌ | ❌ | ✅ | ✅ | N/A |
| 文件操作 | ❌ | ❌ | ❌ | ❌ | ✅ |

### 模式选择指南

```
编辑什么内容?
│
├─ 纯文本/笔记 → Text Mode
│   └─ 需要大纲组织? → Org Mode
│
├─ 代码
│   ├─ Python → Python Mode
│   ├─ C/C++ → C Mode
│   └─ JavaScript → JS Mode
│
├─ 标记语言
│   ├─ Markdown → Markdown Mode
│   ├─ HTML → HTML Mode
│   └─ LaTeX → LaTeX Mode
│
└─ 文件管理 → Dired Mode
```

### 实用技巧

1. **查看当前模式信息:**
   ```
   C-h m (describe-mode)
   显示当前 Major Mode 和所有 Minor Modes
   ```

2. **快速切换模式:**
   ```
   M-x python-mode
   M-x text-mode
   M-x org-mode
   ```

3. **查看模式快捷键:**
   ```
   C-h m
   滚动到 Key bindings 部分
   ```

4. **临时禁用所有 Minor Modes:**
   ```elisp
   M-: (mapc (lambda (mode) (funcall mode -1)) minor-mode-list)
   ```

---

## 常见问题

### Q1: Major Mode 可以共存吗?
**A:** 不可以,一个 buffer 只能有一个 Major Mode。但可以:
- 使用继承关系(如 Python Mode 继承自 Prog Mode)
- 在不同 buffer 使用不同 Major Mode
- 使用 `polymode` 在同一文件中混合模式(高级技巧)

### Q2: 如何知道某个模式有哪些快捷键?
**A:**
```
1. C-h m (查看模式文档)
2. C-h b (查看所有快捷键绑定)
3. C-h w <command> (查看命令绑定的快捷键)
```

### Q3: Minor Mode 冲突怎么办?
**A:** 某些 Minor Mode 可能冲突(如两个补全插件):
```
1. 查看已启用的: C-h m
2. 关闭冲突的: M-x <mode-name>
3. 在配置中设置优先级
```

### Q4: 为什么我的文件没有语法高亮?
**A:** 检查:
```
1. 是否在正确的 Major Mode?
   M-x describe-mode

2. 是否关闭了 font-lock?
   M-x font-lock-mode (切换)

3. 是否需要安装对应的 mode?
   M-x package-install RET <mode-name>
```

### Q5: 如何让某个 Minor Mode 在所有文件中生效?
**A:**
```elisp
;; 方法1:使用 global- 前缀
(global-line-number-mode 1)
(global-hl-line-mode 1)

;; 方法2:使用 hook 钩子
(add-hook 'prog-mode-hook 'flycheck-mode)  ; 所有编程模式
(add-hook 'text-mode-hook 'auto-fill-mode) ; 所有文本模式
```

---

## 进阶学习

完成模式系统理解后,可以继续学习:

1. **Hook 系统** - 在模式激活时自动执行操作
2. **自定义语法高亮** - `font-lock-mode` 深入
3. **创建自己的 Major Mode** - `define-derived-mode`
4. **Polymode** - 在一个文件中使用多个 Major Mode

---

## 练习任务

### 任务1:模式探索
```
1. 创建 5 个不同扩展名的文件(.py, .c, .txt, .org, .md)
2. 观察每个文件自动进入的模式
3. 用 C-h m 查看每个模式的特性
4. 手动切换模式并观察变化
```

### 任务2:Minor Mode 组合
```
1. 打开一个 Python 文件
2. 依次开启:
   - M-x display-line-numbers-mode
   - M-x hl-line-mode
   - M-x company-mode
3. 观察效果
4. 一个个关闭,体会差异
```

### 任务3:配置实战
```
1. 编辑 ~/.emacs
2. 添加全局 Minor Modes 配置
3. 添加特定模式的 Hook
4. 重启 Emacs 验证
```

恭喜你深入理解了 Emacs 的模式系统! 🎉
