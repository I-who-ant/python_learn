# Emacs 补全系统详解:ido/smex/fido 对比

## 目录
1. [问题背景](#问题背景)
2. [什么是命令补全](#什么是命令补全)
3. [历史演进](#历史演进)
4. [实战对比](#实战对比)
5. [推荐方案](#推荐方案)

---

## 问题背景

### 你看到的这句话是什么意思?

```
"Emacs 28 自带了 fido-mode 和 fido-vertical-mode，
所以你不再需要 smex 和 ido"
```

**简单解释:**
- Emacs 的不同版本提供了不同的**命令补全系统**
- 旧版本需要安装插件(`smex`, `ido`)
- **Emacs 28+** 已经内置了更好的方案(`fido-mode`)
- 就像手机升级后内置了原本需

## 什么是命令补全?

### 场景:执行命令时的体验差异
要下载的 App

---
#### 问题:在 Emacs 中执行命令

当你按 `M-x` (执行命令) 时,需要输入命令名:

```
M-x save-buffer
M-x python-mode
M-x package-install
...
```

**痛点:**
- 命令名很长,容易打错
- 不记得完整命令名
- 想快速找到命令

---

### 不同补全系统的体验对比

#### 1. 默认补全(最原始,体验差)

```
操作:
M-x sa [按 Tab]

显示:
Possible completions are:
save-buffer
save-buffers-kill-emacs
save-current-buffer
save-excursion
save-restriction
...

问题:
❌ 弹出一个大窗口,打断操作流
❌ 需要输入完整前缀才能补全
❌ 不支持模糊匹配
```

**视觉效果:**
```
┌─────────────────────────┐
│ M-x sa_                 │  ← 输入区
├─────────────────────────┤
│ Possible completions:   │
│ save-buffer             │
│ save-buffers-kill-emacs │
│ save-current-buffer     │
│ ...                     │  ← 候选窗口(占据屏幕)
└─────────────────────────┘
```

---

#### 2. ido-mode (改进,但需要配置)

**ido** = **I**nteractive **DO** things

```
操作:
M-x sa

显示(在 minibuffer 中):
M-x {save-buffer | save-buffers-kill-emacs | save-current-buffer}
     ^^^^^^^^^^^
      当前选中

特点:
✅ 在底部 minibuffer 显示,不弹窗
✅ 支持模糊匹配(sa → save-buffer)
✅ C-s/C-r 快速切换候选
❌ 需要手动配置
❌ 只能用于某些场景(打开文件、切换 buffer)
```

**视觉效果:**
```
┌────────────────────────────────┐
│                                │
│      你的编辑区域               │
│                                │
├────────────────────────────────┤
│ M-x {save-buffer | save-buffers-kill-emacs | ...} │ ← 底部显示候选
└────────────────────────────────┘
```

---

#### 3. smex (ido 的增强,需要安装插件)

**smex** = **S**mart **M-x** **Ex**tension

```
操作:
M-x sa

特点:
✅ 基于 ido,补全体验好
✅ 记住常用命令,优先显示
✅ 模糊匹配
❌ 需要安装第三方插件
❌ 不再维护(停更)

示例:
M-x save
显示:
{save-buffer | save-current-buffer | save-excursion}
 ^^^^^^^^^^^
  最常用的排在最前面
```

---

#### 4. fido-mode (Emacs 28+,**现代方案**) ⭐

**fido** = **F**ake **IDO**

```
操作:
M-x sa

特点:
✅ 内置于 Emacs 28+,无需安装
✅ 模糊匹配(flexible matching)
✅ 在 minibuffer 显示
✅ 简洁高效
✅ 支持所有补全场景

示例:
M-x sa
显示:
save-buffer
save-buffers-kill-emacs
save-current-buffer
```

**视觉效果(水平):**
```
M-x {save-buffer | save-buffers-kill-emacs | save-current-buffer}
     ^^^^^^^^^^^
```

**视觉效果(垂直,fido-vertical-mode):**
```
M-x sa
→ save-buffer              ← 当前选中
  save-buffers-kill-emacs
  save-current-buffer
  save-excursion
```

---

## 历史演进

### 时间线:Emacs 补全系统的发展

```
┌─────────────────────────────────────────┐
│ 2000年代早期                             │
│ 默认补全                                 │
│ ❌ 体验差,弹窗打断                       │
└──────────────┬──────────────────────────┘
               ↓
┌─────────────────────────────────────────┐
│ 2005年左右                               │
│ ido-mode 出现                            │
│ ✅ 改进补全体验                          │
│ ❌ 只支持部分场景(文件、buffer)          │
└──────────────┬──────────────────────────┘
               ↓
┌─────────────────────────────────────────┐
│ 2010年左右                               │
│ smex 插件                                │
│ ✅ 让 M-x 也能用 ido 风格补全            │
│ ✅ 记住常用命令                          │
│ ❌ 需要安装插件                          │
└──────────────┬──────────────────────────┘
               ↓
┌─────────────────────────────────────────┐
│ 2020年 (Emacs 27)                        │
│ icomplete-mode 改进                      │
│ ✅ 内置补全增强                          │
└──────────────┬──────────────────────────┘
               ↓
┌─────────────────────────────────────────┐
│ 2021年 (Emacs 28) ← 现在                 │
│ fido-mode / fido-vertical-mode           │
│ ✅ 内置,无需插件                         │
│ ✅ 结合 ido 和 icomplete 优点            │
│ ✅ 支持所有场景                          │
│ ✅ 模糊匹配                              │
└─────────────────────────────────────────┘
```

---

## 实战对比

### 场景 1:执行命令 (M-x)

#### 使用默认补全

```
操作:
M-x package-ins [Tab]

结果:
弹出大窗口显示所有 package-in* 开头的命令
需要继续输入完整名称
```

#### 使用 smex (旧方案,需要安装)

```
操作:
M-x pack [自动匹配]

结果:
{package-install | package-list-packages | package-refresh-contents}
 ^^^^^^^^^^^^^^^^
  最常用的排第一

优点:
✅ 自动补全
✅ 智能排序
缺点:
❌ 需要安装: M-x package-install RET smex
❌ 需要配置: (smex-initialize)
```

#### 使用 fido-mode (新方案,内置) ⭐

```
操作:
M-x pack [自动匹配]

结果:
package-install
package-list-packages
package-refresh-contents

优点:
✅ 无需安装,Emacs 28+ 自带
✅ 模糊匹配
✅ 简洁高效

启用方法:
(fido-mode 1)  ; 水平显示
或
(fido-vertical-mode 1)  ; 垂直显示(推荐)
```

---

### 场景 2:打开文件 (C-x C-f)

#### 默认补全

```
C-x C-f ~/Doc [Tab]

结果:
弹出窗口显示 ~/Documents/
需要继续输入
```

#### fido-mode

```
C-x C-f ~/Doc

结果(实时显示):
~/Documents/
~/Downloads/

特点:
✅ 即时匹配
✅ 模糊搜索(Doc → Documents)
✅ C-n/C-p 选择
```

---

### 场景 3:切换 buffer (C-x b)

#### 默认补全

```
C-x b init [Tab]

结果:
需要输入完整名称
```

#### fido-mode

```
C-x b init

结果:
init.el
init-backup.el
init-old.el

操作:
C-n/C-p 选择
RET 确认
```

---

## fido-mode 详细使用

### 启用 fido-mode

在 `~/.emacs` 中添加:

```elisp
;; 方式1:水平显示候选(紧凑)
(fido-mode 1)

;; 方式2:垂直显示候选(推荐,更清晰)
(fido-vertical-mode 1)
```

**对比效果:**

**fido-mode (水平):**
```
M-x {save-buffer | save-buffers-kill-emacs | save-current-buffer}
```

**fido-vertical-mode (垂直):**
```
M-x save
→ save-buffer              ← 箭头指示当前选中
  save-buffers-kill-emacs
  save-current-buffer
  save-excursion
  save-restriction
```

---

### fido-mode 快捷键

在补全界面中:

| 快捷键 | 作用 |
|--------|------|
| `C-n` / `C-s` | 下一个候选 |
| `C-p` / `C-r` | 上一个候选 |
| `RET` | 确认选择 |
| `C-j` | 确认当前输入(不选择候选) |
| `C-g` | 取消 |
| `TAB` | 完成当前候选 |
| `SPC` | 继续输入空格(不选择) |

---

### 实战演示:使用 fido-vertical-mode

**步骤 1:启用**

```elisp
;; 在 ~/.emacs 中添加
(fido-vertical-mode 1)

;; 保存并执行
C-x C-s
M-x eval-buffer
```

**步骤 2:测试 M-x 补全**

```
M-x pack

显示:
→ package-install              ← C-n 选择下一个
  package-list-packages        ← C-p 选择上一个
  package-refresh-contents
  package-delete

输入:
继续输入 i
过滤为:
→ package-install

确认:
按 RET
```

**步骤 3:测试文件打开**

```
C-x C-f ~/Doc

显示:
→ ~/Documents/
  ~/Downloads/

选择:
C-n 或继续输入 u
变成:
→ ~/Documents/

按 RET 进入目录
```

---

## 为什么不再需要 smex 和 ido?

### 原因分析

| 特性 | ido + smex (旧) | fido-mode (新) |
|------|----------------|----------------|
| **安装** | 需要安装插件 | ✅ 内置,无需安装 |
| **配置** | 需要复杂配置 | ✅ 一行代码启用 |
| **模糊匹配** | ✅ 支持 | ✅ 支持 |
| **M-x 补全** | ✅ (需要 smex) | ✅ 原生支持 |
| **文件补全** | ✅ (需要 ido) | ✅ 原生支持 |
| **维护状态** | ❌ smex 停更 | ✅ 活跃维护 |
| **性能** | 一般 | ✅ 更快 |

### 迁移建议

**如果你用的是 Emacs 28+:**

```elisp
;; ❌ 旧配置(可以删除)
(require 'ido)
(ido-mode 1)
(ido-everywhere 1)

(require 'smex)
(smex-initialize)
(global-set-key (kbd "M-x") 'smex)

;; ✅ 新配置(推荐)
(fido-vertical-mode 1)
```

就这么简单!一行搞定!

---

## 推荐方案

### 方案 1:最简配置(推荐新手)

```elisp
;; 只需要一行
(fido-vertical-mode 1)
```

**效果:**
- ✅ M-x 补全
- ✅ C-x C-f 文件补全
- ✅ C-x b buffer 补全
- ✅ 模糊匹配
- ✅ 垂直显示,清晰直观

---

### 方案 2:增强配置

```elisp
;; 启用 fido 垂直模式
(fido-vertical-mode 1)

;; 调整显示行数(默认 10)
(setq icomplete-max-delay-chars 0)  ; 立即显示候选
(setq icomplete-compute-delay 0)    ; 无延迟计算
(setq icomplete-show-matches-on-no-input t)  ; 无输入时也显示

;; 设置候选列表高度
(setq icomplete-prospects-height 1)  ; 只显示当前候选(紧凑)
;; 或
(setq icomplete-prospects-height 10) ; 显示 10 个候选
```

---

### 方案 3:高级用户(ivy/counsel 或 vertico)

如果你想要更强大的功能:

```elisp
;; 选项1: ivy/counsel (成熟稳定)
(use-package ivy
  :ensure t
  :config
  (ivy-mode 1))

(use-package counsel
  :ensure t
  :config
  (counsel-mode 1))

;; 选项2: vertico (现代轻量)
(use-package vertico
  :ensure t
  :config
  (vertico-mode 1))
```

**但对于新手,fido-vertical-mode 已经足够!**

---

## 完整配置示例

### 推荐的 ~/.emacs 配置

```elisp
;;; init.el --- Emacs 配置文件

;;; Commentary:
;; 现代 Emacs 配置

;;; Code:

;; ========== 补全系统 ==========

;; 启用 fido 垂直补全
(fido-vertical-mode 1)

;; ========== 基础设置 ==========

;; 显示行号
(global-display-line-numbers-mode 1)

;; 高亮当前行
(global-hl-line-mode 1)

;; 括号匹配
(electric-pair-mode 1)

;; 显示列号
(column-number-mode 1)

;;; init.el ends here
```

保存并执行:
```
C-x C-s (保存)
M-x eval-buffer (立即生效)
```

---

## 测试你的配置

### 测试清单

**1. 测试 M-x 补全**
```
M-x pack
→ 应该显示垂直候选列表
→ C-n/C-p 可以选择
→ 输入更多字符可以过滤
```

**2. 测试文件打开**
```
C-x C-f ~/
→ 应该显示目录列表
→ 支持模糊匹配
```

**3. 测试 buffer 切换**
```
C-x b
→ 显示所有 buffer
→ 输入名称过滤
```

---

## 常见问题

### Q1: 我用的是 Emacs 27,没有 fido-mode?

**A:** 升级到 Emacs 28+ 或使用 icomplete-mode:

```elisp
;; Emacs 27 替代方案
(icomplete-mode 1)
(icomplete-vertical-mode 1)  ; 如果有
```

### Q2: fido-mode 和 ido-mode 可以同时用吗?

**A:** 不推荐,选一个:

```elisp
;; 二选一

;; 方案1:fido-mode (推荐)
(fido-vertical-mode 1)

;; 方案2:ido-mode (旧)
(ido-mode 1)
(ido-everywhere 1)
```

### Q3: 我已经安装了 smex,需要卸载吗?

**A:** 不必须,但可以简化配置:

```elisp
;; 注释掉或删除 smex 相关配置
;; (require 'smex)
;; (smex-initialize)

;; 改用 fido
(fido-vertical-mode 1)
```

### Q4: 候选太多,能限制数量吗?

**A:** 可以:

```elisp
(setq icomplete-prospects-height 5)  ; 只显示 5 个候选
```

---

## 总结

### 一句话总结

> **Emacs 28+ 内置的 `fido-mode` 替代了以前需要安装的 `ido` 和 `smex` 插件,提供了更好的补全体验。**

### 核心要点

1. **fido-mode** = 现代的、内置的补全系统
2. **ido + smex** = 旧的、需要安装插件的方案
3. **Emacs 28+** 用户只需 `(fido-vertical-mode 1)` 一行配置
4. **效果:** 模糊匹配 + 垂直显示 + 无需插件

### 推荐配置

```elisp
;; 在 ~/.emacs 中添加这一行就够了
(fido-vertical-mode 1)
```

就这么简单! 🎉

---

## 练习任务

### 任务:体验 fido-vertical-mode

**步骤 1:配置**
```elisp
;; 在 ~/.emacs 添加
(fido-vertical-mode 1)

;; 保存并生效
C-x C-s
M-x eval-buffer
```

**步骤 2:测试**
```
1. M-x package-in
   观察垂直候选列表

2. C-n/C-p 选择候选

3. 继续输入字符过滤

4. RET 确认
```

**步骤 3:对比**
```
1. 关闭 fido-mode:
   M-x fido-mode RET

2. 再次测试 M-x package-in
   观察差异

3. 重新启用:
   M-x fido-vertical-mode RET
```

完成这个练习,你就理解补全系统的差异了! 🚀
