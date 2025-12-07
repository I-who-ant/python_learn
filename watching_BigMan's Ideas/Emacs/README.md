# Emacs 学习指南

欢迎来到 Emacs 学习之旅!本目录包含从零开始学习 Emacs 的完整教程。

---

## 📚 学习路线

### 适合完全新手

```
第1天: 阅读基础操作入门
    ↓
第2-3天: 按照实战练习教程练习 1-3
    ↓
第4-5天: 完成练习 4-7
    ↓
第6-7天: 完成练习 8-10
    ↓
第2周: 每天使用 Emacs,查阅速查表
    ↓
第3-4周: 探索配置和插件
```

---

## 📖 文档列表

### 1. [01-Emacs基础操作入门.md](./01-Emacs基础操作入门.md)
**适合:** 第一次接触 Emacs 的人
**内容:**
- Emacs 基本概念(Buffer, Window, Frame)
- 启动与退出
- 光标移动(C-f/b/n/p, C-a/e)
- 文本编辑(删除、复制、粘贴)
- 文件操作(打开、保存)
- 窗口管理(分割、切换)
- 搜索与替换
- 帮助系统

**预计阅读时间:** 30-45 分钟

**学习建议:**
- 边读边跟着操作
- 不要试图一次记住所有快捷键
- 重点记住"必记快捷键(生存技能)"部分

---

### 2. [02-Emacs快捷键速查表.md](./02-Emacs快捷键速查表.md)
**适合:** 已了解基础,需要快速查询的人
**内容:**
- 完整的快捷键分类列表
- 按功能分类(移动、编辑、文件、搜索等)
- 按使用频率排序(⭐⭐⭐⭐⭐ 到 ⭐⭐⭐)
- 新手必记 TOP 20
- 打印版本(可贴在显示器旁)

**使用方法:**
1. 打印或保存到手机
2. 忘记快捷键时快速查阅
3. 定期复习高频快捷键

---

### 3. [03-Emacs实战练习教程.md](./03-Emacs实战练习教程.md)
**适合:** 看完基础操作,需要实践的人
**内容:**
- 10 个循序渐进的实战练习
- 每个练习都有具体的操作步骤
- 包含检查点和完成标记
- 综合实战项目(购物清单管理器)

**练习列表:**
1. 第一次文件编辑 (5 分钟)
2. 光标移动训练 (10 分钟)
3. 文本编辑实战 (15 分钟)
4. 多窗口操作 (10 分钟)
5. 搜索与替换 (15 分钟)
6. Buffer 管理 (10 分钟)
7. 矩形编辑魔法 (10 分钟)
8. Kill Ring 循环粘贴 (10 分钟)
9. 标记跳转 (8 分钟)
10. 综合实战项目 (20 分钟)

**总时间:** 约 2-3 小时

---

### 4. [04-Emacs模式系统详解.md](./04-Emacs模式系统详解.md)
**适合:** 了解基础操作后,想深入理解 Emacs 工作原理的人
**内容:**
- Major Mode 与 Minor Mode 的区别
- Fundamental Mode vs Text Mode
- 编程语言模式(Python, C, etc.)
- Org Mode 完全指南
- Dired 文件管理模式
- 常用 Minor Modes(行号、自动补全、语法检查)
- 模式的激活与切换
- 自定义模式配置

**预计阅读时间:** 40-50 分钟

**核心知识点:**
- 一个 buffer 只能有 1 个 Major Mode
- 可以同时开启多个 Minor Modes
- 模式决定了编辑器的行为和快捷键
- 通过文件扩展名自动激活对应模式

**学习建议:**
- 创建不同类型的文件观察自动激活的模式
- 使用 `C-h m` 查看当前模式详情
- 尝试手动切换模式体会差异

---

## 🎯 学习目标

### 第 1 周目标
- [ ] 能不假思索地按 `C-x C-s` 保存
- [ ] 用 `C-f/b/n/p` 移动比方向键更快
- [ ] 记住文件操作快捷键
- [ ] 能用 `C-s` 搜索

### 第 2 周目标
- [ ] 熟练使用多窗口编辑
- [ ] 掌握复制粘贴(C-SPC, C-w, M-w, C-y)
- [ ] 会用 M-% 批量替换
- [ ] 能在多个 buffer 间切换

### 第 3 周目标
- [ ] 尝试矩形编辑
- [ ] 使用标记跳转
- [ ] 完成实战项目
- [ ] 能用 Emacs 完成日常工作

### 第 4 周目标
- [ ] Emacs 成为主力编辑器
- [ ] 探索配置文件
- [ ] 学习宏录制
- [ ] 考虑安装插件

---

## 🚀 快速开始

### 5 分钟体验

```bash
# 1. 启动 Emacs
emacs

# 2. 跟着提示按 C-h t 启动官方教程
#    或者直接打开我们的教程文件

# 3. 完成第一个练习:创建 hello.txt
C-x C-f ~/hello.txt
输入:Hello Emacs!
C-x C-s (保存)
C-x C-c (退出)
```

### 30 分钟入门

1. 阅读 `01-Emacs基础操作入门.md` 的前半部分
2. 完成 `03-Emacs实战练习教程.md` 的练习 1-3
3. 打印 `02-Emacs快捷键速查表.md` 贴在显示器旁

### 1 天掌握基础

1. 完整阅读 `01-Emacs基础操作入门.md`
2. 完成 `03-Emacs实战练习教程.md` 的所有练习
3. 尝试用 Emacs 编辑真实文件

---

## 💡 学习技巧

### 1. 不要死记硬背
- Emacs 快捷键有规律可循(C-f = Forward, C-b = Backward)
- 理解助记词比记忆更重要

### 2. 强迫自己使用
- 设置 Emacs 为默认编辑器
- 每天至少用 Emacs 30 分钟
- 不要遇到困难就切回老编辑器

### 3. 善用帮助系统
```
C-h k    查看快捷键
C-h f    查看函数
C-h t    官方教程
C-h m    当前模式帮助
```

### 4. 交换按键
Ctrl 键用得很频繁,建议交换 Caps Lock 和 Ctrl:

**Linux:**
```bash
setxkbmap -option ctrl:swapcaps
```

**macOS:**
系统偏好设置 → 键盘 → 修饰键

**Windows:**
使用 SharpKeys 或修改注册表

### 5. 循序渐进
```
第1天: 只用 C-f/b/n/p 移动
第2天: 加上 C-a/e 行首尾
第3天: 加上 C-s 搜索
第4天: 加上 C-w/M-w/C-y 复制粘贴
...
```

---

## 📊 学习进度检查

### 自测清单

**基础操作(必须掌握):**
- [ ] 能启动和退出 Emacs
- [ ] 能打开、保存、关闭文件
- [ ] 能用快捷键移动光标(不用方向键)
- [ ] 能删除、复制、粘贴文本
- [ ] 能搜索和替换
- [ ] 能用 C-g 取消命令
- [ ] 能用 C-/ 撤销操作

**进阶操作(逐步掌握):**
- [ ] 能分割窗口并在多窗口编辑
- [ ] 能在多个 buffer 间切换
- [ ] 能使用矩形编辑
- [ ] 能使用标记跳转
- [ ] 理解 Kill Ring 机制
- [ ] 会使用帮助系统

**高级技巧(可选):**
- [ ] 会录制和使用宏
- [ ] 会使用寄存器
- [ ] 会简单配置 Emacs
- [ ] 会安装和使用插件

---

## 🔧 推荐配置

### 最小化配置

创建 `~/.emacs` 或 `~/.emacs.d/init.el`:

```elisp
;; 显示行号
(global-display-line-numbers-mode 1)

;; 高亮当前行
(global-hl-line-mode 1)

;; 自动匹配括号
(electric-pair-mode 1)

;; 显示列号
(column-number-mode 1)

;; 关闭启动画面
(setq inhibit-startup-message t)

;; 关闭工具栏
(tool-bar-mode -1)

;; 更好的撤销系统(可选)
;; 需要先安装 undo-tree: M-x package-install RET undo-tree
;; (global-undo-tree-mode 1)
```

### 安装插件

```
M-x package-install RET
输入插件名,如:
- company (自动补全)
- magit (Git 集成)
- evil (Vim 模式)
```

---

## 🆚 Emacs vs 其他编辑器

### Emacs 的优势
- 极度可扩展(用 Elisp 定制一切)
- 统一的快捷键体系
- 强大的文本处理能力
- 活跃的社区,30+ 年历史
- 不只是编辑器(邮件、日程、笔记、浏览器...)

### Emacs 的劣势
- 学习曲线陡峭
- 默认配置简陋
- 快捷键与其他软件不同
- 需要时间培养肌肉记忆

### 何时选择 Emacs?
✅ 你是重度文本编辑用户
✅ 你喜欢键盘操作,讨厌鼠标
✅ 你愿意投入时间学习
✅ 你想要高度定制化的工具
✅ 你喜欢 Lisp 和可编程性

❌ 你只是偶尔编辑文本
❌ 你喜欢图形界面和鼠标
❌ 你想开箱即用不折腾
❌ 你已经熟练使用 Vim/VSCode/IDE

---

## 📚 扩展资源

### 官方资源
- **官方手册:** `C-h i m Emacs`
- **官方教程:** `C-h t`
- **官方网站:** [gnu.org/software/emacs](https://www.gnu.org/software/emacs/)

### 在线教程
- **Emacs Wiki:** [emacswiki.org](https://www.emacswiki.org/)
- **Mastering Emacs:** [masteringemacs.org](https://www.masteringemacs.org/)
- **Awesome Emacs:** [github.com/emacs-tw/awesome-emacs](https://github.com/emacs-tw/awesome-emacs)

### 视频教程
- YouTube 搜索: "Emacs tutorial for beginners"
- Bilibili 搜索: "Emacs 教程"

### 书籍推荐
- *Learning GNU Emacs* by Cameron et al.
- *Mastering Emacs* by Mickey Petersen
- *Practical Emacs Tutorial* (在线免费)

---

## 🎓 进阶学习路径

### 1. 配置篇
- 学习 Emacs Lisp 基础
- 编写自己的配置文件
- 理解 major mode 和 minor mode
- 自定义快捷键绑定

### 2. 插件篇
- 使用 package.el 管理插件
- 探索流行插件(company, ivy, magit)
- 尝试不同的主题
- 配置语言开发环境

### 3. 效率篇
- 掌握宏录制(C-x ( ))
- 学习寄存器使用
- 使用 abbrev 和 yasnippet
- 配置项目管理(projectile)

### 4. 专家篇
- 深入 Org-mode(笔记、任务、发布)
- 使用 Magit 管理 Git
- Emacs 作为邮件客户端
- 编写自己的 minor mode

---

## ❓ 常见问题

### Q: 学习 Emacs 需要多久?
**A:**
- 基础操作: 1-2 周
- 熟练使用: 1 个月
- 深度掌握: 3-6 个月
- 完全定制: 持续学习

### Q: Emacs vs Vim,哪个更好?
**A:** 都很强大,主要区别:
- Emacs: 可扩展性更强,Lisp 配置
- Vim: 模态编辑,更快速的操作
- 选择取决于个人喜好和使用场景

### Q: Emacs 能用来编程吗?
**A:** 完全可以!Emacs 支持几乎所有编程语言:
- 内置: C, Python, Lisp, Shell
- 插件: JavaScript, TypeScript, Rust, Go 等
- 代码补全、语法检查、调试都支持

### Q: 必须用终端版本吗?
**A:** 不必须,GUI 版本功能更丰富:
- 支持鼠标(虽然不推荐常用)
- 更好的颜色和字体
- 可以显示图片
- 建议初学者用 GUI 版

### Q: 手指会痛怎么办?
**A:**
1. 交换 Caps Lock 和 Ctrl 键
2. 使用外置键盘(更好的键程)
3. 定期休息,做手指操
4. 考虑使用人体工学键盘

---

## 🎯 下一步

### 完成基础学习后

1. **实践应用:**
   - 用 Emacs 写代码
   - 用 Emacs 写文档
   - 用 Emacs 管理笔记

2. **深入探索:**
   - 学习 Org-mode
   - 配置开发环境
   - 探索 Emacs 生态系统

3. **参与社区:**
   - 订阅 r/emacs
   - 关注 Emacs 博客
   - 分享你的配置

---

## 📝 反馈与贡献

如果你在学习过程中:
- 发现错误或不清楚的地方
- 有更好的讲解方式
- 想要补充新的内容

欢迎提出建议!

---

## 🎉 最后的话

> Emacs is a great operating system, lacking only a decent editor.
> (Emacs 是个伟大的操作系统,只是缺少一个好编辑器)
> —— 程序员笑话

虽然是玩笑,但也反映了 Emacs 的强大和复杂。学习 Emacs 就像学习一门武术,最初很难,但一旦掌握,将终身受益。

**祝你学习愉快!** 🚀

---

**开始你的 Emacs 之旅:** 打开 `01-Emacs基础操作入门.md`
