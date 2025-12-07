# Emacs 快捷键速查表

## 快速查找索引
- [启动退出](#启动退出)
- [光标移动](#光标移动)
- [文本编辑](#文本编辑)
- [选择复制](#选择复制)
- [文件操作](#文件操作)
- [搜索替换](#搜索替换)
- [窗口管理](#窗口管理)
- [Buffer管理](#buffer管理)
- [帮助系统](#帮助系统)

---

## 符号说明

| 符号 | 含义 | 键盘操作 |
|------|------|----------|
| `C-` | Control | 按住 Ctrl |
| `M-` | Meta | 按住 Alt (或 Esc 后释放) |
| `S-` | Shift | 按住 Shift |
| `RET` | Return | 回车键 |
| `SPC` | Space | 空格键 |
| `DEL` | Delete | 退格键 |

**示例:**
- `C-x C-s` = 按住 Ctrl,按 x,再按 s
- `M-x` = 按住 Alt,按 x
- `C-M-f` = 同时按住 Ctrl+Alt,按 f

---

## 启动退出

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x C-c` | save-buffers-kill-terminal | 退出 Emacs(提示保存) |
| `C-z` | suspend-frame | 挂起到后台(终端模式) |
| `C-g` | keyboard-quit | **取消当前命令** |
| `ESC ESC ESC` | keyboard-escape-quit | 紧急退出命令 |

---

## 光标移动

### 基础移动

| 快捷键 | 命令 | 说明 | 记忆 |
|--------|------|------|------|
| `C-f` | forward-char | 向前一个字符 | **F**orward |
| `C-b` | backward-char | 向后一个字符 | **B**ackward |
| `C-n` | next-line | 下一行 | **N**ext |
| `C-p` | previous-line | 上一行 | **P**revious |
| `C-a` | move-beginning-of-line | 行首 | **A** = 字母表开头 |
| `C-e` | move-end-of-line | 行尾 | **E**nd |

### 按词移动

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `M-f` | forward-word | 前移一个词 |
| `M-b` | backward-word | 后移一个词 |
| `M-a` | backward-sentence | 句首 |
| `M-e` | forward-sentence | 句尾 |
| `M-{` | backward-paragraph | 段首 |
| `M-}` | forward-paragraph | 段尾 |

### 屏幕滚动

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-v` | scroll-up-command | 向下翻页 |
| `M-v` | scroll-down-command | 向上翻页 |
| `C-l` | recenter-top-bottom | 光标居中(连按循环) |
| `M-<` | beginning-of-buffer | 文件开头 |
| `M->` | end-of-buffer | 文件结尾 |
| `M-g g` | goto-line | 跳转到行号 |

### 高级跳转

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-u C-SPC` | pop-global-mark | 跳回上一个标记 |
| `C-x C-x` | exchange-point-and-mark | 光标与标记互换 |
| `M-g M-g` | goto-line | 跳转到行(同 M-g g) |

---

## 文本编辑

### 删除

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-d` | delete-char | 删除光标后字符 |
| `DEL` | delete-backward-char | 删除光标前字符 |
| `M-d` | kill-word | 删除光标后的词 |
| `M-DEL` | backward-kill-word | 删除光标前的词 |
| `C-k` | kill-line | **删除到行尾** |
| `C-S-Backspace` | kill-whole-line | 删除整行 |
| `M-k` | kill-sentence | 删除到句尾 |

### 撤销重做

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-/` | undo | 撤销 |
| `C-_` | undo | 撤销(同上) |
| `C-x u` | undo | 撤销(同上) |
| `C-g` 后 `C-/` | - | "重做"(打断撤销链) |

### 插入与换行

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `RET` | newline | 换行 |
| `C-o` | open-line | 在当前位置插入空行 |
| `C-j` | electric-newline-and-maybe-indent | 换行并缩进 |
| `M-^` | delete-indentation | 合并到上一行 |

---

## 选择复制

### 标记与选区

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-SPC` | set-mark-command | **设置标记**(选区起点) |
| `C-@` | set-mark-command | 设置标记(同上) |
| `C-SPC C-SPC` | - | 设置标记(不激活选区) |
| `C-x h` | mark-whole-buffer | 选择整个文件 |
| `M-h` | mark-paragraph | 选择段落 |

### 剪切复制粘贴

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-w` | kill-region | **剪切**选中区域 |
| `M-w` | kill-ring-save | **复制**选中区域 |
| `C-y` | yank | **粘贴**(Yank) |
| `M-y` | yank-pop | 循环粘贴历史 |

### 矩形操作

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x r k` | kill-rectangle | 矩形剪切 |
| `C-x r M-w` | copy-rectangle-as-kill | 矩形复制 |
| `C-x r y` | yank-rectangle | 矩形粘贴 |
| `C-x r t` | string-rectangle | 矩形插入文本 |
| `C-x r d` | delete-rectangle | 矩形删除 |
| `C-x r o` | open-rectangle | 矩形插入空白 |

---

## 文件操作

### 打开保存

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x C-f` | find-file | **打开文件** |
| `C-x C-s` | save-buffer | **保存文件** |
| `C-x C-w` | write-file | 另存为 |
| `C-x s` | save-some-buffers | 保存所有(逐个询问) |
| `C-x i` | insert-file | 插入文件内容 |

### Buffer 管理

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x b` | switch-to-buffer | **切换 buffer** |
| `C-x C-b` | list-buffers | 列出所有 buffer |
| `C-x k` | kill-buffer | 关闭 buffer |
| `C-x →` | next-buffer | 下一个 buffer |
| `C-x ←` | previous-buffer | 上一个 buffer |

---

## 搜索替换

### 增量搜索

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-s` | isearch-forward | **向前搜索** |
| `C-r` | isearch-backward | 向后搜索 |
| `C-s C-s` | - | 重复上次搜索 |
| `C-s RET` | - | 结束搜索(留在位置) |
| `C-g` | - | 取消搜索(回到起点) |

### 正则搜索

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-M-s` | isearch-forward-regexp | 正则向前搜索 |
| `C-M-r` | isearch-backward-regexp | 正则向后搜索 |

### 替换

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `M-%` | query-replace | **查找替换** |
| `C-M-%` | query-replace-regexp | 正则替换 |

**替换时的选项:**
- `y` - 替换这一个
- `n` - 跳过这一个
- `!` - 替换剩余所有
- `^` - 返回上一个
- `q` - 退出替换

---

## 窗口管理

### 窗口分割

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x 2` | split-window-below | **水平分割**(上下) |
| `C-x 3` | split-window-right | **垂直分割**(左右) |
| `C-x o` | other-window | 切换到下一个窗口 |
| `C-x 0` | delete-window | 关闭当前窗口 |
| `C-x 1` | delete-other-windows | **只保留当前窗口** |

### 窗口调整

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x ^` | enlarge-window | 增加窗口高度 |
| `C-x {` | shrink-window-horizontally | 减小窗口宽度 |
| `C-x }` | enlarge-window-horizontally | 增加窗口宽度 |
| `C-x +` | balance-windows | 平衡所有窗口大小 |
| `C-x -` | shrink-window-if-larger-than-buffer | 缩小到内容大小 |

---

## 帮助系统

### 查看帮助

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-h k` | describe-key | 查看快捷键功能 |
| `C-h f` | describe-function | 查看函数文档 |
| `C-h v` | describe-variable | 查看变量 |
| `C-h m` | describe-mode | 当前模式帮助 |
| `C-h t` | help-with-tutorial | **启动教程** |
| `C-h a` | apropos-command | 模糊搜索命令 |
| `C-h w` | where-is | 查看命令的快捷键 |

### 信息查看

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-h i` | info | 打开 Info 手册 |
| `C-h C-h` | help-for-help | 帮助的帮助 |
| `C-h ?` | help-for-help | 同上 |

---

## 高级操作

### 命令执行

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `M-x` | execute-extended-command | **执行命令** |
| `C-u` | universal-argument | 重复次数前缀 |
| `M-数字` | digit-argument | 参数前缀 |

**示例:**
- `C-u 10 C-n` - 向下移动 10 行
- `C-u 5 M-d` - 删除 5 个词
- `M-5 C-k` - 删除 5 行

### 宏操作

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x (` | kmacro-start-macro | 开始录制宏 |
| `C-x )` | kmacro-end-macro | 结束录制宏 |
| `C-x e` | kmacro-end-and-call-macro | 执行宏 |
| `C-u C-x (` | - | 追加到上一个宏 |

### 大小写转换

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `M-u` | upcase-word | 词转大写 |
| `M-l` | downcase-word | 词转小写 |
| `M-c` | capitalize-word | 词首字母大写 |
| `C-x C-u` | upcase-region | 区域转大写 |
| `C-x C-l` | downcase-region | 区域转小写 |

### 注释

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `M-;` | comment-dwim | 智能注释/取消注释 |

### 缩进

| 快捷键 | 命令 | 说明 |
|--------|------|------|
| `C-x TAB` | indent-rigidly | 调整区域缩进 |
| `M-i` | tab-to-tab-stop | 插入 tab |
| `C-M-\` | indent-region | 自动缩进区域 |

---

## 按功能分类速查

### 新手必记 TOP 20

```
启动退出:
  C-x C-c    退出
  C-g        取消

文件:
  C-x C-f    打开
  C-x C-s    保存

移动:
  C-f/b/n/p  前/后/下/上
  C-a/e      行首/尾
  M-</M->    文件首/尾

编辑:
  C-d        删除字符
  C-k        删除到行尾
  C-/        撤销

选择复制:
  C-SPC      标记
  C-w        剪切
  M-w        复制
  C-y        粘贴

搜索:
  C-s        搜索
  M-%        替换

帮助:
  C-h t      教程
```

### 按使用频率排序

#### ⭐⭐⭐⭐⭐ (每天都用)
```
C-x C-f    打开文件
C-x C-s    保存文件
C-s        搜索
C-g        取消
C-/        撤销
C-SPC      标记
C-w        剪切
M-w        复制
C-y        粘贴
C-a/e      行首/尾
C-k        删除到行尾
```

#### ⭐⭐⭐⭐ (经常用)
```
C-x b      切换 buffer
C-x 2/3    分割窗口
C-x o      切换窗口
C-x 1      只留当前窗口
M-%        替换
M-</M->    文件首/尾
C-x C-b    列出 buffer
```

#### ⭐⭐⭐ (偶尔用)
```
C-x k      关闭 buffer
M-g g      跳转行号
C-h k      查看快捷键
C-x r k    矩形剪切
M-d        删词
C-x C-x    光标标记互换
```

---

## 模式特定快捷键

### Dired 模式(文件管理器)

| 快捷键 | 说明 |
|--------|------|
| `C-x d` | 打开 Dired |
| `RET` | 打开文件/目录 |
| `d` | 标记删除 |
| `x` | 执行删除 |
| `m` | 标记 |
| `u` | 取消标记 |
| `C` | 复制 |
| `R` | 重命名/移动 |
| `+` | 新建目录 |
| `g` | 刷新 |

### Org 模式

| 快捷键 | 说明 |
|--------|------|
| `TAB` | 折叠/展开 |
| `S-TAB` | 全局折叠/展开 |
| `M-RET` | 新建同级项 |
| `M-←/→` | 提升/降低层级 |
| `C-c C-t` | 切换 TODO 状态 |

---

## 快捷键冲突解决

### 终端下的问题

某些快捷键在终端中可能被占用:

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| `C-s` 冻结 | 终端流控 | `stty -ixon` 禁用流控 |
| `C-z` 挂起 | shell 控制 | 用 `fg` 恢复 |
| `M-` 不工作 | Alt 键问题 | 用 `ESC` 后释放代替 |

### GUI vs 终端

| 快捷键 | GUI | 终端(tty) |
|--------|-----|-----------|
| `C-SPC` | ✅ | ❌(用 `C-@`) |
| `C-/` | ✅ | ❌(用 `C-_`) |
| `M-<` | ✅ | ✅ |

---

## 自定义快捷键

### 在 ~/.emacs 或 ~/.emacs.d/init.el 中添加:

```elisp
;; 全局绑定
(global-set-key (kbd "C-c c") 'compile)

;; 模式特定绑定
(add-hook 'python-mode-hook
  (lambda ()
    (local-set-key (kbd "C-c r") 'python-run)))

;; 取消绑定
(global-unset-key (kbd "C-x C-c"))
```

---

## 打印版本

### A4 打印布局

```
┌────────────────────────────────────┐
│  Emacs 快捷键速查表(核心版)          │
├────────────────────────────────────┤
│  文件:                              │
│    C-x C-f  打开   C-x C-s  保存   │
│  移动:                              │
│    C-f/b/n/p  光标  C-a/e  行首尾  │
│  编辑:                              │
│    C-d  删字符  C-k  删到行尾       │
│    C-SPC  标记  C-w  剪  M-w  复制 │
│    C-y  粘贴    C-/  撤销          │
│  窗口:                              │
│    C-x 2/3  分割  C-x o  切换      │
│  搜索:                              │
│    C-s  搜索    M-%  替换          │
│  帮助:                              │
│    C-h k  查快捷键  C-h t  教程    │
│  退出:                              │
│    C-x C-c  退出  C-g  取消        │
└────────────────────────────────────┘
```

---

## 在线资源

- **官方教程:** 启动 Emacs 后按 `C-h t`
- **官方手册:** `C-h i m Emacs`
- **快捷键查询:** `C-h k` 然后按快捷键
- **命令搜索:** `C-h a` 输入关键词

---

**提示:** 打印这份速查表贴在显示器旁边,最初两周频繁查阅,之后就会形成肌肉记忆!
