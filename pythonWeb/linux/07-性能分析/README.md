# 07 - 性能分析

> 系统性能监控和分析工具

## 目录

- [实时监控](#实时监控)
- [CPU 分析](#cpu-分析)
- [内存分析](#内存分析)
- [磁盘 I/O 分析](#磁盘-io-分析)
- [网络监控](#网络监控)
- [性能分析工具](#性能分析工具)
- [系统诊断](#系统诊断)
- [性能调优](#性能调优)

---

## 实时监控

### top - 经典监控

```bash
# 启动
top

# 显示进程树
top -o %CPU  # 按CPU排序
top -o %MEM  # 按内存排序

# 显示完整命令行
top -c

# 显示虚拟内存大小
top -v

# 以批处理模式运行（用于脚本）
top -b -n 1 > top-output.txt

# 设置刷新间隔
top -d 1  # 1秒刷新

# 只显示指定用户的进程
top -u username

# 只显示指定PID
top -p 1234

# 保存配置
# 在top中: W (大写)
```

**top 快捷键**:
```
h, ?      # 帮助
z        # 彩色/单色切换
x        # 高亮排序列
y        # 高亮运行进程
k        # 终止进程
r        # 调整优先级 (renice)
P        # 按CPU排序 (默认)
M        # 按内存排序
T        # 按时间排序
u        # 按用户过滤
n        # 设置显示进程数
f        # 选择显示列
F        # 选择排序列
c        # 显示完整命令行
L        # 搜索字符串
<, >     # 切换排序列
q        # 退出
```

**列说明**:
```
PID       # 进程ID
USER      # 用户
PR        # 优先级 (priority)
NI        # nice值 (-20高优先级, 19低优先级)
VIRT      # 虚拟内存
RES       # 常驻内存
SHR       # 共享内存
S         # 状态 (R运行 S睡眠 Z僵尸)
%CPU      # CPU使用率
%MEM      # 内存使用率
TIME+     # 累计CPU时间
COMMAND   # 命令
```

### htop - 增强版 top

```bash
# 安装
sudo pacman -S htop

# 启动
htop

# 无鼠标
htop -s PID

# 批处理模式
htop -b -n 1 > htop-output.txt

# 指定用户
htop -u username

# 指定进程
htop -p 1234,5678

# 显示线程
htop -t

# 显示树形
htop -y

# 切换颜色方案
htop -C
```

**htop 快捷键**:
```
F1, h     # 帮助
F2, s     # 设置
F3, /     # 搜索
F4, \     # 过滤
F5, t     # 树形显示
F6, >     # 选择排序列
F7, [     # 减少优先级
F8, ]     # 增加优先级
F9, k     # 杀死进程
F10, q    # 退出
Space     # 标记进程
c         # 标记子进程
U         # 取消标记
L         # 查找文件
H         # 显示/隐藏线程
K         # 显示/隐藏内核线程
P         # 按CPU排序
M         # 按内存排序
T         # 按时间排序
```

**颜色编码**:
- 绿色: 低优先级
- 蓝色: 标准优先级
- 红色: 内核线程
- 紫色: I/O 等待进程

### btop - 现代监控（推荐）

```bash
# 安装
sudo pacman -S btop

# 启动
btop

# 批处理模式
btop -b -n 1

# 指定主题
btop -t dark

# 自定义配置
# 按 ESC 进入菜单
# 按 q 退出
```

**功能**:
- CPU 图表
- 内存和交换分区使用
- 磁盘 I/O
- 网络 I/O
- 进程列表和详细信息
- 历史趋势图

### glances - 全方位监控

```bash
# 安装
sudo pacman -S glances

# 启动
glances

# 远程监控
glances -c -p 61209  # 客户端模式

# Web 界面
glances -w

# 导出数据
glances --export-csv  # CSV导出
glances --export-json # JSON导出

# 监控特定进程
glances --filter-process ssh

# 显示特定信息
glances -1  # 每CPU显示
```

### nmon - IBM 监控工具

```bash
# 安装
sudo pacman -S nmon

# 启动
nmon

# 命令行选项
nmon -f -s 60 -c 10  # 60秒采样，10次，共10分钟

# 查看结果
nmon_analyzer  # 需要Excel或LibreOffice
```

---

## CPU 分析

### mpstat - 多处理器统计

```bash
# 安装
sudo pacman -S sysstat

# 所有CPU
mpstat

# 指定CPU
mpstat -P ALL

# 间隔1秒采样10次
mpstat 1 10

# 显示所有间隔
mpstat -P ALL 1 5

# JSON 输出
mpstat -o JSON 1 5
```

**输出说明**:
```
%usr     # 用户空间CPU使用
%nice    # 低优先级进程CPU使用
%sys     # 内核空间CPU使用
%iowait  # I/O等待时间
%irq     # 硬件中断
%soft    # 软件中断
%steal   # 虚拟化中等待时间
%guest   # 虚拟化中客户机CPU时间
%idle    # 空闲时间
```

### pidstat - 进程统计

```bash
# 所有进程CPU使用
pidstat

# 指定PID
pidstat -p 1234

# 指定间隔
pidstat 2 5  # 2秒间隔，5次

# 显示所有PID
pidstat -p ALL

# CPU统计
pidstat -u 2 5  # CPU使用

# 内存统计
pidstat -r 2 5  # 内存使用

# I/O统计
pidstat -d 2 5  # 磁盘I/O

# 显示命令行
pidstat -u -p ALL -t

# 仅显示非零统计
pidstat -I -u
```

### lscpu - CPU信息

```bash
# 基本信息
lscpu

# 查看频率
lscpu | grep -E "CPU\(s\)|Max MHz|Min MHz"

# 查看所有信息
lscpu -e

# JSON格式
lscpu -e=JSON
```

### turbostat - 频率监控

```bash
# 需要 root
sudo pacman -S linux-tools-meta  # Arch: 需要内核工具

# 监控频率
sudo turbostat

# 指定间隔
sudo turbostat -i 1  # 1秒间隔

# 显示详细信息
sudo turbostat -v

# 不显示标题
sudo turbostat -q
```

---

## 内存分析

### free - 内存使用

```bash
# 人类可读格式
free -h

# 以MB为单位
free -m

# 以KB为单位
free -k

# 显示总计行
free -h -t

# 连续刷新
free -h -s 1

# 显示详细信息
free -h -w
```

**输出说明**:
```
total      # 总内存
used       # 已使用
free       # 完全空闲
buff/cache # 缓冲区 + 缓存
available  # 可用于启动新应用（估算值）

注意: available < free + buff/cache
因为缓存可以快速释放，available是实际可用值
```

### vmstat - 虚拟内存统计

```bash
# 安装
sudo pacman -S procps-ng

# 默认
vmstat

# 指定间隔和次数
vmstat 2 10  # 2秒间隔，10次

# 显示字段名称
vmstat -w

# 磁盘统计
vmstat -d

# 分页统计
vmstat -s

# 内存统计
vmstat -m
```

**输出说明**:
```
procs:
  r: 可运行进程数
  b: 阻塞进程数

memory:
  swpd: 交换分区使用
  free: 空闲内存
  buff: 缓冲区
  cache: 缓存
  inact: 非活动内存
  active: 活动内存

swap:
  si: 换入 (KB/s)
  so: 换出 (KB/s)

io:
  bi: 块设备接收 (blocks/s)
  bo: 块设备发送 (blocks/s)

system:
  in: 中断次数/秒
  cs: 上下文切换次数/秒

cpu:
  us: 用户时间
  sy: 系统时间
  id: 空闲时间
  wa: I/O等待时间
  st: 虚拟机偷取时间
```

### pmap - 进程内存映射

```bash
# 查看进程内存映射
pmap 1234

# 详细信息
pmap -d 1234

# 累积模式
pmap -X 1234

# 查看所有进程
pmap $(pgrep process_name)
```

### smem - 内存报告

```bash
# 安装
sudo pacman -S smem

# 显示内存使用
smem

# 按用户显示
smem -u

# 按映射显示
smem -m

# 显示百分比
smem -p

# 显示RSS
smem -r

# 显示图表
smem -t

# 过滤
smem -k -K | grep firefox

# 累计模式
smem -c "name user pid rss"
```

### /proc/meminfo - 详细内存信息

```bash
cat /proc/meminfo

# 常用字段
# MemTotal: 总内存
# MemFree: 完全空闲内存
# MemAvailable: 可用内存（最重要的值）
# Buffers: 缓冲区
# Cached: 文件缓存
# SwapTotal: 交换分区总量
# SwapFree: 交换分区空闲
# Committed_AS: 已提交的内存
```

---

## 磁盘 I/O 分析

### iostat - I/O 统计

```bash
# 安装
sudo pacman -S sysstat

# 默认显示
iostat

# 显示扩展信息
iostat -x

# 显示设备统计
iostat -d

# 显示CPU统计
iostat -c

# 指定间隔
iostat -x 2 5  # 2秒间隔，5次

# 只显示特定设备
iostat -x sda sdb

# 显示详细信息
iostat -y 2 5  # 跳过第一次统计

# 连续刷新
iostat -x 1

# JSON格式
iostat -j JSON -x 1 3
```

**字段说明**:
```
r/s    # 每秒读取次数
w/s    # 每秒写入次数
rMB/s  # 每秒读取MB
wMB/s  # 每秒写入MB
%util  # I/O利用率 (100% = 满负载)
avgrq-sz  # 平均请求大小
avgqu-sz  # 平均队列长度
await     # 平均等待时间(ms)
r_await   # 平均读取等待时间
w_await   # 平均写入等待时间
svctm     # 平均服务时间(已弃用)
```

### iotop - 实时 I/O 监控

```bash
# 需要 root
sudo pacman -S iotop

# 启动
sudo iotop

# 只显示有I/O的进程
sudo iotop -o

# 显示线程
sudo iotop -t

# 批处理模式
sudo iotop -b -n 5  # 5次采样

# 显示PID和用户
sudo iotop -P

# 指定间隔
sudo iotop -d 2  # 2秒间隔

# 过滤
sudo iotop -u username  # 指定用户
sudo iotop -p 1234      # 指定进程
```

**iotop 快捷键**:
```
方向键  # 切换排序
r       # 反转排序顺序
o       # 只显示有I/O的进程
p       # 只显示进程（默认）
t       # 显示/隐藏线程
a       # 累积模式
q       # 退出
```

### fio - I/O 基准测试

```bash
# 安装
sudo pacman -S fio

# 顺序读取测试
fio --name=seqread --rw=read --bs=1M --size=1G --iodepth=1 --numjobs=1 --runtime=60 --time_based --filename=/path/to/file

# 随机读取测试
fio --name=randread --rw=randread --bs=4k --size=1G --iodepth=64 --numjobs=4 --runtime=60 --time_based --filename=/path/to/file

# 混合读写测试
fio --name=mixed --rw=rw --rwmixread=70 --bs=4k --size=1G --iodepth=32 --numjobs=2 --runtime=60 --time_based --filename=/path/to/file

# 清理
rm -f /path/to/file
```

### dd - 简单速度测试

```bash
# 写入测试
dd if=/dev/zero of=/tmp/testfile bs=1G count=1 oflag=direct
# 清理
rm /tmp/testfile

# 读取测试
dd if=/path/to/largefile of=/dev/null bs=1M iflag=direct

# 同步测试
dd if=/dev/zero of=/tmp/testfile bs=1M count=100 oflag=direct
sync
dd if=/tmp/testfile of=/dev/null bs=1M iflag=direct
rm /tmp/testfile

# 测量缓存性能
dd if=/dev/zero of=/tmp/testfile bs=1M count=100
sync
dd if=/tmp/testfile of=/dev/null bs=1M
rm /tmp/testfile
```

---

## 网络监控

### iftop - 流量监控

```bash
# 安装
sudo pacman -S iftop

# 指定接口
sudo iftop -i eth0

# 显示端口
sudo iftop -i eth0 -P

# 显示历史流量
sudo iftop -i eth0 -P -t

# 批处理模式
sudo iftop -i eth0 -P -t -s 100

# 显示源地址和目标地址
sudo iftop -i eth0 -F 192.168.0.0/24
```

### nethogs - 进程网络监控

```bash
# 安装
sudo pacman -S nethogs

# 指定接口
sudo nethogs eth0

# 显示详细信息
sudo nethogs -d 1

# 显示累计流量
sudo nethogs eth0 -a

# 监控多个接口
sudo nethogs eth0 eth1
```

### ss - 网络连接分析

```bash
# 查看所有连接
ss -tan

# 查看监听端口
ss -tuln

# 显示进程
ss -tulnp

# 显示TCP状态
ss -tan state listening
ss -tan state established

# 查看特定端口
ss -tan dport = :80
ss -tan sport = :443

# 查看连接计时器
ss -to

# 显示详细信息
ss -i

# 显示内存信息
ss -m
```

---

## 性能分析工具

### perf - Linux 性能分析器

```bash
# 安装
sudo pacman -S perf

# 查看CPU性能事件
perf list

# 系统范围性能采样
sudo perf top

# 记录性能数据
sudo perf record -F 99 -a -- sleep 10

# 查看记录的报告
perf report

# 性能事件计数
sudo perf stat -d command

# CPU性能计数器
sudo perf stat -e cycles,instructions,cache-misses command

# 函数级分析
sudo perf record -g command
sudo perf report --stdio

# 性能火焰图
sudo perf record -F 99 -ag -- sleep 10
sudo perf script | stackcollapse-perf.pl | flamegraph.pl > perf.svg

# 安装火焰图工具
git clone https://github.com/brendangregg/FlameGraph
```

### strace - 系统调用跟踪

```bash
# 跟踪进程
strace command

# 跟踪指定进程
strace -p 1234

# 统计系统调用
strace -c command

# 跟踪特定系统调用
strace -e open,read command

# 记录到文件
strace -o output.txt command

# 附加到正在运行的进程
strace -p 1234 -f

# 跟踪子进程
strace -f command

# 显示时间戳
strace -t command

# 显示信号
strace -e signal command

# 只跟踪网络相关调用
strace -e trace=network command

# 过滤特定文件
strace -e trace=openat -f command

# CPU时间统计
strace -c -f command
```

### ltrace - 库调用跟踪

```bash
# 跟踪库调用
ltrace command

# 只跟踪特定库
ltrace -e malloc,free command

# 统计库调用
ltrace -c command

# 跟踪系统调用
ltrace -S command

# 显示返回值
ltrace -A 20 command
```

### valgrind - 内存错误检测

```bash
# 安装
sudo pacman -S valgrind

# 内存错误检查
valgrind --leak-check=full ./program

# 详细泄漏报告
valgrind --show-leak-kinds=all --track-origins=yes ./program

# 缓存性能分析
valgrind --tool=cachegrind ./program

# 查看 cachegrind 输出
cg_annotate cachegrind.out.*

# 堆分析
valgrind --tool=massif ./program
ms_print massif.out.*

# 线程错误检测
valgrind --tool=helgrind ./program

# 调用图
valgrind --tool=callgrind ./program
kcachegrind
```

---

## 系统诊断

### dmesg - 内核消息

```bash
# 查看所有消息
dmesg

# 显示时间戳
dmesg -T

# 实时跟踪
dmesg -w

# 过滤优先级
dmesg --level=err,warn

# 查找硬件问题
dmesg | grep -i error
dmesg | grep -i fail

# 查找USB设备
dmesg | grep -i usb

# 查找磁盘错误
dmesg | grep -i sd
dmesg | grep -i nvme

# CPU相关
dmesg | grep -i cpu

# 内存相关
dmesg | grep -i memory

# 网络相关
dmesg | grep -i eth
dmesg | grep -i network
```

### lscpu - CPU信息

```bash
lscpu

# 简洁输出
lscpu -e

# 查看频率
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq

# 查看频率范围
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_min_freq
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq
```

### lsblk - 块设备

```bash
# 查看设备
lsblk

# 显示更多信息
lsblk -f

# 显示权限
lsblk -m

# 显示从属关系
lsblk -p

# 显示I/O统计
lsblk -S
```

### smartctl - 硬盘健康检查

```bash
# 安装
sudo pacman -S smartmontools

# 查看设备信息
sudo smartctl -i /dev/sda

# 运行自检
sudo smartctl -t short /dev/sda

# 查看测试结果
sudo smartctl -l selftest /dev/sda

# 查看属性
sudo smartctl -A /dev/sda

# 查看健康状态
sudo smartctl -H /dev/sda

# 全面测试
sudo smartctl -t long /dev/sda

# 查看错误日志
sudo smartctl -l error /dev/sda
```

---

## 性能调优

### CPU 调优

```bash
# 查看CPU调度器
cat /sys/devices/system/cpu/cpu*/schedutil/*

# 调整CPU频率 (需要安装 cpupower)
sudo pacman -S cpupower

# 查看当前频率
cpufreq-info

# 设置CPU频率策略
sudo cpufreq-set -g performance  # 性能模式
sudo cpufreq-set -g powersave   # 省电模式
sudo cpufreq-set -g ondemand    # 按需模式

# 查看CPU使用详情
top
htop
```

### 内存调优

```bash
# 查看交换分区使用
swapon -s

# 关闭交换分区（高性能需求）
sudo swapoff -a

# 启用交换分区
sudo swapon -a

# 调整 swappiness (0-100, 越低越优先使用内存)
cat /proc/sys/vm/swappiness
sudo sysctl vm.swappiness=10

# 设置透明大页
echo madvise > /sys/kernel/mm/transparent_hugepage/enabled

# 调整OOM killer
cat /proc/sys/vm/oom_dump_tasks
sudo sysctl vm.oom_dump_tasks=0
```

### 磁盘调优

```bash
# 查看I/O调度器
cat /sys/block/sda/queue/scheduler

# 切换调度器
echo mq-deadline > /sys/block/sda/queue/scheduler
echo noop > /sys/block/sda/queue/scheduler
echo bfq > /sys/block/sda/queue/scheduler
echo kyber > /sys/block/sda/queue/scheduler

# 调整预读
blockdev --getra /dev/sda
sudo blockdev --setra 8192 /dev/sda

# 调整 I/O 优先级
ionice -c 2 -n 7 command  # 低优先级
ionice -c 1 -n 0 command  # 实时（危险）
```

### 网络调优

```bash
# 查看网络队列
ip link show

# 调整网络缓冲区
sysctl net.core.rmem_max
sysctl net.core.wmem_max

# TCP缓冲区
sysctl net.ipv4.tcp_rmem
sysctl net.ipv4.tcp_wmem

# 连接跟踪
sysctl net.netfilter.nf_conntrack_max

# 文件句柄
ulimit -n 65536
```

### sysctl - 永久调优

编辑 `/etc/sysctl.d/99-performance.conf`:
```
# 网络性能
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.ipv4.tcp_rmem = 4096 87380 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216
net.core.netdev_max_backlog = 5000

# 内存管理
vm.swappiness = 10
vm.dirty_ratio = 15
vm.dirty_background_ratio = 5
vm.vfs_cache_pressure = 50

# 文件系统
fs.file-max = 2097152
fs.inotify.max_user_watches = 524288
```

应用:
```bash
sudo sysctl -p /etc/sysctl.d/99-performance.conf
```

---

## 基准测试工具

### 内存基准

```bash
# 简单测试
dd if=/dev/zero of=/dev/shm/testfile bs=1G count=1
sync
rm /dev/shm/testfile

# 使用 memtester
sudo pacman -S memtester
sudo memtester 2G 1
```

### CPU基准

```bash
# 使用 sysbench
sudo pacman -S sysbench
sysbench cpu --threads=4 --time=10 run

# Prime numbers
sysbench cpu --cpu-max-prime=20000 --threads=4 run
```

### 磁盘基准

```bash
# fio（推荐）
fio --name=test --rw=randrw --bs=4k --size=1G --numjobs=4 --iodepth=64 --runtime=60 --time_based

# hdparm（IDE/SATA）
sudo pacman -S hdparm
sudo hdparm -tT /dev/sda
```

### 综合基准

```bash
# 使用 phoronix-test-suite
sudo pacman -S phoronix-test-suite

# 运行测试套件
phoronix-test-suite benchmark all
```

---

## 性能监控脚本

### 系统监控脚本

```bash
#!/bin/bash
# system-monitor.sh

echo "=== 系统性能报告 ==="
echo "时间: $(date)"
echo ""

echo "=== CPU ==="
top -bn1 | head -20
echo ""

echo "=== 内存 ==="
free -h
echo ""

echo "=== 磁盘使用 ==="
df -h | head -10
echo ""

echo "=== 负载平均值 ==="
uptime
echo ""

echo "=== 磁盘I/O ==="
iostat -x 1 2 | tail -10
echo ""

echo "=== 进程CPU前10 ==="
ps aux --sort=-%cpu | head -11
echo ""

echo "=== 进程内存前10 ==="
ps aux --sort=-%mem | head -11
echo ""

echo "=== 网络连接 ==="
ss -tan | wc -l
echo ""
```

### 资源趋势监控

```bash
#!/bin/bash
# resource-trend.sh

# 连续监控并记录
while true; do
    echo "$(date): $(top -bn1 | head -1 | awk '{print $8,$9,$10,$11,$12}')"
    sleep 60
done | tee /var/log/resource-trend.log
```

---

## 相关资源

- [性能调优指南](https://wiki.archlinux.org/title/Improving_performance)
- [perf 文档](https://perf.wiki.kernel.org/)
- [sysstat](https://github.com/sysstat/sysstat)
- [FlameGraph](https://github.com/brendangregg/FlameGraph)
- 上一章: [06-Arch包管理](../06-Arch包管理/)
- 下一章: [08-Shell编程](../08-Shell编程/)
