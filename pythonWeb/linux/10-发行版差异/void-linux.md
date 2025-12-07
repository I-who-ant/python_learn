# Void Linux - 独立发行版详解

> 一个独立的、通用的、滚动发布的 Linux 发行版,以 runit 和 XBPS 为特色

## 目录

- [Void Linux 简介](#void-linux-简介)
- [核心特性](#核心特性)
- [包管理器 XBPS](#包管理器-xbps)
- [服务管理 runit](#服务管理-runit)
- [系统配置](#系统配置)
- [与其他发行版对比](#与其他发行版对比)
- [常见任务](#常见任务)
- [系统工具和调试命令](#系统工具和调试命令)
- [优缺点分析](#优缺点分析)

---

## Void Linux 简介

**Void Linux** 是一个独立开发的 Linux 发行版,不基于任何其他发行版(如 Debian、Arch 等)。它由 Juan Romero Pardines 于 2008 年创建。

### 核心特点

- **独立发行版**: 不基于任何其他发行版,完全从头构建
- **滚动发布**: 持续更新,无需重装系统
- **runit 初始化**: 使用 runit 替代 systemd,更轻量快速
- **XBPS 包管理**: 自研的 X Binary Package System
- **支持多种 libc**: 提供 glibc 和 musl 两种版本
- **轻量快速**: 启动速度快,资源占用低
- **混合包管理**: 同时支持二进制包和源码构建

### 两个版本

| 版本 | C库 | 特点 | 适用场景 |
|------|-----|------|----------|
| **glibc** | GNU C Library | 兼容性好,软件丰富 | 桌面系统、通用用途 |
| **musl** | musl libc | 轻量、简洁、静态链接友好 | 嵌入式、容器、极简系统 |

---

## 核心特性

### 1. runit - 初始化系统

Void Linux 使用 **runit** 替代 systemd,这是其最显著的特点之一。

**runit 的优势**:
- 极其简单和轻量(约 10KB)
- 启动速度快
- 进程监督机制可靠
- 配置简单直观
- 无复杂依赖

**runit vs systemd**:
| 特性 | runit | systemd |
|------|-------|---------|
| 大小 | ~10KB | ~10MB |
| 启动速度 | 非常快 | 较快 |
| 复杂度 | 极简 | 复杂 |
| 功能范围 | 专注初始化 | 系统管理套件 |
| 依赖 | 极少 | 较多 |

### 2. XBPS - 包管理系统

**XBPS** (X Binary Package System) 是 Void 自主开发的包管理器,结合了二进制包的便捷和源码构建的灵活性。

**XBPS 特点**:
- 速度快(用C语言编写)
- 支持事务性操作(原子性更新)
- 清晰的依赖解析
- 支持二进制包和源码构建
- 包格式简单(.xbps)

### 3. 滚动发布模式

- 持续更新,无版本号
- 不需要大版本升级
- 始终运行最新软件
- 稳定性与最新性平衡良好

---

## 包管理器 XBPS

### 基本命令

#### 更新和安装

```bash
# 同步远程仓库索引
sudo xbps-install -S

# 更新系统(等同于 pacman -Syu)
sudo xbps-install -Su

# 安装软件包
sudo xbps-install <package>
sudo xbps-install -S <package>  # 同步后安装

# 安装多个包
sudo xbps-install vim git curl

# 强制重新安装
sudo xbps-install -f <package>
```

#### 删除软件

```bash
# 删除软件包
sudo xbps-remove <package>

# 删除软件包及其依赖(推荐)
sudo xbps-remove -R <package>

# 删除孤立包(不再被需要的依赖)
sudo xbps-remove -o

# 删除缓存的包
sudo xbps-remove -O
```

#### 搜索和查询

```bash
# 搜索软件包
xbps-query -Rs <keyword>

# 显示包信息
xbps-query -R <package>

# 查看已安装的包
xbps-query -l

# 查看包的文件列表
xbps-query -f <package>

# 查找文件属于哪个包
xbps-query -o /path/to/file

# 查看包的依赖
xbps-query -x <package>

# 查看哪些包依赖某个包
xbps-query -X <package>
```

#### 系统维护

```bash
# 清理包缓存
sudo xbps-remove -O

# 清理孤立依赖
sudo xbps-remove -o

# 验证包完整性
sudo xbps-pkgdb -a

# 重新配置包
sudo xbps-reconfigure -f <package>

# 重新配置所有包
sudo xbps-reconfigure -a
```

### XBPS 工具集

| 命令 | 功能 |
|------|------|
| `xbps-install` | 安装、更新包 |
| `xbps-remove` | 删除包 |
| `xbps-query` | 查询包信息 |
| `xbps-pkgdb` | 包数据库管理 |
| `xbps-reconfigure` | 重新配置包 |
| `xbps-alternatives` | 管理替代品(类似 update-alternatives) |
| `xbps-checkvers` | 检查包版本 |
| `xbps-create` | 创建包 |
| `xbps-dgraph` | 显示依赖图 |
| `xbps-fbulk` | 批量构建 |
| `xbps-src` | 源码构建系统 |

### 命令对比表

| 操作 | Void (XBPS) | Arch (pacman) | Debian (apt) |
|------|-------------|---------------|--------------|
| **同步仓库** | `xbps-install -S` | `pacman -Sy` | `apt update` |
| **更新系统** | `xbps-install -Su` | `pacman -Syu` | `apt upgrade` |
| **安装包** | `xbps-install pkg` | `pacman -S pkg` | `apt install pkg` |
| **删除包** | `xbps-remove pkg` | `pacman -R pkg` | `apt remove pkg` |
| **删除+依赖** | `xbps-remove -R pkg` | `pacman -Rs pkg` | `apt autoremove pkg` |
| **搜索包** | `xbps-query -Rs key` | `pacman -Ss key` | `apt search key` |
| **包信息** | `xbps-query -R pkg` | `pacman -Si pkg` | `apt show pkg` |
| **已安装列表** | `xbps-query -l` | `pacman -Q` | `apt list --installed` |
| **清理缓存** | `xbps-remove -O` | `pacman -Sc` | `apt autoclean` |
| **文件归属** | `xbps-query -o file` | `pacman -Qo file` | `dpkg -S file` |

### 仓库配置

```bash
# 主仓库配置
/usr/share/xbps.d/

# 本地仓库配置
/etc/xbps.d/

# 仓库文件示例
# /etc/xbps.d/00-repository-main.conf
repository=https://repo-default.voidlinux.org/current
repository=https://repo-default.voidlinux.org/current/musl  # musl版本

# 镜像站点
repository=https://mirrors.tuna.tsinghua.edu.cn/voidlinux/current
repository=https://mirror.sjtu.edu.cn/voidlinux/current
```

### xbps-src - 源码构建

Void 提供了强大的源码构建系统,类似于 Arch 的 ABS:

```bash
# 克隆 void-packages 仓库
git clone https://github.com/void-linux/void-packages.git
cd void-packages

# 初始化构建环境
./xbps-src binary-bootstrap

# 构建包
./xbps-src pkg <package-name>

# 安装构建的包
sudo xbps-install -R hostdir/binpkgs <package-name>
```

---

## 服务管理 runit

### runit 基础概念

**runit** 采用三段式设计:
1. **阶段 1** (`/etc/runit/1`): 系统初始化
2. **阶段 2** (`/etc/runit/2`): 启动服务,进入多用户模式
3. **阶段 3** (`/etc/runit/3`): 系统关闭

服务脚本位置: `/etc/sv/`

### 服务管理命令

runit 使用 `sv` 命令管理服务:

```bash
# 启动服务
sudo sv start <service>

# 停止服务
sudo sv stop <service>

# 重启服务
sudo sv restart <service>

# 查看服务状态
sudo sv status <service>

# 向服务发送信号
sudo sv reload <service>    # 重载配置
sudo sv hup <service>        # 发送 SIGHUP

# 检查服务是否运行
sv check <service>
```

### 启用/禁用服务

服务通过符号链接到 `/var/service/` 来启用:

```bash
# 启用服务(开机自启)
sudo ln -s /etc/sv/<service> /var/service/

# 禁用服务
sudo rm /var/service/<service>

# 查看所有启用的服务
ls -l /var/service/

# 查看所有可用服务
ls -l /etc/sv/
```

### 常用服务示例

```bash
# SSH 服务
sudo sv status sshd
sudo ln -s /etc/sv/sshd /var/service/

# 网络管理
sudo sv status dhcpcd
sudo sv status NetworkManager

# 时间同步
sudo sv status chronyd
sudo sv status ntpd

# Web服务器
sudo ln -s /etc/sv/nginx /var/service/
sudo sv restart nginx
```

### 创建自定义服务

创建一个简单的服务:

```bash
# 1. 创建服务目录
sudo mkdir -p /etc/sv/myservice

# 2. 创建 run 脚本
sudo vim /etc/sv/myservice/run
```

```bash
#!/bin/sh
# /etc/sv/myservice/run

exec 2>&1
exec chpst -u myuser /path/to/myprogram
```

```bash
# 3. 设置执行权限
sudo chmod +x /etc/sv/myservice/run

# 4. 启用服务
sudo ln -s /etc/sv/myservice /var/service/

# 5. 查看状态
sudo sv status myservice
```

### 服务日志

runit 使用 **svlogd** 管理日志:

```bash
# 创建日志目录
sudo mkdir -p /etc/sv/myservice/log

# 创建日志 run 脚本
sudo vim /etc/sv/myservice/log/run
```

```bash
#!/bin/sh
exec svlogd -tt /var/log/myservice
```

```bash
# 设置权限
sudo chmod +x /etc/sv/myservice/log/run

# 查看日志
tail -f /var/log/myservice/current
```

### runit vs systemd 命令对比

| 操作 | runit | systemd |
|------|-------|---------|
| **启动服务** | `sv start sshd` | `systemctl start sshd` |
| **停止服务** | `sv stop sshd` | `systemctl stop sshd` |
| **重启服务** | `sv restart sshd` | `systemctl restart sshd` |
| **查看状态** | `sv status sshd` | `systemctl status sshd` |
| **启用开机自启** | `ln -s /etc/sv/sshd /var/service/` | `systemctl enable sshd` |
| **禁用开机自启** | `rm /var/service/sshd` | `systemctl disable sshd` |
| **查看所有服务** | `ls /var/service/` | `systemctl list-units` |
| **查看日志** | `cat /var/log/<service>/current` | `journalctl -u <service>` |

---

## 系统配置

### 网络配置

#### 使用 dhcpcd (DHCP)

```bash
# 启用 dhcpcd
sudo ln -s /etc/sv/dhcpcd /var/service/
sudo sv start dhcpcd

# 查看状态
ip addr show
```

#### 静态IP配置

编辑 `/etc/rc.local`:

```bash
# /etc/rc.local
ip addr add 192.168.1.100/24 dev eth0
ip route add default via 192.168.1.1
```

或使用 dhcpcd 的静态配置:

```bash
# /etc/dhcpcd.conf
interface eth0
static ip_address=192.168.1.100/24
static routers=192.168.1.1
static domain_name_servers=8.8.8.8 8.8.4.4
```

#### 使用 NetworkManager

```bash
# 安装
sudo xbps-install -S NetworkManager

# 启用服务
sudo ln -s /etc/sv/NetworkManager /var/service/

# 使用 nmcli
nmcli device wifi connect SSID password PASSWORD
```

#### 无线网络

```bash
# 使用 wpa_supplicant
sudo xbps-install -S wpa_supplicant

# 配置
sudo wpa_passphrase "SSID" "password" > /etc/wpa_supplicant/wpa_supplicant.conf

# 启动
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo dhcpcd wlan0
```

### DNS 配置

```bash
# /etc/resolv.conf
nameserver 8.8.8.8
nameserver 8.8.4.4
nameserver 1.1.1.1
```

### 主机名设置

```bash
# 设置主机名
echo "myhostname" | sudo tee /etc/hostname

# 编辑 hosts
sudo vim /etc/hosts
```

```
127.0.0.1   localhost
127.0.1.1   myhostname.localdomain myhostname
::1         localhost ip6-localhost ip6-loopback
```

### 时区设置

```bash
# 查看当前时区
ls -l /etc/localtime

# 设置时区
sudo ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime

# 设置硬件时钟
sudo hwclock --systohc
```

### locale 设置

```bash
# 编辑 locale
sudo vim /etc/default/libc-locales
```

取消注释需要的 locale,如:
```
en_US.UTF-8 UTF-8
zh_CN.UTF-8 UTF-8
```

```bash
# 生成 locale
sudo xbps-reconfigure -f glibc-locales

# 设置系统 locale
echo "LANG=en_US.UTF-8" | sudo tee /etc/locale.conf

# 或者编辑
sudo vim /etc/locale.conf
```

```bash
LANG=en_US.UTF-8
LC_TIME=en_US.UTF-8
```

### 用户管理

```bash
# 创建用户
sudo useradd -m -G wheel,audio,video,storage,optical -s /bin/bash username

# 设置密码
sudo passwd username

# 添加到 sudo 组
sudo usermod -aG wheel username

# 配置 sudo
sudo visudo
```

取消注释:
```
%wheel ALL=(ALL) ALL
```

---

## 与其他发行版对比

### 包管理对比

| 特性 | Void (XBPS) | Arch (pacman) | Debian (apt) | Alpine (apk) |
|------|-------------|---------------|--------------|--------------|
| **包格式** | .xbps | .pkg.tar.zst | .deb | .apk |
| **更新命令** | `xbps-install -Su` | `pacman -Syu` | `apt upgrade` | `apk upgrade` |
| **速度** | 非常快 | 快 | 中等 | 非常快 |
| **依赖解析** | 事务性 | 事务性 | 较好 | 简单 |
| **源码构建** | xbps-src | ABS/AUR | 源码包 | APKBUILD |

### 初始化系统对比

| 发行版 | Init系统 | 启动速度 | 复杂度 | 内存占用 |
|--------|----------|----------|--------|----------|
| **Void** | runit | 非常快 | 极简 | 极低 |
| **Arch** | systemd | 快 | 高 | 中等 |
| **Debian** | systemd | 快 | 高 | 中等 |
| **Alpine** | OpenRC | 非常快 | 低 | 低 |
| **Gentoo** | OpenRC/systemd | 快 | 中等 | 低-中等 |

### 哲学和定位

| 发行版 | 哲学 | 更新模式 | 目标用户 |
|--------|------|----------|----------|
| **Void** | 简洁、独立、runit | 滚动 | 进阶用户、极简主义者 |
| **Arch** | KISS、用户自主 | 滚动 | 进阶用户、定制爱好者 |
| **Debian** | 稳定、通用 | 固定发布 | 所有用户、服务器 |
| **Alpine** | 安全、轻量 | 滚动 | 容器、嵌入式 |
| **Gentoo** | 高度定制、源码编译 | 滚动 | 高级用户、性能追求者 |

---

## 常见任务

### 系统安装后配置

```bash
# 1. 更新系统
sudo xbps-install -Su

# 2. 安装基础工具
sudo xbps-install -S vim git curl wget base-devel

# 3. 安装和配置 sudo
sudo xbps-install -S sudo
sudo visudo  # 启用 wheel 组

# 4. 配置网络
sudo ln -s /etc/sv/dhcpcd /var/service/
# 或
sudo xbps-install -S NetworkManager
sudo ln -s /etc/sv/NetworkManager /var/service/

# 5. 配置时间同步
sudo xbps-install -S chrony
sudo ln -s /etc/sv/chronyd /var/service/
```

### 桌面环境安装

#### XFCE 桌面

```bash
# 安装 Xorg
sudo xbps-install -S xorg

# 安装 XFCE
sudo xbps-install -S xfce4 xfce4-plugins

# 安装显示管理器
sudo xbps-install -S lightdm lightdm-gtk3-greeter

# 启用服务
sudo ln -s /etc/sv/dbus /var/service/
sudo ln -s /etc/sv/lightdm /var/service/
```

#### i3 窗口管理器

```bash
# 安装
sudo xbps-install -S xorg i3 i3status dmenu rxvt-unicode

# 创建 .xinitrc
echo "exec i3" > ~/.xinitrc

# 启动
startx
```

### 开发环境配置

#### Python

```bash
# 安装 Python
sudo xbps-install -S python3 python3-pip python3-devel

# 虚拟环境
sudo xbps-install -S python3-virtualenv
```

#### Node.js

```bash
# 安装 Node.js
sudo xbps-install -S nodejs

# 全局包目录(避免 sudo)
mkdir -p ~/.npm-global
npm config set prefix '~/.npm-global'
echo 'export PATH=~/.npm-global/bin:$PATH' >> ~/.bashrc
```

#### Docker

```bash
# 安装 Docker
sudo xbps-install -S docker

# 启用服务
sudo ln -s /etc/sv/docker /var/service/

# 添加用户到 docker 组
sudo usermod -aG docker $USER
```

### 内核管理

```bash
# 查看当前内核
uname -r

# 安装内核(通常已安装)
sudo xbps-install -S linux

# 安装 LTS 内核
sudo xbps-install -S linux-lts

# 更新内核
sudo xbps-install -Su
sudo reboot

# 删除旧内核
sudo vkpurge list
sudo vkpurge rm <kernel-version>
```

### 图形驱动

```bash
# Intel
sudo xbps-install -S mesa-dri vulkan-loader mesa-vulkan-intel intel-video-accel

# NVIDIA (专有驱动)
sudo xbps-install -S nvidia

# NVIDIA (开源驱动)
sudo xbps-install -S xf86-video-nouveau

# AMD
sudo xbps-install -S mesa-dri vulkan-loader mesa-vulkan-radeon
```

### 音频配置

```bash
# PulseAudio
sudo xbps-install -S pulseaudio alsa-plugins-pulseaudio

# PipeWire (现代选择)
sudo xbps-install -S pipewire alsa-pipewire

# 添加用户到 audio 组
sudo usermod -aG audio $USER
```

---

## 系统工具和调试命令

### 库依赖分析工具

#### ldd - 显示共享库依赖

```bash
# 查看程序依赖的共享库
ldd /bin/bash
ldd /usr/bin/python3

# 查看库的位置
ldd /usr/bin/vim

# 检查缺失的库
ldd /path/to/program | grep "not found"

# 查看 32 位程序的依赖（如果支持）
ldd /path/to/32bit/program
```

输出示例：
```
linux-vdso.so.1 (0x00007ffd8a9e1000)
libtinfo.so.6 => /usr/lib/libtinfo.so.6 (0x00007f8e3c5a0000)
libdl.so.2 => /usr/lib/libdl.so.2 (0x00007f8e3c59b000)
libc.so.6 => /usr/lib/libc.so.6 (0x00007f8e3c3d0000)
```

#### ldconfig - 共享库缓存管理

```bash
# 更新共享库缓存
sudo ldconfig

# 显示所有缓存的库
ldconfig -p

# 搜索特定库
ldconfig -p | grep libssl

# 查看配置文件
cat /etc/ld.so.conf
ls /etc/ld.so.conf.d/

# 查看缓存内容
cat /etc/ld.so.cache  # 二进制文件
```

#### lddtree - 树状显示依赖（需要安装）

```bash
# 安装 pax-utils
sudo xbps-install -S pax-utils

# 树状显示依赖
lddtree /usr/bin/vim

# 只显示缺失的库
lddtree --missing /path/to/program
```

### XBPS 高级依赖管理

#### 包依赖查询

```bash
# 查看包的运行时依赖
xbps-query -x package

# 查看哪些包依赖某个包（反向依赖）
xbps-query -X package

# 递归显示所有依赖
xbps-query -x package --fulldeptree

# 查看包提供的文件
xbps-query -f package

# 查看文件属于哪个包
xbps-query -o /usr/bin/vim
xbps-query -o /lib/libc.so.6

# 查看包的详细信息
xbps-query -R package
```

#### 依赖图可视化

```bash
# 生成依赖图（需要 graphviz）
sudo xbps-install -S graphviz

# 生成包依赖图
xbps-query -x package | xbps-dgraph -R | dot -Tpng -o deps.png

# 查看反向依赖图
xbps-query -X package | xbps-dgraph -R | dot -Tpng -o reverse-deps.png
```

#### 孤立包管理

```bash
# 列出孤立包（不被任何包依赖）
xbps-query -O

# 删除所有孤立包
sudo xbps-remove -o

# 查看将被删除的包（测试模式）
sudo xbps-remove -on
```

### 系统调试工具

#### strace - 系统调用跟踪

```bash
# 安装 strace
sudo xbps-install -S strace

# 跟踪程序的系统调用
strace ls

# 跟踪并统计
strace -c ls

# 跟踪特定系统调用
strace -e open,read ls
strace -e trace=file ls

# 跟踪正在运行的进程
strace -p <PID>

# 输出到文件
strace -o output.log ls

# 跟踪多线程程序
strace -f ./program

# 显示时间戳
strace -t ls
strace -tt ls  # 微秒级时间戳

# 常用调试场景
strace -e trace=open,openat vim file.txt  # 查看打开的文件
strace -e trace=network curl google.com   # 查看网络调用
```

#### ltrace - 库函数调用跟踪

```bash
# 安装 ltrace
sudo xbps-install -S ltrace

# 跟踪库函数调用
ltrace ls

# 跟踪特定函数
ltrace -e malloc,free ./program

# 跟踪系统调用和库调用
ltrace -S ./program

# 统计
ltrace -c ./program
```

#### gdb - 调试器

```bash
# 安装 gdb
sudo xbps-install -S gdb

# 调试程序
gdb ./program

# 调试运行中的进程
gdb -p <PID>

# 带参数启动
gdb --args ./program arg1 arg2

# 常用 gdb 命令
(gdb) run              # 运行程序
(gdb) break main       # 设置断点
(gdb) continue         # 继续执行
(gdb) next             # 单步执行
(gdb) print variable   # 打印变量
(gdb) backtrace        # 查看堆栈
(gdb) info registers   # 查看寄存器
(gdb) quit             # 退出

# 调试 core dump
gdb ./program core

# 启用 core dump
ulimit -c unlimited
echo "core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern
```

#### valgrind - 内存调试

```bash
# 安装 valgrind
sudo xbps-install -S valgrind

# 检测内存泄漏
valgrind --leak-check=full ./program

# 详细输出
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./program

# 检测线程问题
valgrind --tool=helgrind ./program

# 性能分析
valgrind --tool=callgrind ./program
```

### 进程和性能分析

#### 进程管理工具

```bash
# 安装进程管理工具
sudo xbps-install -S htop btop procps-ng

# htop - 交互式进程查看器
htop

# btop - 现代化资源监控
btop

# ps - 进程快照
ps aux
ps -ef
ps -eLf  # 显示线程

# 查找进程
pgrep firefox
pidof firefox

# 进程树
pstree
pstree -p  # 显示 PID

# 查看进程打开的文件
lsof -p <PID>
lsof -c firefox  # 按进程名查看

# 查看特定文件被哪些进程打开
lsof /path/to/file

# 查看网络连接
lsof -i
lsof -i :80
lsof -i TCP:22
```

#### 性能分析工具

```bash
# 安装性能工具
sudo xbps-install -S sysstat iotop iftop

# iostat - I/O 统计
iostat
iostat -x 1  # 每秒更新

# vmstat - 虚拟内存统计
vmstat 1

# iotop - I/O 监控
sudo iotop

# iftop - 网络流量监控
sudo iftop

# 查看系统资源使用
free -h          # 内存使用
df -h            # 磁盘使用
du -sh *         # 目录大小
uptime           # 系统运行时间和负载

# CPU 信息
lscpu
cat /proc/cpuinfo

# 内存信息
cat /proc/meminfo

# 内核模块
lsmod
modinfo <module>
```

#### perf - 性能分析框架

```bash
# 安装 perf
sudo xbps-install -S linux-tools

# 记录性能数据
sudo perf record -a sleep 10
sudo perf record -p <PID> sleep 10

# 查看报告
sudo perf report

# CPU 使用率分析
sudo perf top

# 统计
sudo perf stat ls
sudo perf stat -p <PID> sleep 10
```

### 网络诊断工具

```bash
# 安装网络工具
sudo xbps-install -S \
    iproute2 \
    net-tools \
    bind-utils \
    traceroute \
    tcpdump \
    nmap \
    netcat \
    wget \
    curl \
    mtr

# 网络接口
ip addr show
ip link show
ip route show

# 网络连接
ss -tuln          # 监听端口
ss -tunap         # 所有连接
netstat -tuln     # 旧版命令

# DNS 查询
dig google.com
nslookup google.com
host google.com

# 路由追踪
traceroute google.com
mtr google.com    # 实时路由监控

# 网络抓包
sudo tcpdump -i eth0
sudo tcpdump -i eth0 port 80
sudo tcpdump -i eth0 -w capture.pcap

# 端口扫描
nmap localhost
nmap -p 1-1000 192.168.1.1

# 网络测试
nc -zv google.com 80
curl -I https://google.com
wget --spider https://google.com

# 测速
sudo xbps-install -S speedtest-cli
speedtest-cli
```

### 文件系统工具

```bash
# 安装文件系统工具
sudo xbps-install -S \
    e2fsprogs \
    dosfstools \
    ntfs-3g \
    btrfs-progs \
    xfsprogs

# 查看磁盘信息
lsblk
lsblk -f  # 显示文件系统
blkid     # UUID 和类型

# 分区工具
sudo fdisk -l
sudo parted -l
sudo gdisk -l

# 文件系统检查（需要卸载分区）
sudo fsck /dev/sda1
sudo e2fsck /dev/sda1

# 磁盘使用分析
ncdu /          # 交互式磁盘使用查看器（需安装）
sudo xbps-install -S ncdu

# 查找大文件
find / -type f -size +100M 2>/dev/null
du -ah / | sort -rh | head -20

# 查看文件类型
file /path/to/file

# 查看 ELF 文件信息
readelf -h /bin/bash
objdump -f /bin/bash
```

### 日志管理

```bash
# 系统日志位置
/var/log/

# 常见日志文件
/var/log/messages       # 系统消息（可能不存在）
/var/log/syslog         # 系统日志
/var/log/auth.log       # 认证日志
/var/log/kern.log       # 内核日志
/var/log/Xorg.0.log     # X Server 日志

# runit 服务日志
/var/log/<service>/current

# 查看日志
tail -f /var/log/<service>/current
less /var/log/<service>/current

# 使用 svlogd 查看日志
sudo sv status <service>
tail -f /var/log/<service>/current

# 搜索日志
grep "error" /var/log/messages
grep -r "keyword" /var/log/

# 日志清理
sudo find /var/log -type f -name "*.log" -mtime +30 -delete
```

### 系统信息工具

```bash
# 安装系统信息工具
sudo xbps-install -S \
    neofetch \
    screenfetch \
    inxi \
    hwinfo \
    dmidecode

# 系统信息概览
neofetch
screenfetch
inxi -F

# 硬件信息
sudo lshw
sudo lshw -short
sudo dmidecode

# PCI 设备
lspci
lspci -v

# USB 设备
lsusb
lsusb -v

# 内核信息
uname -a
cat /proc/version

# 发行版信息
cat /etc/os-release
xbps-query -R base-system
```

### 压缩和归档工具

```bash
# 安装压缩工具
sudo xbps-install -S \
    tar \
    gzip \
    bzip2 \
    xz \
    zip \
    unzip \
    p7zip \
    zstd

# tar 常用操作
tar -czf archive.tar.gz files/      # 创建 gzip 压缩包
tar -cjf archive.tar.bz2 files/     # 创建 bzip2 压缩包
tar -cJf archive.tar.xz files/      # 创建 xz 压缩包
tar -xzf archive.tar.gz             # 解压 gzip
tar -xjf archive.tar.bz2            # 解压 bzip2
tar -xJf archive.tar.xz             # 解压 xz
tar -tzf archive.tar.gz             # 查看内容

# zip/unzip
zip -r archive.zip files/
unzip archive.zip
unzip -l archive.zip  # 查看内容

# 7z
7z a archive.7z files/
7z x archive.7z
7z l archive.7z  # 查看内容

# xz 压缩（高压缩率）
xz file
xz -d file.xz
xz -l file.xz  # 查看信息

# zstd（快速压缩）
zstd file
zstd -d file.zst
```

### Void 特有工具

#### xtools - Void 实用工具集

```bash
# 安装 xtools
sudo xbps-install -S xtools

# 常用 xtools 命令
xlocate <file>          # 搜索文件（类似 pkgfile）
xbps-src               # 构建工具
xcheckrestart          # 检查需要重启的服务
xrevbump <pkg>         # 增加包修订号
xlint <template>       # 检查模板错误
xmandoc <pkg>          # 查看包的 man 页面
xdbg <pkg>             # 调试包信息
```

#### vkpurge - 内核清理工具

```bash
# 列出已安装的内核
vkpurge list

# 删除旧内核
sudo vkpurge rm <kernel-version>

# 删除所有旧内核（保留当前）
sudo vkpurge rm all

# 注意：不要删除正在使用的内核！
uname -r  # 查看当前内核版本
```

#### vsv - runit 服务助手

```bash
# 安装 vsv
sudo xbps-install -S vsv

# 查看所有服务状态
vsv

# 启动服务
vsv start <service>

# 停止服务
vsv stop <service>

# 重启服务
vsv restart <service>

# 查看服务状态
vsv status <service>

# 启用服务（创建符号链接）
vsv enable <service>

# 禁用服务
vsv disable <service>
```

### 实用命令组合

#### 查找占用端口的进程

```bash
# 方法 1
sudo lsof -i :8080

# 方法 2
sudo ss -tulnp | grep :8080

# 方法 3
sudo netstat -tulnp | grep :8080

# 方法 4
sudo fuser 8080/tcp
```

#### 查找大文件和清理

```bash
# 查找大于 100MB 的文件
find / -type f -size +100M -exec ls -lh {} \; 2>/dev/null

# 查找并删除旧日志
sudo find /var/log -type f -name "*.log" -mtime +30 -delete

# 清理包缓存
sudo xbps-remove -O

# 清理孤立包
sudo xbps-remove -o
```

#### 批量操作

```bash
# 批量安装包
sudo xbps-install -S $(cat packages.txt)

# 批量重启服务
for svc in nginx mysql redis; do sudo sv restart $svc; done

# 查找并替换
find . -name "*.txt" -exec sed -i 's/old/new/g' {} \;
```

---

## 优缺点分析

### 优点

1. **runit 的优势**
   - 极其轻量和快速
   - 配置简单直观
   - 可靠的进程监督
   - 无复杂依赖

2. **XBPS 包管理**
   - 速度快,效率高
   - 事务性操作安全
   - 清晰的依赖处理
   - 同时支持二进制和源码

3. **独立性**
   - 不受其他发行版政策影响
   - 干净的设计理念
   - 无历史包袱

4. **滚动发布**
   - 始终最新软件
   - 无需大版本升级
   - 更新频率合理

5. **多架构支持**
   - x86_64, i686, ARMv6, ARMv7, AArch64
   - 支持 glibc 和 musl

6. **资源占用低**
   - 内存占用少
   - 磁盘空间小
   - CPU使用率低

### 缺点

1. **社区规模小**
   - 用户基数较小
   - 中文资料少
   - 社区活跃度不如 Arch/Ubuntu

2. **软件包数量**
   - 比 Arch/Debian 少
   - 某些冷门软件可能没有
   - 需要自己打包

3. **学习曲线**
   - runit 需要学习(虽然简单)
   - XBPS 命令与其他不同
   - 缺少图形化工具

4. **文档**
   - 文档不如 Arch Wiki 丰富
   - 很多问题需要自己摸索

5. **兼容性**
   - musl 版本软件兼容性问题
   - 某些闭源软件不支持
   - 容器镜像较少

6. **企业支持**
   - 无商业支持
   - 不适合企业环境
   - 维护依赖社区

### 适用场景

**适合 Void Linux 的场景**:
- 个人桌面/工作站
- 轻量级服务器
- 老旧硬件
- 容器基础镜像(musl版本)
- 嵌入式设备
- 学习 Linux 系统原理

**不适合的场景**:
- 生产环境服务器(除非你很了解)
- 需要大量软件支持的场景
- 需要商业支持的企业环境
- Linux 新手

---

## 实用技巧

### 镜像加速

创建 `/etc/xbps.d/00-repository-main.conf`:

```bash
# 清华源
repository=https://mirrors.tuna.tsinghua.edu.cn/voidlinux/current
repository=https://mirrors.tuna.tsinghua.edu.cn/voidlinux/current/nonfree

# 上海交大源
repository=https://mirror.sjtu.edu.cn/voidlinux/current
repository=https://mirror.sjtu.edu.cn/voidlinux/current/nonfree
```

### 自动化脚本

```bash
# 系统更新脚本
#!/bin/bash
echo "==> 更新系统..."
sudo xbps-install -Su

echo "==> 清理孤立包..."
sudo xbps-remove -o

echo "==> 清理缓存..."
sudo xbps-remove -O

echo "==> 完成!"
```

### 常用别名

添加到 `~/.bashrc`:

```bash
# XBPS 别名
alias xi='sudo xbps-install'
alias xs='xbps-query -Rs'
alias xr='sudo xbps-remove'
alias xu='sudo xbps-install -Su'

# 服务管理别名
alias svs='sudo sv status'
alias svr='sudo sv restart'
alias svstart='sudo sv start'
alias svstop='sudo sv stop'
```

---

## 相关资源

### 官方资源

- **官网**: https://voidlinux.org/
- **GitHub**: https://github.com/void-linux
- **文档**: https://docs.voidlinux.org/
- **Wiki**: https://wiki.voidlinux.org/
- **包搜索**: https://voidlinux.org/packages/

### 社区

- **Reddit**: r/voidlinux
- **IRC**: #voidlinux @ irc.libera.chat
- **论坛**: https://forum.voidlinux.org/

### 学习资源

- [Void Handbook](https://docs.voidlinux.org/)
- [runit documentation](http://smarden.org/runit/)
- [XBPS Manual](https://github.com/void-linux/xbps)
- [void-packages](https://github.com/void-linux/void-packages) - 包构建模板

### 镜像站点

**中国大陆**:
- https://mirrors.tuna.tsinghua.edu.cn/voidlinux/
- https://mirror.sjtu.edu.cn/voidlinux/
- https://mirrors.bfsu.edu.cn/voidlinux/

---

## 总结

Void Linux 是一个独特的发行版,它的 **runit** 和 **XBPS** 为用户提供了与主流发行版不同的体验。如果你:

- 厌倦了 systemd 的复杂性
- 喜欢简洁快速的系统
- 想尝试独立发行版
- 需要轻量级滚动发布系统
- 对容器和嵌入式感兴趣(musl版本)

那么 Void Linux 值得一试!虽然它的社区规模较小,但这也意味着更纯粹的技术交流和更专注的开发。

**一句话总结**: Void Linux = runit + XBPS + 滚动发布 + 独立精神

---

*最后更新: 2025-12-06*
