# 01 - Namespace 详解

> Linux Namespace 是容器隔离的基石

## 目录

- [什么是 Namespace](#什么是-namespace)
- [7 种 Namespace 详解](#7-种-namespace-详解)
- [实战：手动创建 Namespace](#实战手动创建-namespace)
- [Namespace API](#namespace-api)
- [实用工具](#实用工具)

---

## 什么是 Namespace

### 核心概念

> **Namespace 让进程"看到"不同的系统视图**

```
没有 Namespace:
  所有进程共享同一个系统视图
  ├── 进程 A 可以看到进程 B
  ├── 进程 B 可以看到进程 C
  └── 所有进程看到相同的网络、文件系统

有了 Namespace:
  每个进程组有独立的系统视图
  ├── Namespace 1
  │   ├── 进程 A (只能看到 Namespace 1 的资源)
  │   └── 进程 B
  ├── Namespace 2
  │   └── 进程 C (只能看到 Namespace 2 的资源)
  └── Namespace 3
      └── 进程 D
```

### 类比理解

```
想象一个大房间（系统）:

没有隔断:
  所有人在同一个房间
  可以看到彼此
  共享所有资源

有了隔断（Namespace）:
  房间被分成多个小隔间
  每个隔间的人只能看到自己隔间的内容
  但实际上还是同一个大房间（同一个内核）
```

### 查看 Namespace

```bash
# 查看当前进程的 Namespace
ls -l /proc/$$/ns/

# 输出示例:
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 cgroup -> 'cgroup:[4026531835]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 ipc -> 'ipc:[4026531839]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 mnt -> 'mnt:[4026531840]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 net -> 'net:[4026531992]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 pid -> 'pid:[4026531836]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 pid_for_children -> 'pid:[4026531836]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 user -> 'user:[4026531837]'
# lrwxrwxrwx 1 user user 0 Dec  2 10:00 uts -> 'uts:[4026531838]'

lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 cgroup -> 'cgroup:[4026531835]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 ipc -> 'ipc:[4026531839]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 mnt -> 'mnt:[4026531832]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 net -> 'net:[4026531833]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 pid -> 'pid:[4026531836]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 pid_for_children -> 'pid:[4026531836]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 time -> 'time:[4026531834]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 time_for_children -> 'time:[4026531834]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 user -> 'user:[4026531837]'
lrwxrwxrwx 1 seeback seeback 0 12月 2日 21:37 uts -> 'uts:[4026531838]'

# 数字是 Namespace ID，相同ID表示在同一个Namespace中
```

---

## 7 种 Namespace 详解

### 1. PID Namespace (进程ID隔离)

#### 作用
隔离进程 ID 号空间，让不同 Namespace 中的进程拥有独立的 PID。

#### 特点
- 容器内的第一个进程 PID 为 1
- 容器内只能看到自己 Namespace 的进程
- 父 Namespace 可以看到子 Namespace 的进程

#### 实战示例

```bash
# 1. 查看当前系统的进程树
pstree -p

# 2. 创建新的 PID Namespace
sudo unshare --fork --pid --mount-proc /bin/bash

# 3. 在新 Namespace 中查看进程
echo $$  # 输出: 1（当前 bash 的 PID 是 1！）
ps aux   # 只能看到当前 Namespace 的进程

# 4. 在另一个终端查看（宿主机）
ps aux | grep bash  # 可以看到这个 bash 的真实 PID
```

#### 深入理解

```
宿主机视图:                    容器视图:
PID  进程                      PID  进程
1    systemd                   1    /bin/bash
100  sshd                      (看不到宿主机的进程)
1234 /bin/bash (容器进程)
1235 ps aux (在容器内运行)
```

#### C 语言示例

```c
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int child_func(void *arg) {
    printf("子进程 PID: %d\n", getpid());  // 输出: 1
    printf("父进程 PID: %d\n", getppid()); // 输出: 0

    // 执行 shell
    execl("/bin/bash", "/bin/bash", NULL);
    return 0;
}

int main() {
    printf("主进程 PID: %d\n", getpid());

    // 创建子进程栈
    char *stack = malloc(1024 * 1024);
    char *stack_top = stack + 1024 * 1024;

    // 创建新的 PID Namespace
    pid_t pid = clone(child_func, stack_top,
                      CLONE_NEWPID | SIGCHLD, NULL);

    waitpid(pid, NULL, 0);
    return 0;
}

// 编译运行:
// gcc -o pid_ns pid_ns.c
// sudo ./pid_ns
```

---

### 2. Network Namespace (网络隔离)

#### 作用
隔离网络设备、IP 地址、端口、路由表、防火墙规则等。

#### 特点
- 每个 Network Namespace 有独立的网络栈
- 默认只有 loopback 设备
- 可以通过 veth pair 连接不同 Namespace

#### 实战示例

```bash
# 1. 创建新的 Network Namespace
sudo ip netns add netns1

# 2. 列出所有 Network Namespace
ip netns list

# 3. 在新 Namespace 中执行命令
sudo ip netns exec netns1 ip addr show
# 只能看到 lo (loopback) 设备

# 4. 在新 Namespace 中启动 bash
sudo ip netns exec netns1 /bin/bash

# 5. 在这个 bash 中
ip addr show          # 只有 lo
ping 8.8.8.8          # 失败（没有网络设备）

# 6. 为 Namespace 添加网络设备
# 创建 veth pair
sudo ip link add veth0 type veth peer name veth1

# 将 veth1 移到 netns1
sudo ip link set veth1 netns netns1

# 配置 IP 地址
sudo ip addr add 192.168.1.1/24 dev veth0
sudo ip link set veth0 up

sudo ip netns exec netns1 ip addr add 192.168.1.2/24 dev veth1
sudo ip netns exec netns1 ip link set veth1 up
sudo ip netns exec netns1 ip link set lo up

# 7. 测试连通性
sudo ip netns exec netns1 ping 192.168.1.1

# 8. 删除 Network Namespace
sudo ip netns del netns1
```

#### 容器网络示例

```bash
# Docker 容器的网络隔离
docker run -d --name nginx nginx

# 查看容器的网络 Namespace
docker inspect nginx | grep SandboxKey
# 输出类似: /var/run/docker/netns/xxxxx

# 进入容器的网络 Namespace
container_pid=$(docker inspect -f '{{.State.Pid}}' nginx)
sudo nsenter -t $container_pid -n ip addr show

# 容器有自己独立的网络栈
```

#### 完整的容器网络示例

```bash
#!/bin/bash
# 创建一个具有网络的容器 Namespace

# 1. 创建 Network Namespace
sudo ip netns add container1

# 2. 创建 veth pair
sudo ip link add veth0 type veth peer name veth1

# 3. 将 veth1 放入 container1
sudo ip link set veth1 netns container1

# 4. 配置宿主机端
sudo ip addr add 10.0.0.1/24 dev veth0
sudo ip link set veth0 up

# 5. 配置容器端
sudo ip netns exec container1 ip addr add 10.0.0.2/24 dev veth1
sudo ip netns exec container1 ip link set veth1 up
sudo ip netns exec container1 ip link set lo up

# 6. 配置默认路由
sudo ip netns exec container1 ip route add default via 10.0.0.1

# 7. 启用 IP 转发
sudo sysctl -w net.ipv4.ip_forward=1

# 8. 配置 NAT
sudo iptables -t nat -A POSTROUTING -s 10.0.0.0/24 -j MASQUERADE

# 9. 测试
sudo ip netns exec container1 ping 8.8.8.8

# 10. 清理
sudo ip netns del container1
sudo ip link del veth0
```

---

### 3. Mount Namespace (挂载点隔离)

#### 作用
隔离文件系统挂载点，让不同 Namespace 看到不同的文件系统层次。

#### 特点
- 子 Namespace 继承父 Namespace 的挂载点
- 挂载和卸载操作互不影响
- 支持传播属性（shared, slave, private, unbindable）

#### 实战示例

```bash
# 1. 创建测试目录
mkdir -p /tmp/mnt_test/{host,container}

# 2. 在宿主机挂载
sudo mount -t tmpfs tmpfs /tmp/mnt_test/host
echo "host data" | sudo tee /tmp/mnt_test/host/file.txt

# 3. 创建新的 Mount Namespace
sudo unshare --mount /bin/bash

# 4. 在新 Namespace 中
mount -t tmpfs tmpfs /tmp/mnt_test/container
echo "container data" > /tmp/mnt_test/container/file.txt

# 查看挂载点
mount | grep mnt_test

# 5. 在宿主机的另一个终端
mount | grep mnt_test
# 看不到容器内的挂载！

# 6. 退出容器 Namespace
exit

# 挂载点消失
mount | grep mnt_test/container  # 没有输出
```

#### 容器的 rootfs 示例

```bash
# 准备 rootfs
mkdir -p /tmp/container_root
cd /tmp/container_root

# 创建基本目录结构
mkdir -p {bin,lib,lib64,proc,sys,dev,etc}

# 复制必要的二进制文件
cp /bin/bash bin/
cp /bin/ls bin/
cp /bin/cat bin/

# 复制依赖库
ldd /bin/bash | grep -o '/lib.*\.[^ ]*' | xargs -I {} cp {} lib/
ldd /bin/ls | grep -o '/lib.*\.[^ ]*' | xargs -I {} cp {} lib/

# 创建新的 Mount Namespace 并 chroot
sudo unshare --mount --fork /bin/bash -c "
  mount --bind /tmp/container_root /tmp/container_root
  mount --make-private /tmp/container_root
  chroot /tmp/container_root /bin/bash
"

# 在容器内
ls /      # 只能看到 bin, lib, lib64, proc, sys, dev, etc
pwd       # /
ps aux    # 失败（需要挂载 /proc）
```

#### 挂载传播属性

```bash
# 1. shared (共享)
# 父子 Namespace 的挂载操作相互可见
sudo mount --make-shared /mnt

# 2. slave (从属)
# 父 Namespace 的挂载可见，但子的不可见
sudo mount --make-slave /mnt

# 3. private (私有)
# 挂载操作完全隔离
sudo mount --make-private /mnt

# 4. unbindable (不可绑定)
# 不能作为 bind mount 的源
sudo mount --make-unbindable /mnt
```

---

### 4. UTS Namespace (主机名隔离)

#### 作用
隔离主机名(hostname)和域名(domainname)。

#### 特点
- 每个 Namespace 可以有独立的主机名
- 不影响其他 Namespace

#### 实战示例

```bash
# 1. 查看当前主机名
hostname

# 2. 创建新的 UTS Namespace
sudo unshare --uts /bin/bash

# 3. 修改主机名
hostname container1
hostname  # 输出: container1

# 4. 在宿主机的另一个终端
hostname  # 输出: 原来的主机名（未改变）

# 5. 退出容器
exit

# 主机名恢复
hostname  # 输出: 原来的主机名
```

#### C 语言示例

```c
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int child_func(void *arg) {
    // 设置主机名
    sethostname("container", strlen("container"));

    char hostname[100];
    gethostname(hostname, sizeof(hostname));
    printf("容器主机名: %s\n", hostname);

    execl("/bin/bash", "/bin/bash", NULL);
    return 0;
}

int main() {
    char hostname[100];
    gethostname(hostname, sizeof(hostname));
    printf("宿主机主机名: %s\n", hostname);

    char *stack = malloc(1024 * 1024);
    char *stack_top = stack + 1024 * 1024;

    // 创建新的 UTS Namespace
    clone(child_func, stack_top,
          CLONE_NEWUTS | SIGCHLD, NULL);

    wait(NULL);

    gethostname(hostname, sizeof(hostname));
    printf("宿主机主机名未变: %s\n", hostname);

    return 0;
}
```

---

### 5. IPC Namespace (进程间通信隔离)

#### 作用
隔离 System V IPC 和 POSIX 消息队列。

#### 特点
- 隔离消息队列、信号量、共享内存
- 不同 Namespace 的进程无法通过 IPC 通信

#### 实战示例

```bash
# 1. 在宿主机创建消息队列
ipcmk -Q  # 输出: Message queue id: 0

# 查看消息队列
ipcs -q
# ------ Message Queues --------
# key        msqid      owner      perms      used-bytes   messages
# 0x2c0a14f3 0          user       644        0            0

# 2. 创建新的 IPC Namespace
sudo unshare --ipc /bin/bash

# 3. 在新 Namespace 中查看
ipcs -q
# ------ Message Queues --------
# (空的！看不到宿主机的消息队列)

# 4. 在新 Namespace 中创建消息队列
ipcmk -Q  # 输出: Message queue id: 0

# 5. 在宿主机的另一个终端
ipcs -q
# 只能看到宿主机的消息队列，看不到容器的

# 6. 清理
ipcrm -q 0
exit
```

#### 测试程序

```c
// 宿主机程序 (host_ipc.c)
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    key_t key = ftok("/tmp", 'A');
    int msgid = msgget(key, 0666 | IPC_CREAT);
    printf("宿主机消息队列 ID: %d\n", msgid);

    struct {
        long type;
        char text[100];
    } msg;

    msg.type = 1;
    sprintf(msg.text, "来自宿主机的消息");
    msgsnd(msgid, &msg, sizeof(msg.text), 0);

    return 0;
}

// 容器程序 (container_ipc.c)
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    key_t key = ftok("/tmp", 'A');
    int msgid = msgget(key, 0666);  // 不创建，只获取

    if (msgid == -1) {
        printf("容器无法访问宿主机的消息队列\n");
        return 1;
    }

    printf("容器消息队列 ID: %d\n", msgid);
    return 0;
}

// 编译:
// gcc -o host_ipc host_ipc.c
// gcc -o container_ipc container_ipc.c

// 运行:
// ./host_ipc
// sudo unshare --ipc ./container_ipc
```

---

### 6. User Namespace (用户隔离)

#### 作用
隔离用户和组 ID，让容器内的 root 不是宿主机的 root。

#### 特点
- 容器内可以是 root (UID 0)
- 但在宿主机上是普通用户
- 增强安全性

#### 实战示例

```bash
# 1. 创建新的 User Namespace（不需要 sudo！）
unshare --user /bin/bash

# 2. 查看用户
id
# uid=65534(nobody) gid=65534(nogroup)  # 默认映射

# 3. 配置 UID/GID 映射
# 退出后重新创建
unshare --user --map-root-user /bin/bash

# 4. 再次查看
id
# uid=0(root) gid=0(root)  # 容器内是 root！

# 5. 但在宿主机上
echo $$  # 获取当前 bash 的 PID
# 在另一个终端
ps -o user,pid | grep <PID>
# user     <PID>  # 宿主机上仍是普通用户
```

#### UID 映射详解

```bash
# 查看 UID 映射
cat /proc/$$/uid_map
#  容器内UID  宿主机UID  映射范围
#  0          1000        1

# 这表示:
# 容器内的 UID 0 (root) 对应宿主机的 UID 1000 (普通用户)

# 手动配置映射
unshare --user /bin/bash

# 在另一个终端
echo "0 1000 1" | sudo tee /proc/<PID>/uid_map
echo "0 1000 1" | sudo tee /proc/<PID>/gid_map
```

#### 无 root 容器示例

```bash
# 普通用户创建容器（无需 sudo）
unshare --user --map-root-user \
        --pid --fork \
        --mount-proc \
        /bin/bash

# 在容器内
id  # uid=0(root)
ps aux  # PID namespace 隔离

# 尝试修改系统文件
echo "test" > /etc/hosts  # 失败！（只有映射的权限）
```

---

### 7. Cgroup Namespace (Cgroup 根目录隔离)

#### 作用
隔离 Cgroup 根目录，让容器看不到完整的 Cgroup 层次结构。

#### 特点
- 容器内的 /proc/self/cgroup 显示相对路径
- 增强安全性

#### 实战示例

```bash
# 1. 在宿主机查看 Cgroup
cat /proc/self/cgroup
# 输出类似:
# 0::/user.slice/user-1000.slice/session-1.scope

# 2. 创建新的 Cgroup Namespace
sudo unshare --cgroup --fork /bin/bash

# 3. 在新 Namespace 中查看
cat /proc/self/cgroup
# 输出:
# 0::/

# 容器看到的是相对于自己的 Cgroup 路径
```

---

## 实战：手动创建 Namespace

### 组合使用多个 Namespace

```bash
# 创建一个完整隔离的环境
sudo unshare \
    --pid --fork \           # PID 隔离
    --mount-proc \           # 挂载新的 /proc
    --net \                  # 网络隔离
    --uts \                  # 主机名隔离
    --ipc \                  # IPC 隔离
    --user --map-root-user \ # 用户隔离
    --cgroup \               # Cgroup 隔离
    /bin/bash

# 在这个环境中
hostname isolated-container
hostname

id  # uid=0(root)
ps aux  # 只看到少量进程
ip addr show  # 只有 lo 设备
```

### 构建简易容器

```bash
#!/bin/bash
# simple_container.sh - 创建一个简易容器

ROOTFS="/tmp/simple_container_rootfs"

# 1. 准备 rootfs
echo "准备 rootfs..."
mkdir -p $ROOTFS/{bin,lib,lib64,proc,sys,dev,etc}

# 复制必要的二进制
cp /bin/{bash,ls,cat,ps,mount} $ROOTFS/bin/

# 复制依赖库
for binary in bash ls cat ps mount; do
    ldd /bin/$binary | grep -o '/lib.*\.[^ ]*' | xargs -I {} cp {} $ROOTFS/lib/
done

# 2. 创建 Namespace 并启动容器
sudo unshare \
    --pid --fork \
    --mount \
    --uts \
    --ipc \
    --net \
    --user --map-root-user \
    /bin/bash -c "
        # 挂载 proc
        mount -t proc proc $ROOTFS/proc

        # 挂载 sys
        mount -t sysfs sys $ROOTFS/sys

        # 切换根目录
        mount --bind $ROOTFS $ROOTFS
        cd $ROOTFS
        mkdir -p old_root
        pivot_root . old_root

        # 卸载旧的根
        umount -l /old_root
        rmdir /old_root

        # 设置主机名
        hostname simple-container

        # 启动 shell
        /bin/bash
    "

# 3. 清理
sudo rm -rf $ROOTFS
```

---

## Namespace API

### 系统调用

#### 1. clone()
创建新进程并指定 Namespace。

```c
#include <sched.h>

int clone(int (*fn)(void *), void *child_stack,
          int flags, void *arg, ...);

// flags 可以是:
// CLONE_NEWPID   - 新的 PID Namespace
// CLONE_NEWNET   - 新的 Network Namespace
// CLONE_NEWNS    - 新的 Mount Namespace
// CLONE_NEWUTS   - 新的 UTS Namespace
// CLONE_NEWIPC   - 新的 IPC Namespace
// CLONE_NEWUSER  - 新的 User Namespace
// CLONE_NEWCGROUP- 新的 Cgroup Namespace
```

#### 2. unshare()
让当前进程加入新的 Namespace。

```c
#include <sched.h>

int unshare(int flags);

// 示例
unshare(CLONE_NEWPID | CLONE_NEWNET);
```

#### 3. setns()
让当前进程加入已存在的 Namespace。

```c
#include <sched.h>

int setns(int fd, int nstype);

// 示例
int fd = open("/proc/1234/ns/net", O_RDONLY);
setns(fd, CLONE_NEWNET);
```

### Python 示例

```python
#!/usr/bin/env python3
import os
import subprocess

# 创建新的 PID Namespace
pid = os.fork()

if pid == 0:
    # 子进程
    # 这里需要 C 扩展来调用 unshare
    # Python 标准库不直接支持

    subprocess.run(['/bin/bash'])
else:
    # 父进程
    os.waitpid(pid, 0)
```

### Go 示例

```go
package main

import (
    "os"
    "os/exec"
    "syscall"
)

func main() {
    cmd := exec.Command("/bin/bash")

    // 设置 Namespace
    cmd.SysProcAttr = &syscall.SysProcAttr{
        Cloneflags: syscall.CLONE_NEWUTS |
                    syscall.CLONE_NEWPID |
                    syscall.CLONE_NEWNS,
    }

    cmd.Stdin = os.Stdin
    cmd.Stdout = os.Stdout
    cmd.Stderr = os.Stderr

    if err := cmd.Run(); err != nil {
        panic(err)
    }
}

// 编译运行:
// go build -o container container.go
// sudo ./container
```

---

## 实用工具

### 1. nsenter - 进入 Namespace

```bash
# 进入容器的 Namespace
docker inspect --format '{{.State.Pid}}' mycontainer
# 假设输出: 1234

# 进入所有 Namespace
sudo nsenter -t 1234 -a /bin/bash

# 只进入网络 Namespace
sudo nsenter -t 1234 -n ip addr show

# 只进入 PID Namespace
sudo nsenter -t 1234 -p ps aux
```

### 2. lsns - 列出 Namespace

```bash
# 列出所有 Namespace
lsns

# 输出示例:
#        NS TYPE   NPROCS   PID USER   COMMAND
# 4026531835 cgroup    150     1 root   /sbin/init
# 4026531836 pid       150     1 root   /sbin/init
# 4026531837 user      150     1 root   /sbin/init

# 列出特定类型的 Namespace
lsns -t net

# 列出特定进程的 Namespace
lsns -p 1234
```

### 3. ip netns - 管理 Network Namespace

```bash
# 创建
ip netns add myns

# 列出
ip netns list

# 执行命令
ip netns exec myns ip addr show

# 删除
ip netns del myns
```

---

## 常见问题

### Q1: Namespace 和 Cgroups 的区别？
**A**:
- Namespace: 隔离（让进程看不到其他资源）
- Cgroups: 限制（限制进程能使用多少资源）

### Q2: 如何查看进程在哪个 Namespace？
**A**:
```bash
ls -l /proc/<PID>/ns/
# 相同的 Namespace ID 表示在同一个 Namespace
```

### Q3: 可以动态加入 Namespace 吗？
**A**: 可以，使用 setns() 系统调用或 nsenter 命令。

### Q4: Namespace 影响性能吗？
**A**: 影响极小，几乎可以忽略。主要开销在创建时。

---

## 下一步

学完 Namespace，继续学习 [02-Cgroups详解](./02-Cgroups详解.md)，理解如何限制容器资源！

---

*更新日期: 2025-12-02*
