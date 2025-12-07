# 02 - Cgroups 详解

> Linux Cgroups (Control Groups) 是容器资源限制的核心

## 目录

- [什么是 Cgroups](#什么是-cgroups)
- [Cgroups 子系统详解](#cgroups-子系统详解)
- [Cgroups v1 vs v2](#cgroups-v1-vs-v2)
- [实战示例](#实战示例)
- [Docker 中的 Cgroups](#docker-中的-cgroups)

---

## 什么是 Cgroups

### 核心概念

> **Cgroups = 限制、记录、隔离进程组的资源使用**

```
Namespace:  隔离 (让进程看不到其他资源)
Cgroups:    限制 (限制进程能使用多少资源)

┌──────────────────────────────────────┐
│         物理资源                      │
│  CPU: 8核  内存: 16GB  磁盘: 1TB     │
└──────────────────────────────────────┘
              ↓
        Cgroups 分配
              ↓
  ┌──────────┬──────────┬──────────┐
  │ 容器 1    │ 容器 2    │ 容器 3    │
  │ 2核 4GB  │ 2核 4GB  │ 4核 8GB  │
  └──────────┴──────────┴──────────┘
```

### 三大功能

1. **资源限制** - 限制进程组使用的资源上限
2. **优先级控制** - 分配资源的优先级
3. **资源审计** - 记录和统计资源使用情况

### 查看 Cgroups

```bash
# 查看系统支持的 Cgroups 子系统
cat /proc/cgroups

# 输出示例:
# subsys_name    hierarchy   num_cgroups enabled
# cpuset         6           1           1
# cpu            7           86          1
# cpuacct        7           86          1
# blkio          9           86          1
# memory         10          86          1
# devices        2           86          1
# freezer        5           1           1

# 查看当前进程的 Cgroups
cat /proc/self/cgroup
```

---

## Cgroups 子系统详解

### 1. CPU 子系统

#### cpu - CPU 带宽限制

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/cpu/mygroup

# 设置 CPU 配额
# cpu.cfs_period_us: 周期（默认 100000 = 100ms）
# cpu.cfs_quota_us: 配额（-1 表示无限制）

# 限制为 1 个核心（100ms 中使用 100ms）
echo 100000 | sudo tee /sys/fs/cgroup/cpu/mygroup/cpu.cfs_quota_us
echo 100000 | sudo tee /sys/fs/cgroup/cpu/mygroup/cpu.cfs_period_us

# 限制为 0.5 个核心（100ms 中使用 50ms）
echo 50000 | sudo tee /sys/fs/cgroup/cpu/mygroup/cpu.cfs_quota_us

# 将进程加入 Cgroup
echo $$ | sudo tee /sys/fs/cgroup/cpu/mygroup/cgroup.procs

# 测试
# 运行 CPU 密集型程序
dd if=/dev/zero of=/dev/null &

# 观察 CPU 使用率（被限制在 50%）
top -p $!
```

#### cpuacct - CPU 使用统计

```bash
# 查看 CPU 使用情况
cat /sys/fs/cgroup/cpuacct/mygroup/cpuacct.usage
# 输出纳秒为单位的 CPU 使用时间

# 查看每个 CPU 的使用情况
cat /sys/fs/cgroup/cpuacct/mygroup/cpuacct.usage_percpu
```

#### cpuset - CPU 核心绑定

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/cpuset/mygroup

# 将进程绑定到特定 CPU 核心
echo "0,1" | sudo tee /sys/fs/cgroup/cpuset/mygroup/cpuset.cpus
# 只使用 CPU 0 和 1

# 设置内存节点（NUMA）
echo "0" | sudo tee /sys/fs/cgroup/cpuset/mygroup/cpuset.mems

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/cpuset/mygroup/cgroup.procs

# 验证
taskset -p $$
# 输出: pid xxxx's current affinity mask: 3 (二进制 11 = CPU 0,1)
```

---

### 2. Memory 子系统

#### 内存限制

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/memory/mygroup

# 限制内存为 100MB
echo 104857600 | sudo tee /sys/fs/cgroup/memory/mygroup/memory.limit_in_bytes

# 限制内存 + Swap 为 200MB
echo 209715200 | sudo tee /sys/fs/cgroup/memory/mygroup/memory.memsw.limit_in_bytes

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/memory/mygroup/cgroup.procs

# 测试内存限制
# 尝试分配 200MB 内存
python3 -c "x = 'a' * (200 * 1024 * 1024)"
# 会被 OOM Killer 终止
```

#### 内存统计

```bash
# 查看当前内存使用
cat /sys/fs/cgroup/memory/mygroup/memory.usage_in_bytes

# 查看最大内存使用
cat /sys/fs/cgroup/memory/mygroup/memory.max_usage_in_bytes

# 查看详细统计
cat /sys/fs/cgroup/memory/mygroup/memory.stat
# 输出包括:
# - cache: 页缓存
# - rss: 常驻内存
# - swap: 交换分区使用
# - pgfault: 页错误次数
```

#### OOM Control

```bash
# 禁用 OOM Killer（内存不足时不杀进程，而是阻塞）
echo 1 | sudo tee /sys/fs/cgroup/memory/mygroup/memory.oom_control

# 查看 OOM 事件计数
cat /sys/fs/cgroup/memory/mygroup/memory.oom_control
```

---

### 3. blkio - 磁盘 I/O 限制

#### 限制 I/O 带宽

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/blkio/mygroup

# 获取磁盘的主次设备号
ls -l /dev/sda
# brw-rw---- 1 root disk 8, 0 Dec  2 10:00 /dev/sda
# 主设备号: 8, 次设备号: 0

# 限制读取速度为 1MB/s
echo "8:0 1048576" | sudo tee /sys/fs/cgroup/blkio/mygroup/blkio.throttle.read_bps_device

# 限制写入速度为 1MB/s
echo "8:0 1048576" | sudo tee /sys/fs/cgroup/blkio/mygroup/blkio.throttle.write_bps_device

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/blkio/mygroup/cgroup.procs

# 测试
dd if=/dev/zero of=/tmp/testfile bs=1M count=100
# 写入速度被限制在 1MB/s
```

#### 限制 IOPS

```bash
# 限制读取 IOPS 为 100
echo "8:0 100" | sudo tee /sys/fs/cgroup/blkio/mygroup/blkio.throttle.read_iops_device

# 限制写入 IOPS 为 100
echo "8:0 100" | sudo tee /sys/fs/cgroup/blkio/mygroup/blkio.throttle.write_iops_device
```

#### I/O 统计

```bash
# 查看 I/O 统计
cat /sys/fs/cgroup/blkio/mygroup/blkio.throttle.io_service_bytes
# 输出每个设备的读写字节数

cat /sys/fs/cgroup/blkio/mygroup/blkio.throttle.io_serviced
# 输出每个设备的读写次数
```

---

### 4. devices - 设备访问控制

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/devices/mygroup

# 默认拒绝所有设备访问
echo "a" | sudo tee /sys/fs/cgroup/devices/mygroup/devices.deny

# 允许访问特定设备
# 格式: <type> <major>:<minor> <access>
# type: a(all), c(char), b(block)
# access: r(read), w(write), m(mknod)

# 允许读写 /dev/null (字符设备 1:3)
echo "c 1:3 rw" | sudo tee /sys/fs/cgroup/devices/mygroup/devices.allow

# 允许读写 /dev/zero (字符设备 1:5)
echo "c 1:5 rw" | sudo tee /sys/fs/cgroup/devices/mygroup/devices.allow

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/devices/mygroup/cgroup.procs

# 测试
cat /dev/null  # 成功
cat /dev/random  # 失败（权限被拒绝）
```

---

### 5. net_cls - 网络流量标记

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/net_cls/mygroup

# 设置 classid
echo 0x100001 | sudo tee /sys/fs/cgroup/net_cls/mygroup/net_cls.classid

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/net_cls/mygroup/cgroup.procs

# 配合 tc (traffic control) 使用
# 限制带宽为 1Mbps
sudo tc qdisc add dev eth0 root handle 1: htb default 30
sudo tc class add dev eth0 parent 1: classid 1:1 htb rate 1mbit
sudo tc filter add dev eth0 protocol ip parent 1:0 prio 1 handle 1: cgroup
```

---

### 6. freezer - 暂停/恢复进程

```bash
# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/freezer/mygroup

# 将进程加入
sleep 1000 &
echo $! | sudo tee /sys/fs/cgroup/freezer/mygroup/cgroup.procs

# 暂停所有进程
echo FROZEN | sudo tee /sys/fs/cgroup/freezer/mygroup/freezer.state

# 检查状态
ps aux | grep sleep  # 进程还在，但暂停了

# 恢复进程
echo THAWED | sudo tee /sys/fs/cgroup/freezer/mygroup/freezer.state
```

---

## Cgroups v1 vs v2

### 主要区别

| 特性 | Cgroups v1 | Cgroups v2 |
|-----|-----------|-----------|
| 层次结构 | 每个子系统独立层次 | 统一层次结构 |
| 挂载点 | /sys/fs/cgroup/<subsystem> | /sys/fs/cgroup |
| 配置文件 | 各子系统独立 | 统一接口 |
| 内存控制 | memory.limit_in_bytes | memory.max |
| CPU 控制 | cpu.shares, cpu.cfs_quota_us | cpu.weight, cpu.max |

### 检查 Cgroups 版本

```bash
# 检查系统使用的版本
mount | grep cgroup

# Cgroups v1:
# cgroup on /sys/fs/cgroup/cpu type cgroup (rw,...)
# cgroup on /sys/fs/cgroup/memory type cgroup (rw,...)

# Cgroups v2:
# cgroup2 on /sys/fs/cgroup type cgroup2 (rw,...)

# 或者
stat -fc %T /sys/fs/cgroup/
# v1: tmpfs
# v2: cgroup2fs
```

### Cgroups v2 示例

```bash
# 启用 Cgroups v2（需要内核支持）
# 在 /etc/default/grub 添加:
# GRUB_CMDLINE_LINUX="systemd.unified_cgroup_hierarchy=1"
# sudo grub-mkconfig -o /boot/grub/grub.cfg
# sudo reboot

# 创建 Cgroup
sudo mkdir /sys/fs/cgroup/mygroup

# 启用控制器
echo "+cpu +memory +io" | sudo tee /sys/fs/cgroup/cgroup.subtree_control

# 限制 CPU（权重模式）
echo "10000" | sudo tee /sys/fs/cgroup/mygroup/cpu.weight
# 默认 100，范围 1-10000

# 限制内存
echo "100M" | sudo tee /sys/fs/cgroup/mygroup/memory.max

# 限制 I/O
echo "8:0 rbps=1048576 wbps=1048576" | sudo tee /sys/fs/cgroup/mygroup/io.max

# 将进程加入
echo $$ | sudo tee /sys/fs/cgroup/mygroup/cgroup.procs
```

---

## 实战示例

### 示例 1: 限制进程内存和 CPU

```bash
#!/bin/bash
# limit_process.sh - 启动受限制的进程

# 1. 创建 Cgroups
sudo mkdir -p /sys/fs/cgroup/cpu/limited
sudo mkdir -p /sys/fs/cgroup/memory/limited

# 2. 设置 CPU 限制（50%）
echo 50000 | sudo tee /sys/fs/cgroup/cpu/limited/cpu.cfs_quota_us
echo 100000 | sudo tee /sys/fs/cgroup/cpu/limited/cpu.cfs_period_us

# 3. 设置内存限制（512MB）
echo 536870912 | sudo tee /sys/fs/cgroup/memory/limited/memory.limit_in_bytes

# 4. 启动进程
./my_program &
PID=$!

# 5. 将进程加入 Cgroups
echo $PID | sudo tee /sys/fs/cgroup/cpu/limited/cgroup.procs
echo $PID | sudo tee /sys/fs/cgroup/memory/limited/cgroup.procs

echo "进程 $PID 已启动，资源受限"

# 6. 监控资源使用
watch -n 1 "
  echo 'CPU 使用:'
  cat /sys/fs/cgroup/cpu/limited/cpuacct.usage
  echo 'Memory 使用:'
  cat /sys/fs/cgroup/memory/limited/memory.usage_in_bytes
"

# 7. 清理
wait $PID
sudo rmdir /sys/fs/cgroup/cpu/limited
sudo rmdir /sys/fs/cgroup/memory/limited
```

### 示例 2: 压力测试

```bash
# 安装 stress 工具
sudo pacman -S stress

# 不受限制的压力测试
stress --cpu 4 --vm 2 --vm-bytes 512M --timeout 60s

# 创建受限 Cgroup
sudo mkdir -p /sys/fs/cgroup/{cpu,memory}/stress_test

# 限制 CPU 为 100%（1 核心）
echo 100000 | sudo tee /sys/fs/cgroup/cpu/stress_test/cpu.cfs_quota_us

# 限制内存为 256MB
echo 268435456 | sudo tee /sys/fs/cgroup/memory/stress_test/memory.limit_in_bytes

# 在 Cgroup 中运行 stress
sudo cgexec -g cpu,memory:stress_test \
  stress --cpu 4 --vm 2 --vm-bytes 512M --timeout 60s

# 观察资源使用
htop  # CPU 使用被限制在 100%
```

---

## Docker 中的 Cgroups

### Docker 如何使用 Cgroups

```bash
# 启动 Docker 容器
docker run -d --name test \
  --cpus=0.5 \
  --memory=512m \
  nginx

# 查看容器的 Cgroup
docker inspect test | grep -A 10 "CgroupParent"

# 直接查看 Cgroup 文件
CONTAINER_ID=$(docker inspect -f '{{.Id}}' test)

# CPU 限制
cat /sys/fs/cgroup/cpu/docker/$CONTAINER_ID/cpu.cfs_quota_us

# 内存限制
cat /sys/fs/cgroup/memory/docker/$CONTAINER_ID/memory.limit_in_bytes

# I/O 统计
cat /sys/fs/cgroup/blkio/docker/$CONTAINER_ID/blkio.throttle.io_service_bytes
```

### Docker 资源限制选项

```bash
# CPU 限制
docker run --cpus=2 nginx          # 限制 2 个 CPU
docker run --cpu-shares=512 nginx  # CPU 权重
docker run --cpuset-cpus="0,1" nginx  # 绑定 CPU 0,1

# 内存限制
docker run --memory=512m nginx          # 限制内存 512MB
docker run --memory-swap=1g nginx       # 限制内存+Swap
docker run --memory-reservation=256m nginx  # 软限制
docker run --oom-kill-disable nginx     # 禁用 OOM Killer

# I/O 限制
docker run --device-read-bps=/dev/sda:1mb nginx   # 读取限制
docker run --device-write-bps=/dev/sda:1mb nginx  # 写入限制
docker run --device-read-iops=/dev/sda:100 nginx  # IOPS 限制
```

### 实时监控 Docker 资源

```bash
# Docker stats
docker stats

# 输出:
# CONTAINER  CPU %  MEM USAGE / LIMIT  MEM %  NET I/O  BLOCK I/O
# test       0.5%   50MB / 512MB       10%    1KB/0B   10MB/0B

# 查看单个容器
docker stats test --no-stream

# 导出为 JSON
docker stats test --no-stream --format "{{json .}}"
```

---

## 实用脚本

### Cgroup 管理工具

```bash
#!/bin/bash
# cgroup_manager.sh - Cgroup 管理脚本

function create_cgroup() {
    local name=$1
    local cpu_limit=$2  # 百分比
    local mem_limit=$3  # MB

    # 创建目录
    sudo mkdir -p /sys/fs/cgroup/cpu/$name
    sudo mkdir -p /sys/fs/cgroup/memory/$name

    # 设置 CPU 限制
    local quota=$((100000 * cpu_limit / 100))
    echo $quota | sudo tee /sys/fs/cgroup/cpu/$name/cpu.cfs_quota_us

    # 设置内存限制
    local mem_bytes=$((mem_limit * 1024 * 1024))
    echo $mem_bytes | sudo tee /sys/fs/cgroup/memory/$name/memory.limit_in_bytes

    echo "Created cgroup: $name (CPU: ${cpu_limit}%, Memory: ${mem_limit}MB)"
}

function add_process() {
    local name=$1
    local pid=$2

    echo $pid | sudo tee /sys/fs/cgroup/cpu/$name/cgroup.procs
    echo $pid | sudo tee /sys/fs/cgroup/memory/$name/cgroup.procs

    echo "Added process $pid to cgroup $name"
}

function show_stats() {
    local name=$1

    echo "=== Cgroup Stats: $name ==="
    echo "CPU usage: $(cat /sys/fs/cgroup/cpu/$name/cpuacct.usage) ns"
    echo "Memory usage: $(cat /sys/fs/cgroup/memory/$name/memory.usage_in_bytes) bytes"
}

function delete_cgroup() {
    local name=$1

    sudo rmdir /sys/fs/cgroup/cpu/$name
    sudo rmdir /sys/fs/cgroup/memory/$name

    echo "Deleted cgroup: $name"
}

# 使用示例
create_cgroup "myapp" 50 512
add_process "myapp" $$
show_stats "myapp"
# delete_cgroup "myapp"
```

---

## 常见问题

### Q1: Cgroups 和 Namespace 的区别？
**A**:
- Namespace: 隔离（看不到）
- Cgroups: 限制（用多少）

### Q2: 为什么我的 Cgroup 限制不生效？
**A**: 检查：
1. 是否有足够的权限（需要 sudo）
2. Cgroup 是否已挂载
3. 进程是否已加入 Cgroup
4. 限制值是否正确设置

### Q3: Cgroups v2 的优势？
**A**:
- 统一的层次结构
- 更好的性能
- 更简单的配置

### Q4: 如何在生产环境使用 Cgroups？
**A**: 建议：
- 使用 systemd (自动管理 Cgroups)
- 使用 Docker/Kubernetes (封装了 Cgroups)
- 不要手动管理，除非必要

---

## 下一步

学完 Cgroups，继续学习 [03-Chroot详解](./03-Chroot详解.md)，理解文件系统隔离！

---

*更新日期: 2025-12-02*
