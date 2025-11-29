# 03 - 系统管理

> 进程、服务、用户和系统资源管理

## 目录

- [进程管理](#进程管理)
- [systemd 服务管理](#systemd-服务管理)
- [用户和组管理](#用户和组管理)
- [系统信息查看](#系统信息查看)
- [日志管理](#日志管理)
- [时间管理](#时间管理)
- [系统资源限制](#系统资源限制)
- [定时任务](#定时任务)

---

## 进程管理

### 进程查看

```bash
# 查看所有进程
ps aux

# 查看进程树
pstree

# 实时查看进程（类似 Windows 任务管理器）
top

# 彩色实时进程（推荐）
htop
# 安装: sudo pacman -S htop

# 列出当前用户的进程
ps -u

# 查看特定进程的PID
pidof process_name
pgrep process_name

# 查看进程的详细信息
ps -ef
```

### top/htop 快捷键

**top**:
```
空格      # 立即刷新
P         # 按CPU使用排序
M         # 按内存使用排序
N         # 按PID排序
T         # 按运行时间排序
u         # 按用户过滤
k         # 终止进程
r         # 调整优先级
q         # 退出
```

**htop** (更友好):
```
F1-10     # 各种排序和过滤选项
空格      # 标记进程
U         # 取消标记
k         # 杀死进程
l         # 查看打开的文件
s         # 跟踪系统调用
t         # 进程树显示
a         # 设置CPU亲和性
```

### 进程终止

```bash
# 按PID终止进程
kill 12345

# 强制终止（SIGKILL）
kill -9 12345

# 按名称终止进程
killall firefox

# 按名称模糊匹配终止
pkill firefox

# 交互式终止进程
killall -i firefox

# 终止进程组
kill -TERM -- -1234  # 负数表示进程组ID

# 不同信号
kill -TERM 12345     # 优雅终止 (15)
kill -KILL 12345     # 强制终止 (9)
kill -HUP 12345      # 挂起 (1)
kill -INT 12345      # 中断 (2)
kill -QUIT 12345     # 退出 (3)
kill -STOP 12345     # 停止 (19)
kill -CONT 12345     # 继续 (18)

# 常用信号速查
kill -l              # 列出所有信号
```

### 前台后台作业

```bash
# 后台运行
command &

# 前台运行后转到后台（已运行）
# Ctrl+Z暂停，然后
bg              # 后台继续运行

# 查看作业
jobs

# 恢复前台
fg

# 将后台作业转到前台
fg %1           # 作业号1

# 后台运行并关闭终端不退出
nohup command &

# 或使用 screen/tmux
screen -S session_name
# 之后可以 detach，不影响运行
```

### 优先级调整

```bash
# 启动时设置优先级（-20最高，19最低）
nice -n 10 command

# nice 值范围: -20(最高) 到 19(最低)
# 非root只能设置正数

# 查看进程优先级
ps -o pid,ni,pri,cmd

# 调整运行中进程优先级
renice 10 -p 12345

# 调整指定用户的所有进程
renice 5 -u username
```

---

## systemd 服务管理

### systemd 基础

**systemd** 是现代 Linux 的初始化系统和服务管理器,特点:
- 并行启动服务
- 自动依赖解析
- 统一的日志管理
- 完整的系统控制

### 基本命令

```bash
# 查看服务状态
systemctl status service_name

# 启动服务
sudo systemctl start service_name

# 停止服务
sudo systemctl stop service_name

# 重启服务
sudo systemctl restart service_name

# 重载配置
sudo systemctl reload service_name

# 设置开机自启
sudo systemctl enable service_name

# 禁用开机自启
sudo systemctl disable service_name

# 启用并启动
sudo systemctl enable --now service_name

# 禁用并停止
sudo systemctl disable --now service_name

# 查看是否开机自启
systemctl is-enabled service_name

# 查看服务是否运行
systemctl is-active service_name

# 查看服务是否失败
systemctl is-failed service_name
```

### 查看服务信息

```bash
# 查看服务详细信息
systemctl show service_name

# 查看服务文件位置
systemctl cat service_name

# 查看依赖关系
systemctl list-dependencies service_name

# 查看服务的安装状态
systemctl list-unit-files | grep service_name

# 查看所有服务
systemctl list-units --type=service

# 查看失败的服务
systemctl --failed

# 查看所有服务（包括不活跃的）
systemctl list-units --all --type=service

# 查看所有已启用的服务
systemctl list-unit-files --state=enabled

# 查看所有运行的服务
systemctl list-units --type=service --state=running
```

### 服务日志

```bash
# 查看服务日志
journalctl -u service_name

# 实时跟踪日志
journalctl -fu service_name

# 查看指定时间段的日志
journalctl -u service_name --since "1 hour ago"
journalctl -u service_name --since "2025-11-28 10:00:00" --until "2025-11-28 11:00:00"

# 查看最近的日志
journalctl -u service_name -n 100

# 显示详细信息
journalctl -u service_name -x

# 日志优先级
journalctl -u service_name -p err
# 优先级: emerg, alert, crit, err, warning, notice, info, debug
```

### 系统信息

```bash
# 分析启动时间
systemd-analyze

# 显示每个服务的启动时间
systemd-analyze blame

# 关键链分析
systemd-analyze critical-chain

# 查看单元列表
systemctl list-units

# 查看所有单元（包括失败的）
systemctl list-units --all

# 查看特定类型的所有单元
systemctl list-units --type=target
```

### 常见服务

**网络服务**:
```bash
# NetworkManager
sudo systemctl start NetworkManager
sudo systemctl enable NetworkManager

# SSH 服务
sudo systemctl start sshd
sudo systemctl enable sshd
```

**Web 服务器**:
```bash
# Apache (Arch: httpd)
sudo systemctl start httpd
sudo systemctl enable httpd

# Nginx
sudo systemctl start nginx
sudo systemctl enable nginx

# PHP-FPM
sudo systemctl start php-fpm
sudo systemctl enable php-fpm
```

### 目标（targets）

```bash
# 查看当前目标
systemctl get-default

# 设置默认目标
sudo systemctl set-default graphical.target  # 图形界面
sudo systemctl set-default multi-user.target # 命令行

# 切换目标（不重启）
sudo systemctl isolate graphical.target
sudo systemctl isolate multi-user.target

# 常见目标
graphical.target      # 图形界面
multi-user.target     # 多用户（命令行）
rescue.target         # 救援模式
emergency.target      # 紧急模式
```

### 重启和关机

```bash
# 重启
sudo reboot
sudo systemctl reboot

# 关机
sudo poweroff
sudo systemctl poweroff

# 挂起
sudo systemctl suspend

# 休眠
sudo systemctl hibernate

# 混合睡眠
sudo systemctl hybrid-sleep
```

### 创建自定义服务

创建 `/etc/systemd/system/myservice.service`:
```ini
[Unit]
Description=My Custom Service
After=network.target

[Service]
Type=simple
User=username
Group=groupname
WorkingDirectory=/path/to/directory
ExecStart=/path/to/command
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

应用服务:
```bash
# 重载 systemd
sudo systemctl daemon-reload

# 启用并启动服务
sudo systemctl enable --now myservice

# 查看状态
systemctl status myservice
```

### systemd 计时器（替代 cron）

创建服务 `/etc/systemd/system/mytask.service`:
```ini
[Unit]
Description=My Periodic Task

[Service]
Type=oneshot
ExecStart=/path/to/command
```

创建计时器 `/etc/systemd/system/mytask.timer`:
```ini
[Unit]
Description=Run my task every hour

[Timer]
OnBootSec=5min
OnUnitActiveSec=1h

[Install]
WantedBy=timers.target
```

启用计时器:
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now mytask.timer

# 查看计时器
systemctl list-timers
systemctl status mytask.timer
```

---

## 用户和组管理

### 添加用户

```bash
# 基本添加用户
sudo useradd -m username
# -m: 创建家目录

# 添加用户并设置属性
sudo useradd -m -s /bin/bash -c "Full Name" username
# -s: 指定shell
# -c: 注释（通常放全名）

# 创建系统用户（无登录权限）
sudo useradd -r -s /bin/false systemuser

# 批量添加用户（从文件）
# 文件格式: username:password
sudo newusers users.txt

# 设置密码
sudo passwd username

# 锁定/解锁账户
sudo passwd -l username  # 锁定
sudo passwd -u username  # 解锁
```

### 修改用户

```bash
# 修改用户信息
sudo usermod -c "New Full Name" username

# 修改shell
sudo usermod -s /bin/zsh username

# 锁定用户
sudo usermod -L username

# 解锁用户
sudo usermod -U username

# 添加到组
sudo usermod -aG groupname username
# -a: 添加（追加到现有组）
# -G: 组列表

# 修改主组
sudo usermod -g newgroup username

# 移动家目录
sudo usermod -d /new/home username

# 修改UID
sudo usermod -u 1001 username
```

### 删除用户

```bash
# 删除用户（保留家目录）
sudo userdel username

# 删除用户和家目录
sudo userdel -r username

# 强制删除（即使用户已登录）
sudo userdel -f username
```

### 查看用户信息

```bash
# 查看所有用户
cat /etc/passwd

# 查看当前用户
whoami
id
id username

# 查看当前登录用户
who
w
w  # 更详细的信息

# 查看用户历史登录
last
last username
lastlog  # 所有用户的最后登录

# 查看用户的组
groups username
groups
```

### 组管理

```bash
# 创建组
sudo groupadd groupname

# 创建系统组
sudo groupadd -r systemgroup

# 添加用户到组
sudo gpasswd -a username groupname

# 从组删除用户
sudo gpasswd -d username groupname

# 设置组管理员
sudo gpasswd -A username groupname
# 组管理员可以添加/删除成员

# 删除组
sudo groupdel groupname

# 查看组信息
cat /etc/group
```

### sudo 配置

```bash
# 编辑 sudo 配置
sudo visudo

# 基本 sudo 配置示例
# 用户 user1 可以 sudo 所有命令
user1 ALL=(ALL) ALL

# 用户 user2 可以 sudo 所有命令且无需密码
user2 ALL=(ALL) NOPASSWD: ALL

# 组 wheel 可以 sudo 所有命令
%wheel ALL=(ALL) ALL

# 用户 user3 可以 sudo 特定命令
user3 ALL=(ALL) /usr/bin/systemctl restart nginx

# 设置默认编辑器
export EDITOR=vim

# 查看 sudo 权限
sudo -l
```

### sudo 实用技巧

```bash
# 作为其他用户执行命令
sudo -u username command

# 以 root 身份执行（默认）
sudo command

# 保持环境变量
sudo -E command

# 执行多个命令
sudo bash -c "command1 && command2"

# 切换到 root（不安全）
sudo -i

# 查看 sudo 日志
sudo cat /var/log/auth.log
```

---

## 系统信息查看

### 基本信息

```bash
# 内核信息
uname -a                # 所有信息
uname -r                # 内核版本
uname -m                # 架构

# 发行版信息
cat /etc/os-release
lsb_release -a          # 需要安装: pacman -S lsb-release

# 主机名
hostnamectl
hostname

# 当前运行时间
uptime
whoami
```

### 硬件信息

```bash
# CPU 信息
lscpu
cat /proc/cpuinfo

# 内存信息
free -h
cat /proc/meminfo

# 磁盘信息
lsblk
fdisk -l
df -h

# PCI 设备
lspci

# USB 设备
lsusb

# 详细硬件信息
sudo dmidecode

# 硬件摘要
inxi
# 安装: sudo pacman -S inxi
```

### 实时系统状态

```bash
# 系统状态总览
top
htop

# CPU 使用
mpstat -P ALL 1 5
# 安装: sudo pacman -S sysstat

# 内存使用
free -h -s 1  # 每秒更新

# 磁盘 I/O
iostat -x 1
# 安装: sudo pacman -S sysstat

# 网络
iftop
# 安装: sudo pacman -S iftop

# 进程树
pstree -p
```

---

## 日志管理

### journalctl - systemd 日志

**基本用法**:
```bash
# 查看所有日志
journalctl

# 查看本轮启动的所有日志
journalctl -b

# 查看上一次启动的日志
journalctl -b -1

# 查看指定服务的日志
journalctl -u nginx

# 实时跟踪日志
journalctl -f

# 实时跟踪特定服务
journalctl -fu nginx

# 查看最近100行
journalctl -n 100

# 查看今天的日志
journalctl --since today

# 查看指定时间段的日志
journalctl --since "2025-11-28 10:00:00" --until "2025-11-28 11:00:00"
journalctl --since "1 hour ago"
journalctl --since "yesterday"
```

**过滤日志**:
```bash
# 按优先级过滤
journalctl -p err       # 只显示错误
journalctl -p warning  # 只显示警告
journalctl -p 0..3     # 范围过滤

# 按用户过滤
journalctl _UID=1000

# 按进程过滤
journalctl _PID=1234

# 按可执行文件过滤
journalctl _COMM=nginx

# 多个条件
journalctl -u nginx _PID=5678
```

**输出格式**:
```bash
# 详细输出
journalctl -x          # 附带说明

# JSON 格式
journalctl -o json

# 单行显示
journalctl -o cat

# 简洁显示
journalctl -o short

# 自定义格式
journalctl -o json-pretty
```

### 传统日志

```bash
# 内核消息
dmesg
dmesg -T        # 带时间戳
dmesg -w        # 实时跟踪

# 认证日志
tail -f /var/log/auth.log

# 系统日志
tail -f /var/log/syslog

# Apache 访问日志
tail -f /var/log/httpd/access_log

# Apache 错误日志
tail -f /var/log/httpd/error_log

# 应用程序日志
tail -f /var/log/app.log
```

### 日志轮转

```bash
# 查看 logrotate 配置
cat /etc/logrotate.conf

# 强制轮转
sudo logrotate -f /etc/logrotate.conf

# 测试轮转（不执行）
sudo logrotate -d /etc/logrotate.conf
```

---

## 时间管理

### 查看和设置时间

```bash
# 查看当前时间
date

# 查看硬件时钟
hwclock --show

# 查看时区
timedatectl

# 设置时区
sudo timedatectl set-timezone Asia/Shanghai

# 手动设置系统时间
sudo timedatectl set-time "2025-11-28 15:30:00"

# 同步硬件时钟
sudo hwclock --systohc
```

### NTP 时间同步

```bash
# 启用 NTP 同步
sudo timedatectl set-ntp true

# 查看 NTP 状态
timedatectl status

# 手动同步（一次性）
sudo ntpd -qg
sudo hwclock -w

# 或使用 chrony
sudo systemctl start chronyd
sudo systemctl enable chronyd
```

---

## 系统资源限制

### ulimit - 资源限制

```bash
# 查看当前限制
ulimit -a

# 查看特定限制
ulimit -n          # 打开文件数
ulimit -u          # 最大用户进程数
ulimit -f          # 文件大小限制
ulimit -t          # CPU时间限制

# 设置限制（当前会话）
ulimit -n 65536    # 设置打开文件数

# 永久设置（添加到 /etc/security/limits.conf 或用户 ~/.bashrc）
ulimit -n 65536    # 添加到 ~/.bashrc
```

### limits.conf

编辑 `/etc/security/limits.conf`:
```
# 格式: <domain> <type> <item> <value>

# 用户 alice 最大打开文件数
alice soft nofile 65536
alice hard nofile 65536

# 用户 alice 最大进程数
alice soft nproc  4096
alice hard nproc  8192

# 阻止用户 spam 用户登录
spam hard cpu 1
spam hard nproc 1

# 组 developers 的限制
@developers soft nofile 32768
@developers hard nofile 65536

# 软硬限制说明
# soft: 警告限制
# hard: 最大限制
```

### cgroups（容器资源限制）

```bash
# 查看 cgroup
cat /proc/self/cgroup

# 限制 CPU 使用（使用 systemd-run）
systemd-run --unit=cpu-limit --scope -p CPUQuota=50% command
```

---

## 定时任务

### cron - 传统定时任务

```bash
# 查看 crontab
crontab -l

# 编辑 crontab
crontab -e

# 为其他用户编辑
sudo crontab -e -u username

# crontab 格式
# 分 时 日 月 星期 命令
# *  *  *  *  *  *
# |  |  |  |  |  |
# |  |  |  |  |  +-- 星期几 (0-7, 0和7都表示周日)
# |  |  |  |  +------ 月份 (1-12)
# |  |  |  +--------- 日期 (1-31)
# |  |  +------------ 小时 (0-23)
# |  +--------------- 分钟 (0-59)
# +----------------- 秒 (0-59, 可选)
```

**示例**:
```bash
# 每天凌晨2点执行
0 2 * * * /path/to/script.sh

# 每周一凌晨3点执行
0 3 * * 1 /path/to/script.sh

# 每15分钟执行
*/15 * * * * /path/to/script.sh

# 每小时执行
0 * * * * /path/to/script.sh

# 每天的 8:00, 12:00, 18:00 执行
0 8,12,18 * * * /path/to/script.sh

# 每天 9:00 到 17:00 之间每小时执行
0 9-17 * * * /path/to/script.sh

# 每分钟执行
* * * * * /path/to/script.sh

# 每月的1号执行
0 0 1 * * /path/to/script.sh

# 每年的1月1号执行
0 0 1 1 * /path/to/script.sh
```

**特殊字符串**:
```bash
# @yearly    每年执行一次 (0 0 1 1 *)
# @monthly   每月执行一次 (0 0 1 * *)
# @weekly    每周执行一次 (0 0 * * 0)
# @daily     每天执行一次 (0 0 * * *)
# @hourly    每小时执行一次 (0 * * * *)
# @reboot    系统启动时执行

# 示例
@daily /path/to/daily-backup.sh
```

### systemd timer（推荐）

**优势**:
- 可以设置精确的依赖关系
- 日志集成到 journalctl
- 支持事件触发

**创建服务**:
```ini
# /etc/systemd/system/backup.service
[Unit]
Description=Backup Service

[Service]
Type=oneshot
ExecStart=/path/to/backup.sh
```

**创建计时器**:
```ini
# /etc/systemd/system/backup.timer
[Unit]
Description=Backup Timer

[Timer]
OnCalendar=daily
Persistent=true  # 如果错过时间，启动时立即执行

[Install]
WantedBy=timers.target
```

**启用**:
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now backup.timer

# 查看计时器
systemctl list-timers
```

---

## 常见系统管理任务

### 系统维护脚本

```bash
#!/bin/bash
# system-maintenance.sh

echo "=== 系统维护开始 ==="

# 1. 更新软件包
echo "1. 更新软件包..."
sudo pacman -Syu --noconfirm

# 2. 清理孤立包
echo "2. 清理孤立包..."
sudo pacman -Rns $(pacman -Qtdq) --noconfirm

# 3. 清理包缓存
echo "3. 清理包缓存..."
sudo paccache -r

# 4. 清理 systemd 日志
echo "4. 清理 systemd 日志（保留30天）..."
sudo journalctl --vacuum-time=30d

# 5. 清理临时文件
echo "5. 清理临时文件..."
sudo rm -rf /tmp/*
sudo rm -rf /var/tmp/*

# 6. 重建索引
echo "6. 重建文件索引..."
sudo updatedb  # 需要安装: pacman -S mlocate

# 7. 检查磁盘
echo "7. 检查磁盘..."
df -h

echo "=== 系统维护完成 ==="
```

### 监控脚本

```bash
#!/bin/bash
# monitor.sh - 系统监控

echo "=== 系统状态 ==="

echo "时间: $(date)"
echo "运行时间: $(uptime -p)"
echo "负载: $(uptime | awk -F'load average:' '{print $2}')"

echo ""
echo "=== 内存使用 ==="
free -h

echo ""
echo "=== 磁盘使用 ==="
df -h | grep -E '/(dev|nvme)'

echo ""
echo "=== CPU 使用前10进程 ==="
ps aux --sort=-%cpu | head -11

echo ""
echo "=== 内存使用前10进程 ==="
ps aux --sort=-%mem | head -11

echo ""
echo "=== 系统信息 ==="
uname -r
```

### 自动清理脚本

```bash
#!/bin/bash
# auto-cleanup.sh

# 清理下载目录
find ~/Downloads -type f -mtime +7 -delete

# 清理浏览器缓存
rm -rf ~/.cache/mozilla
rm -rf ~/.cache/google-chrome

# 清理日志文件（保留最近30天）
find /var/log -name "*.log" -mtime +30 -delete

# 清理 systemd 日志
sudo journalctl --vacuum-time=30d

echo "清理完成"
```

---

## 故障排除

### 系统启动问题

```bash
# 查看启动问题
journalctl -b -p err

# 分析启动时间
systemd-analyze blame

# 查看单元失败
systemctl --failed

# 查看依赖问题
systemctl list-dependencies --failed
```

### 性能问题

```bash
# 查看 CPU 瓶颈
top
htop

# 查看内存瓶颈
free -h
cat /proc/meminfo

# 查看 I/O 瓶颈
iotop

# 查看网络瓶颈
iftop
netstat -i

# 查看系统调用
strace -c command
```

### 磁盘空间问题

```bash
# 查找大文件
find / -type f -size +100M 2>/dev/null

# 查看目录大小
du -h --max-depth=1 | sort -hr

# 查看磁盘使用
df -h

# 查找最近修改的大文件
find / -type f -size +100M -mtime -7 2>/dev/null
```

---

## 相关资源

- [systemd 官方文档](https://www.freedesktop.org/software/systemd/man/)
- [Arch Wiki - systemd](https://wiki.archlinux.org/title/Systemd)
- [systemd 计时器](https://wiki.archlinux.org/title/Systemd/Timers)
- 上一章: [02-文本处理](../02-文本处理/)
- 下一章: [04-网络工具](../04-网络工具/)
