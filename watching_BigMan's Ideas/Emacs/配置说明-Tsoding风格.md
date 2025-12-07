# Emacs 配置说明 - Tsoding 风格模块化配置

## 📁 配置目录结构

```
~/
├── .emacs                    # 主配置文件
├── .emacs.custom.el          # Customize 自动生成的配置
├── .emacs.rc/                # 模块化配置目录
│   ├── rc.el                 # 包管理工具
│   ├── misc-rc.el            # 实用工具函数
│   ├── org-mode-rc.el        # Org-mode 增强
│   ├── autocommit-rc.el      # 自动 Git 提交
│   └── 配置分析.md           # 详细文档
└── .emacs.local/             # 自定义语言模式
    ├── simpc-mode.el         # 轻量级 C/C++ 模式
    ├── c3-mode.el            # C3 语言
    ├── fasm-mode.el          # FASM 汇编
    ├── jai-mode.el           # Jai 语言
    ├── basm-mode.el          # BASM 汇编
    ├── porth-mode.el         # Porth 语言
    ├── noq-mode.el           # Noq 语言
    ├── umka-mode.el          # Umka 语言
    ├── nothings-mode.el      # Nothings 风格 C
    └── 配置建议.md           # 详细文档
```

---

## ✅ 已完成的配置

### 1. 创建了本地配置目录

```bash
~/.emacs.rc/      # 模块化配置
~/.emacs.local/   # 自定义语言模式
```

### 2. 复制了 Tsoding 的配置文件

所有文件已从 `watching_BigMan's Ideas/Tsoding/dotfiles-master/` 复制到本地。

### 3. 更新了 ~/.emacs 主配置

集成了 Tsoding 的模块化配置系统,默认启用:
- ✅ `rc.el` - 包管理工具
- ✅ `misc-rc.el` - 实用工具函数

其他模块已注释,按需启用。

---

## 🚀 当前配置特性

### 已启用的功能

#### 来自 rc.el:
- `rc/require` - 自动安装缺失的包
- `rc/require-theme` - 自动安装并加载主题
- MELPA 包源已启用

#### 来自 misc-rc.el:
- `C-x C-g` - 打开光标下的文件路径
- `C-c i m` - imenu 快速跳转
- `C-,` - 复制当前行
- `C-c M-q` - 反向填充段落
- `C-x p d` - 插入时间戳
- `C-x p s` - 搜索选中文本
- 编译输出自动着色
- 退出 Emacs 需要确认

#### 来自你的原配置:
- Fira Code 字体
- fido-vertical-mode 补全
- 相对行号显示
- 括号自动配对
- 语法高亮
- 等等...

---

## 📖 如何使用

### 立即生效

**重启 Emacs** 或在 Emacs 中执行:
```elisp
M-x load-file RET ~/.emacs RET
```

你会看到启动消息:
```
✓ 已加载: rc.el (包管理工具)
✓ 已加载: misc-rc.el (实用工具)
✓ Emacs 启动完成! 加载时间: 0.5s
```

---

### 测试新功能

#### 1. 测试 rc/require (自动安装包)

```elisp
;; 在 ~/.emacs 中取消注释这些行:
(rc/require 'magit)
(when (require 'magit nil t)
  (global-set-key (kbd "C-c m s") 'magit-status))

;; 重新加载配置
M-x load-file RET ~/.emacs RET

;; Magit 会自动安装!
```

#### 2. 测试快捷键

```
C-,         # 复制当前行
C-x C-g     # 打开光标下的文件路径
C-c i m     # 跳转到函数/类
C-x p d     # 插入时间戳
```

#### 3. 测试主题自动安装

```elisp
;; 在 ~/.emacs 中取消注释:
(rc/require-theme 'gruber-darker)

;; 重新加载配置,主题会自动安装并启用!
```

---

## 🔧 启用更多功能

### 启用 Org-mode 增强

```elisp
;; 在 ~/.emacs 中找到这几行,去掉注释:
(when (file-exists-p "~/.emacs.rc/org-mode-rc.el")
  (load "~/.emacs.rc/org-mode-rc.el")
  (message "✓ 已加载: org-mode-rc.el (Org-mode 增强)"))
```

**新增功能:**
- `C-x a` - Org Agenda
- `C-x p i` - 从 URL 自动提取标题插入链接
- `C-x p t` - 从 URL 创建 TODO 任务
- `C-c c` - Org Capture 快速捕获

**需要创建 Agenda 目录:**
```bash
mkdir -p ~/Documents/Agenda
cd ~/Documents/Agenda
git init
echo "* TODO 第一个任务" > Tasks.org
```

---

### 启用自动 Git 提交 (可选)

```elisp
;; 在 ~/.emacs 中取消注释:
(when (file-exists-p "~/.emacs.rc/autocommit-rc.el")
  (load "~/.emacs.rc/autocommit-rc.el")
  (message "✓ 已加载: autocommit-rc.el (自动 Git 提交)"))
```

**初始化 Autocommit 目录:**
```elisp
M-x rc/autocommit-init-dir RET ~/Documents/Agenda RET
```

**效果:** 每次保存文件自动 `git commit && git push`

---

### 启用自定义语言模式

#### 轻量级 C/C++ 模式 (推荐)

```elisp
;; 在 ~/.emacs 中取消注释:
(when (file-exists-p "~/.emacs.local/simpc-mode.el")
  (require 'simpc-mode)
  ;; 可选: 替代默认 c-mode
  (add-to-list 'auto-mode-alist '("\\.c\\'" . simpc-mode))
  (add-to-list 'auto-mode-alist '("\\.h\\'" . simpc-mode))
  (message "✓ 已加载: simpc-mode"))
```

**优点:** 比 c-mode 启动快,语法高亮够用

#### 其他语言模式

```elisp
;; C3 语言
(when (file-exists-p "~/.emacs.local/c3-mode.el")
  (require 'c3-mode))

;; FASM 汇编
(when (file-exists-p "~/.emacs.local/fasm-mode.el")
  (require 'fasm-mode))

;; Jai 语言
(when (file-exists-p "~/.emacs.local/jai-mode.el")
  (require 'jai-mode))
```

---

### 安装常用包 (使用 rc/require)

```elisp
;; 在 ~/.emacs 中取消注释并根据需要修改:

;; Git 集成
(rc/require 'magit)
(global-set-key (kbd "C-c m s") 'magit-status)

;; 自动补全
(rc/require 'company)
(global-company-mode 1)

;; 多光标编辑
(rc/require 'multiple-cursors)
(global-set-key (kbd "C->") 'mc/mark-next-like-this)

;; 快捷键提示
(rc/require 'which-key)
(which-key-mode 1)

;; 彩虹括号
(rc/require 'rainbow-delimiters)
(add-hook 'prog-mode-hook 'rainbow-delimiters-mode)
```

---

## 📋 新增快捷键速查

### 来自 misc-rc.el

| 快捷键 | 功能 | 说明 |
|--------|------|------|
| `C-x C-g` | `find-file-at-point` | 打开光标下的文件路径 |
| `C-c i m` | `imenu` | 跳转到函数/类定义 |
| `C-,` | `rc/duplicate-line` | 复制当前行 |
| `C-c M-q` | `rc/unfill-paragraph` | 合并段落为一行 |
| `C-x p d` | `rc/insert-timestamp` | 插入时间戳 `(20251206-153000)` |
| `C-x p s` | `rc/rgrep-selected` | 递归搜索选中文本 |

### 来自 org-mode-rc.el (需要启用)

| 快捷键 | 功能 | 说明 |
|--------|------|------|
| `C-x a` | `org-agenda` | 打开 Org Agenda |
| `C-x p i` | `org-cliplink` | 插入剪贴板 URL 链接 |
| `C-x p t` | `rc/cliplink-task` | 从 URL 创建 TODO 任务 |
| `C-x p w` | `rc/org-kill-heading-name-save` | 复制标题名称 |
| `C-c c` | `org-capture` | 快速捕获任务 |
| `C-x a u` | Agenda 视图 | 未计划任务 |
| `C-x a p` | Agenda 视图 | 个人任务 |
| `C-x a w` | Agenda 视图 | 工作任务 |

### 原有快捷键 (保留)

| 快捷键 | 功能 |
|--------|------|
| `C-c e` | 打开配置文件 |
| `C-c C-e` | 打开 custom-file |

---

## 🎨 自定义配置示例

### 示例 1: 最小配置 (当前状态)

```elisp
# 只加载了:
- rc.el (包管理)
- misc-rc.el (实用工具)

# 适合:
- 新手
- 想要轻量级配置
```

### 示例 2: Org-mode 用户

```elisp
# 在 ~/.emacs 中启用:
- rc.el
- misc-rc.el
- org-mode-rc.el  ← 取消注释

# 适合:
- 使用 Org-mode 管理任务
- 需要 GTD 工作流
```

### 示例 3: 完整配置

```elisp
# 在 ~/.emacs 中启用:
- rc.el
- misc-rc.el
- org-mode-rc.el
- autocommit-rc.el  ← 取消注释
- simpc-mode.el     ← 取消注释

# 安装常用包:
(rc/require 'magit 'company 'which-key 'rainbow-delimiters)

# 适合:
- 重度 Emacs 用户
- 需要完整功能
```

---

## ⚠️ 注意事项

### 1. 配置冲突

某些设置在 `misc-rc.el` 和你的 `.emacs` 中都有,比如:
- `make-backup-files nil`
- `indent-tabs-mode nil`
- `tab-width 4`

这是正常的,后加载的设置会覆盖前面的。

### 2. 快捷键覆盖

`misc-rc.el` 定义的快捷键可能和你的习惯冲突。

**解决方法:**
```elisp
;; 在 ~/.emacs 的末尾重新绑定
(global-set-key (kbd "C-,") 'my-other-function)
```

### 3. 启动时间

加载 `rc.el` 和 `misc-rc.el` 会增加约 0.02-0.05 秒启动时间,完全可以接受。

---

## 🔍 故障排查

### 问题 1: 启动报错 "Cannot find file rc.el"

**原因:** 文件路径不对

**解决:**
```bash
ls ~/.emacs.rc/rc.el
# 确认文件存在
```

### 问题 2: rc/require 不工作

**原因:** rc.el 没有加载

**解决:**
```elisp
;; 在 *Messages* buffer 中查看
M-x switch-to-buffer RET *Messages* RET

;; 应该看到:
;; ✓ 已加载: rc.el (包管理工具)
```

### 问题 3: 快捷键不生效

**原因:** misc-rc.el 没有加载

**解决:** 同上,检查 *Messages* buffer

---

## 📚 相关文档

- **rc.el 详细分析:** `~/.emacs.rc/配置分析.md`
- **语言模式分析:** `~/.emacs.local/配置建议.md`
- **Emacs 教程:** `watching_BigMan's Ideas/Emacs/` 目录

---

## 🎯 下一步建议

### 立即可做:

1. **重启 Emacs** 查看新配置
2. **测试新快捷键** (C-,, C-x C-g 等)
3. **尝试自动安装包**:
   ```elisp
   ;; 在 ~/.emacs 中添加
   (rc/require 'magit)
   (rc/require-theme 'gruber-darker)
   ```

### 按需启用:

4. **如果你使用 Org-mode** → 启用 `org-mode-rc.el`
5. **如果你编写 C 代码** → 启用 `simpc-mode.el`
6. **如果你想要自动备份笔记** → 启用 `autocommit-rc.el`

---

## ✨ 总结

### 当前配置状态:

```
✅ 已集成 Tsoding 的模块化配置系统
✅ 包管理工具 (rc/require) 可用
✅ 实用工具函数已加载
✅ 自定义语言模式已就绪 (需手动启用)
✅ 配置可移植性提升
```

### 核心优势:

1. **模块化** - 配置分类清晰
2. **可扩展** - 按需启用功能
3. **自动化** - rc/require 自动安装包
4. **实用** - 丰富的快捷键和工具函数

---

**祝你使用愉快!** 🚀

有任何问题可以查看:
- `~/.emacs.rc/配置分析.md`
- `~/.emacs.local/配置建议.md`
