# Emacs Help 系统与 Elisp 基础

## 目录
1. [Help 系统导航](#help-系统导航)
2. [Emacs Lisp 基础命令](#emacs-lisp-基础命令)
3. [常用 Elisp 函数详解](#常用-elisp-函数详解)
4. [配置文件编写](#配置文件编写)

---

## Help 系统导航

### 问题:在 Help 中查看函数后如何回退?

当你在 Help buffer 中按 `RET`(回车)跳转到另一个函数定义后,可以使用以下方法返回:

#### 方法 1: `l` (back) - 最常用 ⭐

```
在 *Help* buffer 中:
l    ← 按字母 l (小写的 L)
     (back - 返回上一个位置)
```

**实战演示:**
```
1. 查看函数帮助
   C-h f setq RET

2. 在 *Help* buffer 中,看到 "defvar" 链接
   移动光标到 "defvar" 上,按 RET

3. 跳转到 defvar 的帮助

4. 想回到 setq 的帮助
   按 l (back)

5. 可以继续按 l 返回更早的页面
```

#### 方法 2: `r` (forward) - 前进

```
l    ← 后退
r    ← 前进(redo)

就像浏览器的 后退/前进 按钮
```

#### 方法 3: `q` - 退出 Help

```
q    ← 关闭 *Help* buffer
```

#### 方法 4: `C-x b` - 切换 buffer

```
C-x b RET    ← 切换回之前的 buffer
```

---

### Help 系统完整导航快捷键

在 **\*Help\*** buffer 中可用:

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `l` | help-go-back | **后退**(返回上一个 Help 页面) |
| `r` | help-go-forward | **前进**(在后退后前进) |
| `RET` | help-follow | 跟随链接(跳转到函数/变量定义) |
| `TAB` | forward-button | 移动到下一个链接 |
| `S-TAB` | backward-button | 移动到上一个链接 |
| `q` | quit-window | 关闭 Help 窗口 |
| `s` | help-view-source | 查看源代码 |
| `g` | revert-buffer | 刷新当前 Help 页面 |
| `n` | next-line | 下一行 |
| `p` | previous-line | 上一行 |
| `SPC` | scroll-up | 向下翻页 |
| `S-SPC` | scroll-down | 向上翻页 |

---

### 实战练习:Help 系统导航

**练习 1:探索函数链**

```
1. 查看 setq 的帮助
   C-h f setq RET

2. 在 Help 中找到 "set" 链接
   按 TAB 跳到 "set" 链接
   按 RET 跳转

3. 继续查看 "symbol-value" 链接
   TAB 找到链接
   RET 跳转

4. 现在导航历史是:
   setq → set → symbol-value

5. 后退操作:
   按 l (回到 set)
   再按 l (回到 setq)

6. 前进操作:
   按 r (前进到 set)
   再按 r (前进到 symbol-value)
```

**练习 2:查看源代码**

```
1. C-h f setq RET
2. 在 Help buffer 中按 s (view source)
3. 跳转到 setq 的源代码定义
4. 按 q 关闭源代码窗口
5. 回到 Help buffer
```

---

### Help 系统的历史栈

Emacs Help 维护一个**历史栈**:

```
初始状态:
[]

查看 setq:
[setq]

点击 set:
[setq] → [set]

点击 symbol-value:
[setq] → [set] → [symbol-value]
            ↑
          当前位置

按 l (back):
[setq] → [set] → [symbol-value]
     ↑
   当前位置

按 r (forward):
[setq] → [set] → [symbol-value]
            ↑
          当前位置
```

---

## Emacs Lisp 基础命令

### 什么是 Emacs Lisp (Elisp)?

**Emacs Lisp** 是 Emacs 的配置和扩展语言,也是 Lisp 方言之一。

**Lisp 的特点:**
- 括号语法: `(function arg1 arg2)`
- 前缀表达式: `(+ 1 2)` 而不是 `1 + 2`
- 函数式编程
- 动态类型

---

### 核心 Elisp 函数详解

#### 1. `setq` - 设置变量 ⭐⭐⭐⭐⭐

**语法:**
```elisp
(setq VARIABLE VALUE)
```

**作用:** 设置变量的值

**示例:**
```elisp
;; 设置单个变量
(setq my-name "Zhang San")

;; 设置多个变量
(setq name "Li Si"
      age 25
      city "Beijing")

;; 实用配置示例
(setq inhibit-startup-message t)     ; 关闭启动画面
(setq make-backup-files nil)         ; 不创建备份文件(~结尾)
(setq auto-save-default nil)         ; 关闭自动保存
(setq indent-tabs-mode nil)          ; 使用空格而非 Tab
(setq-default tab-width 4)           ; Tab 宽度为 4
```

**查看帮助:**
```
C-h f setq RET
```

**关键点:**
- `setq` = "SET Quoted" (设置引用的变量)
- 不需要在变量名前加引号
- 可以一次设置多个变量

---

#### 2. `defvar` - 定义变量

**语法:**
```elisp
(defvar VARIABLE VALUE "DOCSTRING")
```

**作用:** 定义一个变量(如果未定义),并提供文档字符串

**示例:**
```elisp
;; 定义一个变量
(defvar my-project-path "~/projects/"
  "我的项目目录路径")

;; 如果变量已经存在,defvar 不会覆盖
(defvar my-name "Default Name")  ; 首次定义
(defvar my-name "New Name")      ; 不会改变 my-name 的值

;; 查看变量值
my-name  ; → "Default Name"
```

**setq vs defvar:**
```elisp
;; defvar: 定义变量(首次),提供文档
(defvar my-var "initial")
(defvar my-var "changed")  ; 不会改变值
my-var  ; → "initial"

;; setq: 总是设置变量值
(setq my-var "initial")
(setq my-var "changed")    ; 会改变值
my-var  ; → "changed"
```

---

#### 3. `defun` - 定义函数

**语法:**
```elisp
(defun FUNCTION-NAME (ARGS)
  "DOCSTRING"
  BODY)
```

**示例:**
```elisp
;; 简单函数
(defun say-hello ()
  "打招呼"
  (message "Hello, Emacs!"))

;; 调用函数
(say-hello)  ; → 在 minibuffer 显示 "Hello, Emacs!"

;; 带参数的函数
(defun greet (name)
  "向 NAME 打招呼"
  (message "Hello, %s!" name))

(greet "Li Si")  ; → "Hello, Li Si!"

;; 多参数函数
(defun add (a b)
  "计算 A + B"
  (+ a b))

(add 3 5)  ; → 8

;; 实用函数示例:快速打开配置文件
(defun open-config ()
  "快速打开 Emacs 配置文件"
  (interactive)  ; 允许通过 M-x 调用
  (find-file "~/.emacs"))

;; 使用: M-x open-config
```

---

#### 4. `let` - 局部变量

**语法:**
```elisp
(let ((VAR1 VALUE1)
      (VAR2 VALUE2))
  BODY)
```

**作用:** 创建局部变量,只在 let 块内有效

**示例:**
```elisp
;; 基础用法
(let ((x 10)
      (y 20))
  (+ x y))  ; → 30

;; x 和 y 在 let 外部不存在

;; 实用示例:临时设置配置
(let ((case-fold-search nil))  ; 临时区分大小写
  (search-forward "Emacs"))

;; 嵌套 let
(let ((x 5))
  (let ((y 10))
    (+ x y)))  ; → 15
```

---

#### 5. `lambda` - 匿名函数

**语法:**
```elisp
(lambda (ARGS) BODY)
```

**示例:**
```elisp
;; 直接调用
((lambda (x) (* x x)) 5)  ; → 25

;; 作为参数传递
(mapcar (lambda (x) (* x 2))
        '(1 2 3 4))  ; → (2 4 6 8)

;; 在 hook 中使用
(add-hook 'python-mode-hook
  (lambda ()
    (setq indent-tabs-mode nil)
    (setq python-indent-offset 4)))
```

---

#### 6. `if` - 条件判断

**语法:**
```elisp
(if CONDITION
    THEN
  ELSE)
```

**示例:**
```elisp
;; 简单条件
(if (> 5 3)
    "5 大于 3"
  "5 不大于 3")  ; → "5 大于 3"

;; 实用示例:检查文件是否存在
(if (file-exists-p "~/.emacs")
    (load "~/.emacs")
  (message "配置文件不存在"))

;; 多条语句使用 progn
(if (> score 60)
    (progn
      (message "及格")
      (setq pass t))
  (progn
    (message "不及格")
    (setq pass nil)))
```

---

#### 7. `when` 和 `unless` - 简化条件

**when: 只有 THEN 分支**
```elisp
(when CONDITION
  BODY)

;; 等同于
(if CONDITION
    (progn BODY))
```

**示例:**
```elisp
;; when 示例
(when (file-exists-p custom-file)
  (load custom-file)
  (message "配置已加载"))

;; unless 示例(条件为假时执行)
(unless (package-installed-p 'company)
  (package-install 'company))
```

---

#### 8. `cond` - 多条件判断

**语法:**
```elisp
(cond
  (CONDITION1 ACTION1)
  (CONDITION2 ACTION2)
  (t DEFAULT-ACTION))  ; t = 默认分支
```

**示例:**
```elisp
(defun grade (score)
  (cond
    ((>= score 90) "优秀")
    ((>= score 80) "良好")
    ((>= score 60) "及格")
    (t "不及格")))

(grade 85)  ; → "良好"
```

---

### 常用数据操作函数

#### 列表操作

```elisp
;; 创建列表
(list 1 2 3)          ; → (1 2 3)
'(1 2 3)              ; → (1 2 3) (引用形式)

;; 列表操作
(car '(1 2 3))        ; → 1 (第一个元素)
(cdr '(1 2 3))        ; → (2 3) (剩余元素)
(cons 0 '(1 2 3))     ; → (0 1 2 3) (添加到开头)
(append '(1 2) '(3 4)); → (1 2 3 4) (连接列表)
(length '(1 2 3))     ; → 3 (列表长度)
(nth 1 '(a b c))      ; → b (获取第n个元素,从0开始)
```

#### 字符串操作

```elisp
;; 连接字符串
(concat "Hello" " " "World")  ; → "Hello World"

;; 格式化字符串
(format "Name: %s, Age: %d" "Li Si" 25)  ; → "Name: Li Si, Age: 25"

;; 字符串长度
(length "Hello")  ; → 5

;; 子字符串
(substring "Hello World" 0 5)  ; → "Hello"

;; 字符串比较
(string= "abc" "abc")   ; → t (相等)
(string< "abc" "def")   ; → t (小于)
```

---

## 配置文件编写

### 基础配置示例

**~/.emacs 或 ~/.emacs.d/init.el**

```elisp
;;; init.el --- Emacs 配置文件

;;; Commentary:
;; 我的 Emacs 配置

;;; Code:

;; ========== 基础设置 ==========

;; 关闭启动画面
(setq inhibit-startup-message t)

;; 显示行号
(global-display-line-numbers-mode 1)

;; 高亮当前行
(global-hl-line-mode 1)

;; 显示列号
(column-number-mode 1)

;; 括号匹配
(electric-pair-mode 1)

;; 不创建备份文件
(setq make-backup-files nil)

;; ========== 自定义函数 ==========

(defun my/open-config ()
  "快速打开配置文件"
  (interactive)
  (find-file user-init-file))

;; 绑定快捷键
(global-set-key (kbd "C-c c") 'my/open-config)

;; ========== 模式特定配置 ==========

;; Python 模式配置
(add-hook 'python-mode-hook
  (lambda ()
    (setq indent-tabs-mode nil)
    (setq python-indent-offset 4)))

;; Text 模式自动换行
(add-hook 'text-mode-hook 'auto-fill-mode)

;; ========== Custom File ==========

;; 自定义配置存放位置
(setq custom-file (expand-file-name "custom.el" user-emacs-directory))
(when (file-exists-p custom-file)
  (load custom-file))

(provide 'init)
;;; init.el ends here
```

---

### 实战:编写你的第一个配置

**步骤 1:打开配置文件**
```
C-x C-f ~/.emacs RET
```

**步骤 2:添加简单配置**
```elisp
;; 显示行号
(global-display-line-numbers-mode 1)

;; 高亮当前行
(global-hl-line-mode 1)
```

**步骤 3:保存并测试**
```
C-x C-s (保存)
M-x eval-buffer (立即生效,无需重启)
```

**步骤 4:重启验证**
```
C-x C-c (退出)
emacs (重新启动)
```

---

## 调试与测试

### 在 *scratch* buffer 中测试

```elisp
;; 测试变量设置
(setq test-var "Hello")
test-var  ; 按 C-j 查看结果

;; 测试函数
(defun test-func ()
  (message "Test"))
(test-func)  ; 按 C-j 执行

;; 测试条件
(if (> 5 3)
    "true"
  "false")  ; 按 C-j 查看结果
```

### 使用 ielm (交互式 Elisp)

```
1. M-x ielm
2. 输入 Elisp 表达式
3. 按回车立即看到结果

示例:
ELISP> (+ 1 2)
3
ELISP> (setq name "Zhang San")
"Zhang San"
ELISP> name
"Zhang San"
```

---

## 常见问题

### Q1: setq vs setq-default?
```elisp
;; setq: 设置当前 buffer 的局部值
(setq tab-width 4)  ; 只影响当前 buffer

;; setq-default: 设置全局默认值
(setq-default tab-width 4)  ; 影响所有 buffer
```

### Q2: 如何查看变量的值?
```
方法1: C-h v 变量名
方法2: 在 *scratch* 中输入变量名,按 C-j
方法3: M-: 变量名 RET
```

### Q3: 配置不生效怎么办?
```
1. 检查语法错误: 括号是否匹配
2. 重启 Emacs
3. 查看 *Messages* buffer: C-h e
4. 启用 debug: M-x toggle-debug-on-error
```

---

## 学习资源

**内置帮助:**
```
C-h f    查看函数
C-h v    查看变量
C-h k    查看快捷键
C-h i m Elisp    Elisp 手册
```

**推荐阅读:**
1. Emacs Lisp Intro (内置): `C-h i m Emacs Lisp Intro`
2. Elisp Reference Manual: `C-h i m Elisp`

---

## 练习任务

### 任务 1: 探索 Help 系统
```
1. C-h f setq
2. 点击 "set" 链接
3. 点击 "symbol-value" 链接
4. 用 l 返回 set
5. 用 l 返回 setq
6. 用 r 前进到 set
```

### 任务 2: 编写简单配置
```
1. 打开 ~/.emacs
2. 添加:
   (setq inhibit-startup-message t)
   (global-display-line-numbers-mode 1)
3. M-x eval-buffer 测试
4. 重启 Emacs 验证
```

### 任务 3: 定义自己的函数
```elisp
(defun my/insert-date ()
  "插入当前日期"
  (interactive)
  (insert (format-time-string "%Y-%m-%d")))

;; 绑定快捷键
(global-set-key (kbd "C-c d") 'my/insert-date)

;; 测试: C-c d
```

完成这些,你就入门 Elisp 了! 🎉
