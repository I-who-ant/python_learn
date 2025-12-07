# X Window System 配置命令详解

## 目录
1. [xset - X 用户偏好设置工具](#xset)
2. [xrandr - 显示器配置工具](#xrandr)
3. [xmodmap - 键盘映射修改工具](#xmodmap)
4. [setxkbmap - 键盘布局设置工具](#setxkbmap)
5. [xrdb - X 资源数据库管理工具](#xrdb)
6. [实战示例](#实战示例)

---

## xset

### 简介
`xset` 是 X Window System 的用户偏好设置工具,用于配置显示、键盘、鼠标、屏幕保护等各种 X 服务器参数。

### 基本语法
```bash
xset [选项] [参数]
```

### 主要功能

#### 1. 显示电源管理(DPMS)
```bash
# 启用 DPMS(显示电源管理信号)
xset +dpms

# 禁用 DPMS
xset -dpms

# 设置待机、挂起、关闭的时间(秒)
xset dpms 300 600 900
# 300秒后待机,600秒后挂起,900秒后关闭

# 强制进入某个状态
xset dpms force standby    # 强制待机
xset dpms force suspend    # 强制挂起
xset dpms force off        # 强制关闭显示器
xset dpms force on         # 强制开启显示器
```

#### 2. 屏幕保护设置
```bash
# 设置屏幕保护时间(秒)
xset s 300              # 300秒后启动屏幕保护

# 禁用屏幕保护
xset s off

# 启用屏幕保护
xset s on

# 设置屏幕保护为黑屏
xset s blank

# 设置屏幕保护不黑屏
xset s noblank

# 立即激活屏幕保护
xset s activate

# 重置屏幕保护计时器
xset s reset
```

#### 3. 键盘设置
```bash
# 设置键盘重复延迟和速率
xset r rate 200 30
# 200ms 延迟后开始重复,每秒重复30次

# 禁用键盘自动重复
xset -r

# 启用键盘自动重复
xset r on

# 禁用特定键的自动重复
xset -r 64              # 禁用键码64(通常是 Alt 键)

# 启用特定键的自动重复
xset r 64
```

#### 4. 鼠标设置
```bash
# 设置鼠标加速度和阈值
xset m 2/1 4
# 加速倍数 2/1,移动4个像素后开始加速

# 恢复鼠标默认设置
xset m default

# 禁用鼠标加速
xset m 1/1 0
```

#### 5. 蜂鸣器(Bell)设置
```bash
# 设置蜂鸣器音量、音调、持续时间
xset b 50 1000 100
# 音量50%,音调1000Hz,持续100ms

# 禁用蜂鸣器
xset -b
xset b off

# 启用蜂鸣器
xset b on
```

#### 6. LED 指示灯
```bash
# 开启键盘 LED 灯(1=Num Lock, 2=Caps Lock, 3=Scroll Lock)
xset led 1              # 开启 Num Lock 灯

# 关闭键盘 LED 灯
xset -led 2             # 关闭 Caps Lock 灯
```

#### 7. 字体路径
```bash
# 设置字体路径
xset fp= /usr/share/fonts/TTF,/usr/share/fonts/Type1

# 添加字体路径
xset +fp /path/to/fonts

# 移除字体路径
xset -fp /path/to/fonts

# 恢复默认字体路径
xset fp default

# 重新扫描字体目录
xset fp rehash
```

#### 8. 查询当前设置
```bash
# 显示当前所有设置
xset q
```

### 常用组合示例
```bash
# 禁用屏幕保护和电源管理(适合演示、看视频)
xset s off -dpms

# 快速键盘重复(适合程序员)
xset r rate 150 40

# 节能设置(5分钟待机)
xset dpms 300 600 900

# 禁用蜂鸣器(安静模式)
xset -b
```

---

## xrandr

### 简介
`xrandr` 是 RandR(Resize and Rotate)扩展的命令行工具,用于配置显示器分辨率、刷新率、旋转、多显示器布局等。

### 基本语法
```bash
xrandr [选项]
```

### 主要功能

#### 1. 查看显示器信息
```bash
# 列出所有显示器及支持的分辨率
xrandr

# 详细输出
xrandr --verbose

# 仅列出已连接的显示器
xrandr --listmonitors
```

#### 2. 设置分辨率和刷新率
```bash
# 设置分辨率
xrandr --output HDMI-1 --mode 1920x1080

# 设置分辨率和刷新率
xrandr --output HDMI-1 --mode 1920x1080 --rate 60

# 使用最佳分辨率
xrandr --output HDMI-1 --auto
```

#### 3. 屏幕旋转
```bash
# 旋转显示器
xrandr --output HDMI-1 --rotate left    # 左旋转90度
xrandr --output HDMI-1 --rotate right   # 右旋转90度
xrandr --output HDMI-1 --rotate inverted # 倒转180度
xrandr --output HDMI-1 --rotate normal  # 正常方向
```

#### 4. 多显示器配置
```bash
# 扩展显示(双屏)
xrandr --output HDMI-1 --auto --right-of eDP-1

# 镜像显示
xrandr --output HDMI-1 --same-as eDP-1

# 设置主显示器
xrandr --output eDP-1 --primary

# 关闭显示器
xrandr --output HDMI-1 --off

# 开启显示器
xrandr --output HDMI-1 --auto
```

#### 5. 显示器定位
```bash
# HDMI-1 在 eDP-1 右侧
xrandr --output HDMI-1 --right-of eDP-1

# HDMI-1 在 eDP-1 左侧
xrandr --output HDMI-1 --left-of eDP-1

# HDMI-1 在 eDP-1 上方
xrandr --output HDMI-1 --above eDP-1

# HDMI-1 在 eDP-1 下方
xrandr --output HDMI-1 --below eDP-1
```

#### 6. 亮度调节
```bash
# 设置亮度(0.0-1.0)
xrandr --output eDP-1 --brightness 0.8

# 完全调暗(但不关闭)
xrandr --output eDP-1 --brightness 0.5
```

#### 7. 缩放设置
```bash
# 设置缩放比例
xrandr --output eDP-1 --scale 1.5x1.5

# 设置 DPI
xrandr --dpi 96
```

#### 8. 添加自定义分辨率
```bash
# 计算模式线(使用 cvt 工具)
cvt 1920 1080 60

# 添加新模式
xrandr --newmode "1920x1080_60.00" 173.00 1920 2048 2248 2576 1080 1083 1088 1120 -hsync +vsync

# 将模式添加到输出
xrandr --addmode HDMI-1 "1920x1080_60.00"

# 使用新模式
xrandr --output HDMI-1 --mode "1920x1080_60.00"
```

### 实用脚本示例

#### 双显示器工作设置
```bash
#!/bin/bash
# dual-monitor-setup.sh

# 笔记本屏幕为主显示器
xrandr --output eDP-1 --primary --mode 1920x1080 --pos 0x0 --rotate normal

# 外接显示器在右侧
xrandr --output HDMI-1 --mode 2560x1440 --pos 1920x0 --rotate normal --rate 75
```

#### 仅外接显示器
```bash
#!/bin/bash
# external-only.sh

# 关闭笔记本屏幕,仅使用外接显示器
xrandr --output eDP-1 --off
xrandr --output HDMI-1 --primary --auto
```

---

## xmodmap

### 简介
`xmodmap` 用于修改 X Window System 的键盘映射和鼠标按键映射,可以重新定义键盘按键功能。

### 基本语法
```bash
xmodmap [选项] [文件名]
```

### 主要功能

#### 1. 查看当前映射
```bash
# 显示当前键盘映射表
xmodmap -pke

# 显示修饰键映射
xmodmap -pm

# 显示鼠标按键映射
xmodmap -pp
```

#### 2. 键盘映射修改

##### 交换按键
```bash
# 交换 Caps Lock 和 Ctrl
xmodmap -e "remove Lock = Caps_Lock"
xmodmap -e "remove Control = Control_L"
xmodmap -e "keysym Caps_Lock = Control_L"
xmodmap -e "keysym Control_L = Caps_Lock"
xmodmap -e "add Lock = Caps_Lock"
xmodmap -e "add Control = Control_L"
```

##### Caps Lock 改为 Ctrl
```bash
# 将 Caps Lock 映射为 Ctrl
xmodmap -e "clear Lock"
xmodmap -e "keysym Caps_Lock = Control_L"
xmodmap -e "add Control = Control_L"
```

##### Caps Lock 改为 Escape
```bash
# 将 Caps Lock 映射为 Escape(Vim 用户最爱)
xmodmap -e "clear Lock"
xmodmap -e "keysym Caps_Lock = Escape"
```

#### 3. 修饰键说明
- **Shift**: Shift 键
- **Lock**: Caps Lock 键
- **Control**: Ctrl 键
- **Mod1**: Alt 键
- **Mod2**: Num Lock
- **Mod3**: (很少使用)
- **Mod4**: Super/Windows 键
- **Mod5**: (通常未使用)

#### 4. 使用配置文件
创建 `~/.Xmodmap` 文件:
```bash
# ~/.Xmodmap 示例

! 将 Caps Lock 改为 Ctrl
clear Lock
keysym Caps_Lock = Control_L
add Control = Control_L

! 交换左右 Alt 键
keysym Alt_L = Alt_R
keysym Alt_R = Alt_L

! 禁用 Windows 键
keysym Super_L = NoSymbol
```

加载配置:
```bash
xmodmap ~/.Xmodmap
```

#### 5. 鼠标按键映射
```bash
# 交换鼠标左右键
xmodmap -e "pointer = 3 2 1"

# 恢复默认鼠标按键
xmodmap -e "pointer = 1 2 3"
```

#### 6. 查找键码
```bash
# 使用 xev 查找按键的键码和键符
xev | grep keycode
```

### 实用配置示例

#### 程序员配置
```bash
# ~/.Xmodmap - 程序员优化配置

! Caps Lock 改为 Escape(Vim 用户)
clear Lock
keysym Caps_Lock = Escape

! 右 Alt 改为 Compose 键(方便输入特殊字符)
keysym Alt_R = Multi_key
```

#### Emacs 用户配置
```bash
# ~/.Xmodmap - Emacs 用户配置

! Caps Lock 改为 Control
clear Lock
keysym Caps_Lock = Control_L
add Control = Control_L

! Alt 和 Super 键交换(方便 Meta 键使用)
remove Mod1 = Alt_L
remove Mod4 = Super_L
keysym Alt_L = Super_L
keysym Super_L = Alt_L
add Mod1 = Alt_L
add Mod4 = Super_L
```

---

## setxkbmap

### 简介
`setxkbmap` 用于设置 X Window System 的键盘布局、变体和选项,比 xmodmap 更高级和标准化。

### 基本语法
```bash
setxkbmap [选项]
```

### 主要功能

#### 1. 查看当前布局
```bash
# 显示当前键盘布局
setxkbmap -query

# 列出所有可用布局
localectl list-x11-keymap-layouts

# 列出特定布局的变体
localectl list-x11-keymap-variants us
```

#### 2. 设置键盘布局
```bash
# 设置美国英语布局
setxkbmap us

# 设置简体中文布局
setxkbmap cn

# 设置日语布局
setxkbmap jp

# 设置德语布局
setxkbmap de
```

#### 3. 设置布局变体
```bash
# 美国英语 - Dvorak 布局
setxkbmap us -variant dvorak

# 美国英语 - Colemak 布局
setxkbmap us -variant colemak

# 俄语 - 拼音变体
setxkbmap ru -variant phonetic
```

#### 4. 多布局切换
```bash
# 设置美式英语和俄语,使用 Alt+Shift 切换
setxkbmap -layout us,ru -option grp:alt_shift_toggle

# 设置美式英语和中文,使用 Ctrl+Shift 切换
setxkbmap -layout us,cn -option grp:ctrl_shift_toggle

# 使用 CapsLock 切换布局
setxkbmap -layout us,ru -option grp:caps_toggle
```

#### 5. 常用选项(Options)

##### Caps Lock 相关
```bash
# Caps Lock 作为额外的 Ctrl
setxkbmap -option ctrl:nocaps

# Caps Lock 作为额外的 Escape
setxkbmap -option caps:escape

# 交换 Caps Lock 和 Escape
setxkbmap -option caps:swapescape

# 交换 Caps Lock 和左 Ctrl
setxkbmap -option ctrl:swapcaps
```

##### Ctrl 键相关
```bash
# 左 Alt 作为 Ctrl
setxkbmap -option ctrl:lalt_as_ctrl

# 交换左 Ctrl 和左 Alt
setxkbmap -option ctrl:swap_lalt_lctl
```

##### Alt 和 Super 键
```bash
# 左 Alt 和 左 Super 交换
setxkbmap -option altwin:swap_lalt_lwin

# 禁用 Windows 键
setxkbmap -option altwin:ctrl_win
```

##### Compose 键
```bash
# 右 Alt 作为 Compose 键
setxkbmap -option compose:ralt

# 右 Ctrl 作为 Compose 键
setxkbmap -option compose:rctrl

# Menu 键作为 Compose 键
setxkbmap -option compose:menu
```

#### 6. 清除选项
```bash
# 清除所有选项
setxkbmap -option

# 清除特定选项组
setxkbmap -option ""
```

#### 7. 组合使用
```bash
# 美国布局 + Caps Lock 作为 Ctrl + 右 Alt 作为 Compose
setxkbmap us -option ctrl:nocaps,compose:ralt

# 双布局 + 布局切换 + Caps Lock 优化
setxkbmap -layout us,ru -option grp:alt_shift_toggle,ctrl:nocaps
```

### 持久化配置

#### 方法1: 在 ~/.xinitrc 中添加
```bash
# ~/.xinitrc
setxkbmap us -option ctrl:nocaps,compose:ralt
```

#### 方法2: 使用 X 配置文件
创建 `/etc/X11/xorg.conf.d/00-keyboard.conf`:
```conf
Section "InputClass"
    Identifier "system-keyboard"
    MatchIsKeyboard "on"
    Option "XkbLayout" "us"
    Option "XkbOptions" "ctrl:nocaps,compose:ralt"
EndSection
```

---

## xrdb

### 简介
`xrdb` (X Resource Database) 用于管理 X 应用程序的资源数据库,配置颜色、字体、窗口大小等。

### 基本语法
```bash
xrdb [选项] [文件名]
```

### 主要功能

#### 1. 加载资源文件
```bash
# 加载 .Xresources 文件
xrdb -load ~/.Xresources

# 合并资源文件(不覆盖现有设置)
xrdb -merge ~/.Xresources

# 从标准输入读取
echo "XTerm*background: black" | xrdb -merge
```

#### 2. 查看当前资源
```bash
# 显示当前所有资源
xrdb -query

# 导出到文件
xrdb -query > ~/.Xresources.backup
```

#### 3. 删除资源
```bash
# 删除所有资源
xrdb -remove

# 删除特定资源(需要先编辑后重新加载)
```

#### 4. 预处理选项
```bash
# 使用 C 预处理器定义
xrdb -DCOLOR=blue -merge ~/.Xresources

# 指定显示器
xrdb -display :0 -merge ~/.Xresources
```

### Xresources 配置示例

#### 基本配置文件 `~/.Xresources`
```conf
! ~/.Xresources

! ============================================
! XTerm 配置
! ============================================

! 颜色设置
XTerm*background: #1c1c1c
XTerm*foreground: #d0d0d0
XTerm*cursorColor: #d0d0d0

! 字体设置
XTerm*faceName: DejaVu Sans Mono
XTerm*faceSize: 11

! 滚动设置
XTerm*saveLines: 10000
XTerm*scrollBar: false

! 选择行为
XTerm*selectToClipboard: true

! UTF-8 支持
XTerm*utf8: 1
XTerm*locale: true

! ============================================
! URxvt (rxvt-unicode) 配置
! ============================================

URxvt*background: #1c1c1c
URxvt*foreground: #d0d0d0
URxvt*font: xft:DejaVu Sans Mono:size=11
URxvt*scrollBar: false
URxvt*saveLines: 10000

! Perl 扩展
URxvt*perl-ext-common: default,clipboard,url-select

! ============================================
! Xcursor 配置
! ============================================

Xcursor.theme: Adwaita
Xcursor.size: 24

! ============================================
! Xft 字体渲染
! ============================================

Xft.dpi: 96
Xft.antialias: true
Xft.rgba: rgb
Xft.hinting: true
Xft.hintstyle: hintslight
Xft.lcdfilter: lcddefault
```

#### 颜色主题示例(Solarized Dark)
```conf
! Solarized Dark 配色

*background: #002b36
*foreground: #839496
*fadeColor: #002b36
*cursorColor: #93a1a1
*pointerColorBackground: #586e75
*pointerColorForeground: #93a1a1

! 黑色
*color0: #073642
*color8: #002b36

! 红色
*color1: #dc322f
*color9: #cb4b16

! 绿色
*color2: #859900
*color10: #586e75

! 黄色
*color3: #b58900
*color11: #657b83

! 蓝色
*color4: #268bd2
*color12: #839496

! 品红色
*color5: #d33682
*color13: #6c71c4

! 青色
*color6: #2aa198
*color14: #93a1a1

! 白色
*color7: #eee8d5
*color15: #fdf6e3
```

#### 自动加载配置
在 `~/.xinitrc` 或 `~/.xprofile` 中添加:
```bash
# 加载 X 资源
[ -f ~/.Xresources ] && xrdb -merge ~/.Xresources
```

---

## 实战示例

### 1. 完整的工作环境初始化脚本

创建 `~/.xinitrc`:
```bash
#!/bin/bash
# ~/.xinitrc - X 启动初始化脚本

# 加载 X 资源配置
[ -f ~/.Xresources ] && xrdb -merge ~/.Xresources

# 加载键盘映射
[ -f ~/.Xmodmap ] && xmodmap ~/.Xmodmap

# 设置键盘布局和选项
setxkbmap us -option ctrl:nocaps,compose:ralt

# 禁用蜂鸣器
xset -b

# 设置键盘重复速率
xset r rate 200 40

# 设置屏幕保护(10分钟)
xset s 600

# 设置电源管理
xset dpms 900 1200 1800

# 设置鼠标加速
xset m 2/1 4

# 双显示器设置(如果存在)
if xrandr | grep "HDMI-1 connected"; then
    xrandr --output eDP-1 --primary --mode 1920x1080 --pos 0x0
    xrandr --output HDMI-1 --mode 2560x1440 --pos 1920x0 --rate 75
fi

# 启动窗口管理器或桌面环境
exec i3
```

### 2. 显示器热插拔脚本

创建 `~/bin/monitor-hotplug.sh`:
```bash
#!/bin/bash
# monitor-hotplug.sh - 自动检测并配置外接显示器

INTERNAL="eDP-1"
EXTERNAL="HDMI-1"

if xrandr | grep "$EXTERNAL connected"; then
    echo "外接显示器已连接"
    # 仅使用外接显示器
    xrandr --output $INTERNAL --off
    xrandr --output $EXTERNAL --primary --auto
else
    echo "使用内置显示器"
    xrandr --output $INTERNAL --primary --auto
    xrandr --output $EXTERNAL --off
fi
```

### 3. 蓝光过滤脚本(类似 redshift)

创建 `~/bin/night-mode.sh`:
```bash
#!/bin/bash
# night-mode.sh - 夜间模式(降低蓝光)

MODE=${1:-on}

if [ "$MODE" = "on" ]; then
    # 降低亮度和色温
    xrandr --output eDP-1 --brightness 0.8 --gamma 1.0:0.9:0.8
    echo "夜间模式已启用"
elif [ "$MODE" = "off" ]; then
    # 恢复正常
    xrandr --output eDP-1 --brightness 1.0 --gamma 1.0:1.0:1.0
    echo "夜间模式已关闭"
else
    echo "用法: $0 {on|off}"
    exit 1
fi
```

### 4. 演示模式脚本

创建 `~/bin/presentation-mode.sh`:
```bash
#!/bin/bash
# presentation-mode.sh - 演示模式(禁用屏保和电源管理)

MODE=${1:-on}

if [ "$MODE" = "on" ]; then
    # 禁用屏幕保护和电源管理
    xset s off -dpms

    # 如果有外接显示器,镜像显示
    if xrandr | grep "HDMI-1 connected"; then
        xrandr --output eDP-1 --auto
        xrandr --output HDMI-1 --same-as eDP-1 --auto
    fi

    echo "演示模式已启用"
elif [ "$MODE" = "off" ]; then
    # 恢复屏幕保护和电源管理
    xset s 600
    xset dpms 900 1200 1800

    # 恢复扩展显示
    if xrandr | grep "HDMI-1 connected"; then
        xrandr --output eDP-1 --primary --auto --pos 0x0
        xrandr --output HDMI-1 --auto --right-of eDP-1
    fi

    echo "演示模式已关闭"
else
    echo "用法: $0 {on|off}"
    exit 1
fi

chmod +x ~/bin/presentation-mode.sh
```

### 5. 工作/家庭显示配置切换

创建 `~/bin/display-profile.sh`:
```bash
#!/bin/bash
# display-profile.sh - 快速切换显示配置

PROFILE=${1:-work}
INTERNAL="eDP-1"
EXTERNAL="HDMI-1"

case $PROFILE in
    work)
        # 工作配置:双显示器扩展
        if xrandr | grep "$EXTERNAL connected"; then
            xrandr --output $INTERNAL --primary --mode 1920x1080 --pos 0x0
            xrandr --output $EXTERNAL --mode 2560x1440 --pos 1920x0 --rate 75
            echo "工作配置已应用"
        else
            echo "未检测到外接显示器"
            xrandr --output $INTERNAL --primary --auto
        fi
        ;;

    home)
        # 家庭配置:仅外接大屏
        if xrandr | grep "$EXTERNAL connected"; then
            xrandr --output $INTERNAL --off
            xrandr --output $EXTERNAL --primary --mode 3840x2160 --rate 60
            echo "家庭配置已应用"
        else
            echo "未检测到外接显示器"
            xrandr --output $INTERNAL --primary --auto
        fi
        ;;

    laptop)
        # 笔记本模式:仅内置屏幕
        xrandr --output $INTERNAL --primary --auto
        xrandr --output $EXTERNAL --off
        echo "笔记本模式已应用"
        ;;

    mirror)
        # 镜像模式
        if xrandr | grep "$EXTERNAL connected"; then
            xrandr --output $INTERNAL --auto
            xrandr --output $EXTERNAL --same-as $INTERNAL --auto
            echo "镜像模式已应用"
        else
            echo "未检测到外接显示器"
        fi
        ;;

    *)
        echo "用法: $0 {work|home|laptop|mirror}"
        echo "  work   - 双显示器扩展(办公)"
        echo "  home   - 仅外接4K显示器"
        echo "  laptop - 仅笔记本屏幕"
        echo "  mirror - 镜像显示"
        exit 1
        ;;
esac

chmod +x ~/bin/display-profile.sh
```

### 6. 键盘布局指示器

创建 `~/bin/keyboard-layout.sh`:
```bash
#!/bin/bash
# keyboard-layout.sh - 显示当前键盘布局

current_layout=$(setxkbmap -query | grep layout | awk '{print $2}')
echo "当前键盘布局: $current_layout"

# 可以添加到状态栏
case $current_layout in
    us)
        echo "EN"
        ;;
    ru)
        echo "RU"
        ;;
    cn)
        echo "中文"
        ;;
    *)
        echo "$current_layout"
        ;;
esac
```

---

## 故障排查

### 1. xset 设置不生效
```bash
# 检查 X 服务器是否支持该功能
xset q | grep -i dpms
xset q | grep -i "Screen Saver"

# 某些桌面环境会覆盖 xset 设置
# 需要在桌面环境设置中禁用相应功能
```

### 2. xrandr 设置在重启后丢失
```bash
# 方法1: 添加到 ~/.xinitrc
echo 'xrandr --output HDMI-1 --auto --right-of eDP-1' >> ~/.xinitrc

# 方法2: 创建 udev 规则(高级)
# /etc/udev/rules.d/95-monitor-hotplug.rules
```

### 3. xmodmap 与 setxkbmap 冲突
```bash
# setxkbmap 会重置 xmodmap 的修改
# 解决方案:优先使用 setxkbmap 的 -option
# 或者在 setxkbmap 之后再运行 xmodmap

setxkbmap us
xmodmap ~/.Xmodmap
```

### 4. 查看 X 错误日志
```bash
# 查看 Xorg 日志
cat ~/.local/share/xorg/Xorg.0.log

# 或者
cat /var/log/Xorg.0.log
```

---

## 相关工具

### 1. xev - 事件查看器
```bash
# 查看键盘和鼠标事件
xev

# 仅查看键盘事件
xev | grep keycode
```

### 2. xprop - 窗口属性查看器
```bash
# 点击窗口查看属性
xprop

# 查看根窗口属性
xprop -root
```

### 3. xdpyinfo - 显示信息查看器
```bash
# 查看 X 服务器详细信息
xdpyinfo

# 查看屏幕信息
xdpyinfo | grep dimensions
```

### 4. xinput - 输入设备管理
```bash
# 列出所有输入设备
xinput list

# 查看设备属性
xinput list-props <device-id>

# 设置设备属性
xinput set-prop <device-id> <property> <value>
```

---

## 总结

### 命令对照表

| 命令 | 主要用途 | 常用场景 |
|------|----------|----------|
| **xset** | X 服务器参数设置 | 电源管理、屏保、键盘鼠标速率 |
| **xrandr** | 显示器配置 | 分辨率、刷新率、多显示器布局 |
| **xmodmap** | 键盘映射修改 | 交换按键、自定义键位 |
| **setxkbmap** | 键盘布局设置 | 语言布局、Caps Lock 重映射 |
| **xrdb** | X 资源数据库 | 应用程序颜色、字体配置 |

### 快速参考

```bash
# 查看命令帮助
man xset
man xrandr
man xmodmap
man setxkbmap
man xrdb

# 查询当前设置
xset q                      # 查看所有 xset 设置
xrandr                      # 查看显示器配置
xmodmap -pke                # 查看键盘映射
setxkbmap -query            # 查看键盘布局
xrdb -query                 # 查看 X 资源
```

### 最佳实践

1. **持久化配置**: 将常用设置写入 `~/.xinitrc` 或 `~/.xprofile`
2. **使用脚本**: 为复杂配置创建 shell 脚本
3. **备份配置**: 定期备份 `.Xresources`, `.Xmodmap` 等配置文件
4. **优先级**: `setxkbmap` > `xmodmap`(键盘配置)
5. **测试**: 在终端测试命令生效后再写入配置文件

---

## 参考资源

- [ArchWiki - Xorg](https://wiki.archlinux.org/title/Xorg)
- [ArchWiki - xrandr](https://wiki.archlinux.org/title/Xrandr)
- [ArchWiki - Keyboard configuration](https://wiki.archlinux.org/title/Xorg/Keyboard_configuration)
- [X.Org Documentation](https://www.x.org/wiki/Documentation/)
