;;; .emacs --- Emacs 主配置文件

;;; Commentary:
;; 现代化的 Emacs 配置
;; 使用 custom-file 分离自动生成的配置
;; 模块化加载自定义配置

;;; Code:

;; ========================================
;; Custom File 设置
;; ========================================
;; 将 Customize 界面生成的配置单独存放
(setq custom-file "~/.emacs.custom.el")
(when (file-exists-p custom-file)
  (load custom-file))

;; ========================================
;; 包管理初始化
;; ========================================
(require 'package)
(setq package-archives '(("gnu"   . "https://elpa.gnu.org/packages/")
                         ("melpa" . "https://melpa.org/packages/")))
(package-initialize)

;; ========================================
;; 自定义加载路径
;; ========================================
(add-to-list 'load-path "~/.emacs.local/")

;; ========================================
;; 加载模块化配置
;; ========================================
;; 如果你有这些文件,取消注释
;; (load "~/.emacs.rc/rc.el" t)
;; (load "~/.emacs.rc/misc-rc.el" t)
;; (load "~/.emacs.rc/org-mode-rc.el" t)
;; (load "~/.emacs.rc/autocommit-rc.el" t)

;; ========================================
;; 外观设置
;; ========================================

;; 字体设置
(defun rc/get-default-font ()
  "根据操作系统返回默认字体"
  (cond
   ((eq system-type 'windows-nt) "Consolas-13")
   ((eq system-type 'gnu/linux) "Iosevka-20")
   ((eq system-type 'darwin) "Monaco-14")))

(add-to-list 'default-frame-alist `(font . ,(rc/get-default-font)))

;; 关闭不必要的界面元素
(tool-bar-mode 0)       ; 关闭工具栏
(menu-bar-mode 0)       ; 关闭菜单栏
(scroll-bar-mode 0)     ; 关闭滚动条

;; 显示增强
(column-number-mode 1)  ; 显示列号
(show-paren-mode 1)     ; 高亮匹配括号
(global-hl-line-mode 1) ; 高亮当前行

;; 行号显示
(global-display-line-numbers-mode 1)
;; 行号类型在 custom-file 中设置 (relative/absolute)

;; 主题设置
;; 确保主题包已安装: M-x package-install RET gruber-darker-theme
;; (load-theme 'gruber-darker t)

;; ========================================
;; 补全系统 (现代方案)
;; ========================================
;; 使用 Emacs 28+ 内置的 fido-mode
(fido-vertical-mode 1)

;; 如果你更喜欢水平显示:
;; (fido-mode 1)

;; ========================================
;; 编辑增强
;; ========================================

;; 括号自动配对
(electric-pair-mode 1)

;; 选中文本后输入替换
(delete-selection-mode 1)

;; 平滑滚动
(setq scroll-conservatively 101)

;; ========================================
;; 文件与备份设置
;; ========================================

;; 不创建备份文件 (~文件)
(setq make-backup-files nil)

;; 不创建自动保存文件 (#文件#)
(setq auto-save-default nil)

;; 关闭启动画面
(setq inhibit-startup-message t)

;; 关闭响铃
(setq ring-bell-function 'ignore)

;; 自动重新加载外部修改的文件
(global-auto-revert-mode 1)

;; ========================================
;; 缩进设置
;; ========================================

;; 使用空格而非 Tab
(setq-default indent-tabs-mode nil)

;; Tab 宽度
(setq-default tab-width 4)

;; C/C++ 缩进风格
(setq-default c-basic-offset 4
              c-default-style '((java-mode . "java")
                                (awk-mode . "awk")
                                (other . "bsd")))

;; ========================================
;; 编码设置
;; ========================================

;; 默认使用 UTF-8
(set-language-environment "UTF-8")
(set-default-coding-systems 'utf-8)
(prefer-coding-system 'utf-8)

;; ========================================
;; Dired 文件管理
;; ========================================

(require 'dired-x)

;; 隐藏隐藏文件
(setq dired-omit-files
      (concat dired-omit-files "\\|^\\..+$"))

;; 智能目标目录
(setq-default dired-dwim-target t)

;; 人类可读的文件大小
(setq dired-listing-switches "-alh")

;; ========================================
;; 空白字符处理
;; ========================================

(defun rc/set-up-whitespace-handling ()
  "启用空白字符可视化并在保存时删除行尾空白"
  (interactive)
  (whitespace-mode 1)
  (add-to-list 'write-file-functions 'delete-trailing-whitespace))

;; 为编程模式启用
(dolist (hook '(c-mode-hook
                c++-mode-hook
                python-mode-hook
                emacs-lisp-mode-hook
                rust-mode-hook
                go-mode-hook
                java-mode-hook
                lua-mode-hook
                haskell-mode-hook
                markdown-mode-hook))
  (add-hook hook 'rc/set-up-whitespace-handling))

;; ========================================
;; 编程语言特定设置
;; ========================================

;; Python
(add-hook 'python-mode-hook
          (lambda ()
            (setq python-indent-offset 4)))

;; Emacs Lisp
(add-hook 'emacs-lisp-mode-hook
          (lambda ()
            (local-set-key (kbd "C-c C-j") 'eval-print-last-sexp)))

;; Eldoc mode (函数文档提示)
(add-hook 'emacs-lisp-mode-hook 'eldoc-mode)

;; ========================================
;; 实用工具函数
;; ========================================

(defun rc/open-config ()
  "快速打开配置文件"
  (interactive)
  (find-file user-init-file))

(global-set-key (kbd "C-c e") 'rc/open-config)

(defun rc/open-custom-file ()
  "快速打开 custom-file"
  (interactive)
  (find-file custom-file))

(global-set-key (kbd "C-c C-e") 'rc/open-custom-file)

;; ========================================
;; 常用包配置
;; ========================================

;; 如果你安装了这些包,可以取消注释

;; Magit (Git 集成)
;; (when (require 'magit nil t)
;;   (global-set-key (kbd "C-c m s") 'magit-status)
;;   (global-set-key (kbd "C-c m l") 'magit-log))

;; Company (自动补全)
;; (when (require 'company nil t)
;;   (global-company-mode 1))

;; Multiple Cursors
;; (when (require 'multiple-cursors nil t)
;;   (global-set-key (kbd "C-S-c C-S-c") 'mc/edit-lines)
;;   (global-set-key (kbd "C->") 'mc/mark-next-like-this)
;;   (global-set-key (kbd "C-<") 'mc/mark-previous-like-this))

;; Paredit (Lisp 编辑)
;; (when (require 'paredit nil t)
;;   (add-hook 'emacs-lisp-mode-hook 'paredit-mode)
;;   (add-hook 'lisp-mode-hook 'paredit-mode)
;;   (add-hook 'scheme-mode-hook 'paredit-mode))

;; YASnippet (代码片段)
;; (when (require 'yasnippet nil t)
;;   (setq yas-snippet-dirs '("~/.emacs.snippets/"))
;;   (yas-global-mode 1))

;; ========================================
;; 加载私有配置
;; ========================================
;; 如果你有不想公开的配置,可以放在这里
(load "~/.emacs.shadow/shadow-rc.el" t)

;; ========================================
;; 性能优化
;; ========================================

;; 启动时提高垃圾回收阈值
(setq gc-cons-threshold (* 50 1000 1000))

;; 启动后恢复正常值
(add-hook 'emacs-startup-hook
          (lambda ()
            (setq gc-cons-threshold (* 2 1000 1000))
            (message "Emacs 启动完成! 加载时间: %s"
                     (emacs-init-time))))

;;; .emacs ends here
