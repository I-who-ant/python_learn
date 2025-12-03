# 03 - Chroot 详解

> Chroot 是文件系统隔离的基础，容器的根文件系统就基于此技术

## 什么是 Chroot

**chroot** (change root) 可以改变进程的根目录，让进程"认为"指定目录就是整个文件系统的根目录 `/`。

```bash
# 普通进程看到的根目录
ls /
# bin  boot  dev  etc  home  lib  ...

# chroot 后看到的根目录
chroot /my/new/root ls /
# 只能看到 /my/new/root 下的内容
```

## 基本用法

```bash
# 1. 准备 rootfs
mkdir -p /tmp/mychroot/{bin,lib,lib64}

# 2. 复制必要的二进制文件
cp /bin/bash /tmp/mychroot/bin/
cp /bin/ls /tmp/mychroot/bin/

# 3. 复制依赖库
ldd /bin/bash | grep -o '/lib.*\.[^ ]*' | xargs -I {} cp {} /tmp/mychroot/lib/
ldd /bin/ls | grep -o '/lib.*\.[^ ]*' | xargs -I {} cp {} /tmp/mychroot/lib/

# 4. 切换根目录
sudo chroot /tmp/mychroot /bin/bash

# 5. 在新根目录中
pwd  # /
ls /  # bin lib lib64
ls /home  # 失败（看不到原来的 /home）
```

## 容器中的应用

Docker 容器使用类似 chroot 的技术（实际使用 pivot_root）来隔离文件系统。

```bash
# Docker 容器的 rootfs
docker export mycontainer | tar -C /tmp/container_fs -xf -

# 查看容器的文件系统
ls /tmp/container_fs/
# bin  etc  home  lib  ...

# 这就是容器"看到"的根目录
```

## 安全性

chroot 不能防止 root 用户逃逸！

```bash
# 逃逸示例（需要 root）
mkdir /tmp/breakout
cd /tmp/breakout
mkdir -p new_root
sudo chroot new_root /bin/sh
```

**容器使用组合技术**：chroot + Namespace + Cgroups + capabilities 限制

## 下一步

学习 [04-UnionFS详解](./04-UnionFS详解.md) - 容器镜像的分层存储！
