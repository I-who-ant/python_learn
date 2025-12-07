;; custom-set-variables was added by Custom.
;; If you edit it by hand, you could mess it up, so be careful.
;; Your init file should contain only one such instance.
;; If there is more than one, they won't work right.

(custom-set-variables
 ;; ========================================
 ;; 显示设置
 ;; ========================================

 ;; 相对行号 (显示距离当前行的距离, Vim 风格)
 '(display-line-numbers-type 'relative)

 ;; 或使用绝对行号 (注释掉上面,取消注释下面)
 ;; '(display-line-numbers-type t)

 ;; ========================================
 ;; 警告级别
 ;; ========================================

 ;; 只显示错误级别的警告 (可选, 如果觉得警告太多)
 ;; 建议注释掉或改为 :warning
 ;; '(warning-minimum-level :error)

 ;; ========================================
 ;; 空白字符显示
 ;; ========================================

 '(whitespace-style
   '(face tabs spaces trailing space-before-tab newline
     indentation empty space-after-tab space-mark tab-mark))

 ;; ========================================
 ;; Org Mode 设置
 ;; ========================================

 ;; Org 模块
 '(org-modules
   '(org-bbdb org-bibtex org-docview org-gnus org-habit
     org-info org-irc org-mhe org-rmail org-w3m))

 ;; Org Agenda
 '(org-agenda-dim-blocked-tasks nil)
 '(org-enforce-todo-dependencies nil)
 '(org-refile-use-outline-path 'file)

 ;; Org Cliplink
 '(org-cliplink-transport-implementation 'url-el)

 ;; ========================================
 ;; 主题设置
 ;; ========================================

 ;; 启用的主题
 '(custom-enabled-themes '(gruber-darker))

 ;; 安全主题列表 (避免每次加载主题时询问)
 '(custom-safe-themes
   '("e13beeb34b932f309fb2c360a04a460821ca99fe58f69e65557d6c1b10ba18c7"
     default))

 ;; ========================================
 ;; 包管理
 ;; ========================================

 ;; 已安装的包列表
 '(package-selected-packages
   '(;; 主题
     gruber-darker-theme
     zenburn-theme

     ;; 编辑增强
     multiple-cursors
     move-text
     paredit
     yasnippet

     ;; 补全与导航
     company
     helm
     helm-git-grep
     helm-ls-git

     ;; Git 集成
     magit

     ;; 编程语言支持
     haskell-mode
     rust-mode
     go-mode
     python-mode
     elpy
     lua-mode
     markdown-mode
     yaml-mode
     typescript-mode
     tide
     clojure-mode
     racket-mode
     scala-mode
     d-mode
     nim-mode
     php-mode
     kotlin-mode
     csharp-mode

     ;; 前端开发
     js2-mode
     less-css-mode

     ;; 其他工具
     powershell
     dockerfile-mode
     toml-mode
     nix-mode
     cmake-mode
     nginx-mode
     jinja2-mode
     glsl-mode
     graphviz-dot-mode
     proof-general
     ag
     editorconfig
     org-cliplink
     dash
     dash-functional))

 ;; ========================================
 ;; 安全的本地变量
 ;; ========================================

 '(safe-local-variable-values
   '(;; 允许自动重载和自动提交
     (eval progn
           (auto-revert-mode 1)
           (add-hook 'after-save-hook
                     'rc/autocommit-changes
                     nil
                     'make-it-local)))))

;; ========================================
;; 自定义 Faces (外观)
;; ========================================

(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.

 ;; 示例: 自定义注释颜色
 ;; '(font-lock-comment-face ((t (:foreground "#7f9f7f" :slant italic))))

 ;; 示例: 自定义关键字颜色
 ;; '(font-lock-keyword-face ((t (:foreground "#cc9393" :weight bold))))
 )

;; ========================================
;; 说明
;; ========================================

;; 这个文件由 Emacs 的 Customize 系统自动生成和管理
;;
;; 推荐做法:
;; 1. 通过 M-x customize 图形界面修改设置
;; 2. 点击 [Apply and Save] 保存到此文件
;; 3. 避免手动编辑,除非你清楚自己在做什么
;;
;; 如果要添加自定义配置,请添加到 ~/.emacs 中,而不是这里

;;; .emacs.custom.el ends here
