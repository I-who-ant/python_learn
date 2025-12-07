
Use C-x C-q : Change the file  ,   then use C-c C-c : save the file ( C-c C-k : give up the changes)

grep -rn : 作用 是搜索文件内容，-r 表示递归搜索，-n 表示显示行号，grep 是一个强大的文本搜索工具，可以用于搜索文件中的字符串或正则表达式。


 M-! (shell-command) - 执行单次命令

 C-c s : 打开 shell 


或许需要探讨下 Frame ( 窗口框架 , 通过 emacs 来递进了解)
Emacs 中，Frame 是指一个独立的窗口框架，可以包含多个 Buffer。每个 Frame 都有自己的标题栏、菜单栏、工具栏等界面元素，可以独立地显示和操作。)


## 自定义快捷键 (来自 misc-rc.el 和 ~/.emacs)

C-x C-g : find-file-at-point (打开光标下的文件路径)
C-, : duplicate-line (复制当前行)
C-x p d : insert-timestamp (插入时间戳)
C-c e : rc/open-config (快速打开 ~/.emacs 配置文件)
C-c C-e : rc/open-custom-file (快速打开 ~/.emacs.custom.el)


## 窗口大小调整快捷键

C-x ^ : enlarge-window (增加窗口高度)
C-x } : enlarge-window-horizontally (增加窗口宽度)
C-x { : shrink-window-horizontally (减少窗口宽度)
C-x + : balance-windows (平衡所有窗口大小)


## 删除快捷键

M-d : delete-word (删除光标后的词)
M-DEL / M-Backspace : backward-kill-word (删除光标前的词)
C-d : 删除光标后一个字符
DEL/Backspace : 删除光标前一个字符
C-k : 删除从光标到行尾
C-S-Backspace : kill-whole-line (删除整行)
C-w : kill-region (剪切选中区域)


## 多光标编辑 (需要 multiple-cursors 包)

C-S-c C-S-c : mc/edit-lines (在选中的每一行创建光标)
C-> : mc/mark-next-like-this (标记下一个相同的词)
C-< : mc/mark-previous-like-this (标记上一个相同的词)
C-c C-< : mc/mark-all-like-this (标记所有相同的词)

使用方法:
1. 选中区域 (C-Space 开始选择, 移动光标)
2. C-S-c C-S-c 在每行创建光标
3. 同时编辑所有行


## 查找和替换

C-s : isearch-forward (向前搜索)
C-r : isearch-backward (向后搜索)
M-% : query-replace (交互式替换)
C-M-% : query-replace-regexp (正则表达式替换)

### 全局替换步骤:
1. M-% (或 M-x query-replace)
2. 输入要替换的词
3. 输入替换后的词
4. 按 ! 全部替换 (或 y/n 逐个确认)

### 替换时的选项:
y : 替换当前匹配
n : 跳过当前匹配
! : 替换所有剩余匹配 (全局替换)
q : 退出替换
^ : 返回上一个匹配


## 注释操作

M-; : comment-dwim (智能注释/取消注释切换)

使用方法:
1. 选中区域 (C-Space 开始选择)
2. M-; (如果已注释则取消注释, 未注释则添加注释)