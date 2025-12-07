# 04 - UnionFS详解

> 容器镜像的分层存储原理:深入理解 UnionFS、OverlayFS 和 Docker 镜像系统

## 目录

- [什么是 UnionFS](#什么是-unionfs)
- [UnionFS 实现方式](#unionfs-实现方式)
- [OverlayFS 详解](#overlayfs-详解)
- [AUFS vs OverlayFS](#aufs-vs-overlayfs)
- [Docker 镜像分层原理](#docker-镜像分层原理)
- [分层存储的优势](#分层存储的优势)
- [实战案例](#实战案例)

---

## 什么是 UnionFS

### 核心概念

【本文档是容器技术学习系列的第 04 篇】

> **UnionFS = Union File System (联合文件系统)**
>
> 将多个目录联合挂载到一个挂载点,形成一个统一的文件系统视图

```
┌─────────────────────────────────────┐
│         统一视图 (Merged)            │
│  ┌──────┐ ┌──────┐ ┌──────┐        │
│  │ app/ │ │ lib/ │ │ etc/ │        │
│  └──────┘ └──────┘ └──────┘        │
└─────────────────────────────────────┘
           ↑       ↑       ↑
           │       │       │
    ┌──────┴───────┴───────┴──────┐
    │                               │
┌───▼─────┐  ┌────▼────┐  ┌───▼─────┐
│ Layer 3 │  │ Layer 2 │  │ Layer 1 │
│ (Read)  │  │ (Read)  │  │(R/W)上层│
└─────────┘  └─────────┘  └─────────┘
   应用层      依赖层        基础层
```

### 为什么容器需要 UnionFS

1. **镜像复用** - 多个容器共享相同的基础镜像层
2. **快速启动** - 无需复制整个文件系统
3. **节省空间** - 相同的层只存储一次
4. **写时复制** - 只有修改时才复制文件

### UnionFS 核心特性

```
特性 1: 分层 (Layering)
  └─ 多个只读层 + 1个可写层

特性 2: 联合挂载 (Union Mount)
  └─ 多个目录合并为一个视图

特性 3: 写时复制 (Copy-on-Write)
  └─ 修改文件时,从只读层复制到可写层

特性 4: 白标记 (Whiteout)
  └─ 删除文件时,创建特殊标记而非真删除
```

---

## UnionFS 实现方式

### Linux 中的 UnionFS 实现

| 实现       | 特点                           | 使用场景                    |
|-----------|--------------------------------|----------------------------|
| **OverlayFS** | 内核原生,性能好,主流          | Docker 默认(CentOS 7+)      |
| **AUFS**      | 老牌方案,功能丰富             | Docker 早期版本(Ubuntu)     |
| **Btrfs**     | 文件系统级别,支持快照          | 特定场景                    |
| **ZFS**       | 企业级,功能强大               | 高级存储需求                |
| **Device Mapper** | 块设备级别                | Docker 早期(CentOS 6)       |

### 查看系统支持

```bash
# 查看内核支持的文件系统
cat /proc/filesystems | grep overlay
# overlay

# 查看已加载的模块
lsmod | grep overlay
# overlay  151552  0

# 查看 Docker 使用的存储驱动
docker info | grep "Storage Driver"
# Storage Driver: overlay2
```

---

## OverlayFS 详解

### OverlayFS 架构

```
┌─────────────────────────────────────────┐
│         merged (联合挂载点)              │
│  用户看到的统一文件系统视图              │
└─────────────────────────────────────────┘
                  ↑
                  │ 联合挂载
                  │
    ┌─────────────┼─────────────┐
    │             │             │
┌───▼──┐    ┌────▼────┐    ┌──▼────┐
│upper │    │ lower   │    │ work  │
│(可写)│    │(只读)   │    │(临时) │
└──────┘    └─────────┘    └───────┘
  修改层      基础层         工作层
```

### OverlayFS 核心概念

1. **lowerdir** (只读层)
   - 可以是一个或多个目录
   - 只读,不会被修改
   - 底层基础文件系统

2. **upperdir** (可写层)
   - 只能有一个
   - 所有修改都写入这里
   - 存储容器的变更

3. **workdir** (工作目录)
   - OverlayFS 内部使用
   - 必须与 upperdir 在同一文件系统
   - 用于原子操作

4. **merged** (挂载点)
   - 用户看到的统一视图
   - 合并 lower + upper

### OverlayFS 基础实验

```bash
# 1. 准备目录结构
mkdir -p /tmp/overlay/{lower,upper,work,merged}

# 2. 在 lower 层创建文件
echo "I am from lower layer" > /tmp/overlay/lower/file1.txt
echo "Base config" > /tmp/overlay/lower/config.txt

# 3. 查看 lower 层
ls -la /tmp/overlay/lower/
# file1.txt  config.txt

# 4. 挂载 OverlayFS
sudo mount -t overlay overlay \
  -o lowerdir=/tmp/overlay/lower,\
upperdir=/tmp/overlay/upper,\
workdir=/tmp/overlay/work \
  /tmp/overlay/merged

# 5. 查看合并后的视图
ls -la /tmp/overlay/merged/
# file1.txt  config.txt (来自 lower 层)

# 6. 读取文件
cat /tmp/overlay/merged/file1.txt
# I am from lower layer

# 7. 修改文件 (触发 Copy-on-Write)
echo "Modified!" > /tmp/overlay/merged/file1.txt

# 8. 查看各层的变化
cat /tmp/overlay/lower/file1.txt
# I am from lower layer (原文件未变)

cat /tmp/overlay/upper/file1.txt
# Modified! (修改保存在 upper 层)

cat /tmp/overlay/merged/file1.txt
# Modified! (merged 显示 upper 层的版本)

# 9. 创建新文件
echo "New file" > /tmp/overlay/merged/newfile.txt

ls /tmp/overlay/upper/
# file1.txt  newfile.txt (新文件在 upper 层)

ls /tmp/overlay/lower/
# file1.txt  config.txt (lower 层不变)

# 10. 删除文件 (Whiteout 机制)
rm /tmp/overlay/merged/config.txt

# 查看 upper 层
ls -la /tmp/overlay/upper/
# c--------- 1 root root 0, 0 Dec  3 10:00 config.txt

# 注意:在 upper 层创建了一个字符设备 (主设备号 0, 从设备号 0)
# 这就是 "whiteout" 文件,表示该文件被删除

# 11. 卸载
sudo umount /tmp/overlay/merged
```

### OverlayFS 文件操作详解

#### 读文件

```
1. 用户读取 merged/file.txt
2. 检查 upper 层是否存在
   ├─ 存在 → 返回 upper/file.txt
   └─ 不存在 → 返回 lower/file.txt
```

#### 写文件 (Copy-on-Write)

```
1. 用户修改 merged/file.txt
2. 检查 upper 层是否存在
   ├─ 存在 → 直接修改 upper/file.txt
   └─ 不存在:
      ├─ 从 lower 层复制到 upper 层
      └─ 修改 upper/file.txt
```

#### 删除文件 (Whiteout)

```
1. 用户删除 merged/file.txt
2. 检查文件位置
   ├─ 仅在 upper → 直接删除
   └─ 在 lower → 在 upper 创建 whiteout 标记
      (字符设备,主/从设备号均为 0)
```

### OverlayFS 多层挂载

```bash
# OverlayFS 支持多个 lowerdir (用 : 分隔)
# 右边的优先级高于左边

mkdir -p /tmp/multi/{lower1,lower2,lower3,upper,work,merged}

# 创建不同层的文件
echo "Layer 1" > /tmp/multi/lower1/file.txt
echo "Layer 2" > /tmp/multi/lower2/file.txt
echo "Only in Layer 2" > /tmp/multi/lower2/special.txt

# 多层挂载 (lower3:lower2:lower1,从右向左优先级递减)
sudo mount -t overlay overlay \
  -o lowerdir=/tmp/multi/lower3:/tmp/multi/lower2:/tmp/multi/lower1,\
upperdir=/tmp/multi/upper,\
workdir=/tmp/multi/work \
  /tmp/multi/merged

# 查看文件 (会读取最上层的 lower2 版本)
cat /tmp/multi/merged/file.txt
# Layer 2

# 清理
sudo umount /tmp/multi/merged
```

---

## AUFS vs OverlayFS

### 对比表

| 特性           | OverlayFS                | AUFS                     |
|---------------|--------------------------|--------------------------|
| **内核支持**   | 内核原生 (3.18+)          | 需要内核补丁             |
| **性能**       | 优秀                     | 良好                     |
| **复杂度**     | 简单                     | 复杂                     |
| **层数限制**   | 理论无限制(实践 ~128)     | 最多 127 层              |
| **维护状态**   | 活跃                     | 停滞                     |
| **Docker 支持**| 默认 (overlay2)          | 遗留支持 (aufs)          |

### 为什么 OverlayFS 取代了 AUFS

1. **内核原生** - 无需额外补丁
2. **性能更好** - 更少的系统调用
3. **更简单** - 代码量更少,更易维护
4. **更现代** - 持续优化和改进

---

## Docker 镜像分层原理

### Docker 镜像结构

```
Docker 镜像 = 基础层 + N个中间层 + 容器层

┌─────────────────────────────────────┐
│    Container Layer (可读写)         │ ← docker run 创建
├─────────────────────────────────────┤
│    Layer N: ADD app.py /app/        │ ← Dockerfile 每行指令
├─────────────────────────────────────┤
│    Layer 3: RUN pip install flask   │
├─────────────────────────────────────┤
│    Layer 2: RUN apt install python3 │
├─────────────────────────────────────┤
│    Layer 1: FROM ubuntu:20.04       │ ← 基础镜像
└─────────────────────────────────────┘
```

### 查看镜像分层

```bash
# 1. 拉取镜像
docker pull nginx:latest

# 输出:
# Using default tag: latest
# latest: Pulling from library/nginx
# a2abf6c4d29d: Pull complete  ← Layer 1
# a9edb18cadd1: Pull complete  ← Layer 2
# 589b7251471a: Pull complete  ← Layer 3
# ...

# 2. 查看镜像历史 (分层信息)
docker history nginx:latest

# 输出:
# IMAGE          CREATED        CREATED BY                                      SIZE
# 605c77e624dd   2 weeks ago    /bin/sh -c #(nop)  CMD ["nginx" "-g" "daemon…   0B
# <missing>      2 weeks ago    /bin/sh -c #(nop)  EXPOSE 80                    0B
# <missing>      2 weeks ago    /bin/sh -c #(nop) COPY file:xxx in /           4.62kB
# <missing>      2 weeks ago    /bin/sh -c apt-get update && apt-get install…   61.1MB
# ...

# 3. 查看镜像详细信息
docker inspect nginx:latest | grep -A 20 "GraphDriver"

# 输出:
# "GraphDriver": {
#     "Data": {
#         "LowerDir": "/var/lib/docker/overlay2/xxx/diff:
#                      /var/lib/docker/overlay2/yyy/diff:
#                      /var/lib/docker/overlay2/zzz/diff",
#         "MergedDir": "/var/lib/docker/overlay2/merged",
#         "UpperDir": "/var/lib/docker/overlay2/upper",
#         "WorkDir": "/var/lib/docker/overlay2/work"
#     },
#     "Name": "overlay2"
# }

# 4. 查看实际文件位置
sudo ls -la /var/lib/docker/overlay2/

# 5. 查看某一层的内容
sudo ls -la /var/lib/docker/overlay2/<layer-id>/diff/
```

### Dockerfile 与镜像层

```dockerfile
# 每个指令创建一层

FROM ubuntu:20.04              # Layer 1: 基础镜像
RUN apt update                 # Layer 2: 包更新
RUN apt install -y python3     # Layer 3: 安装 Python
COPY app.py /app/              # Layer 4: 复制文件
RUN pip3 install flask         # Layer 5: 安装依赖
CMD ["python3", "/app/app.py"] # Layer 6: 启动命令(元数据,不占空间)
```

**优化建议**:

```dockerfile
# ❌ 不好:每个 RUN 创建一层
RUN apt update
RUN apt install -y python3
RUN apt install -y pip
RUN apt clean

# ✅ 好:合并命令,减少层数
RUN apt update && \
    apt install -y python3 pip && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*
```

### 容器层 vs 镜像层

```
镜像层 (Image Layers):
  ├─ 只读
  ├─ 可被多个容器共享
  └─ 不可修改

容器层 (Container Layer):
  ├─ 可读写
  ├─ 每个容器独有
  ├─ 存储运行时变更
  └─ 容器删除时,该层也删除
```

```bash
# 运行容器
docker run -d --name web1 nginx
docker run -d --name web2 nginx

# 两个容器共享相同的镜像层(只读)
# 但有各自独立的容器层(可写)

# 在 web1 中修改文件
docker exec web1 sh -c "echo 'Modified' > /usr/share/nginx/html/index.html"

# web1 看到修改
docker exec web1 cat /usr/share/nginx/html/index.html
# Modified

# web2 不受影响
docker exec web2 cat /usr/share/nginx/html/index.html
# (原始内容)

# 查看容器的可写层大小
docker ps -s
# SIZE 列显示容器层大小
```

---

## 分层存储的优势

### 1. 空间节省

```
场景:运行 10 个 Ubuntu 容器

传统方式:
  10 × 300MB = 3GB

分层方式:
  基础层 300MB (共享)
  + 10 × 容器层 (每个几 KB)
  ≈ 300MB + 1MB
```

```bash
# 查看镜像占用空间
docker images
# ubuntu  20.04  300MB

# 运行 3 个容器
docker run -d --name c1 ubuntu sleep 1000
docker run -d --name c2 ubuntu sleep 1000
docker run -d --name c3 ubuntu sleep 1000

# 查看实际磁盘使用
docker system df
# Images:      1         300MB
# Containers:  3         0B (几乎为 0)
```

### 2. 快速部署

```
传统虚拟机:
  复制整个磁盘镜像 → 数分钟

容器:
  只需挂载已有的层 → 秒级启动
```

### 3. 增量更新

```
更新应用:
  只需下载变更的层,不需要重新下载基础层

FROM ubuntu:20.04           # 已有,跳过
RUN apt install python3     # 已有,跳过
COPY app.py /app/           # 新层,下载 (几 KB)
```

```bash
# 第一次 pull
docker pull myapp:v1
# Downloading layers 1-5... (300MB)

# 更新到 v2 (只修改了应用代码)
docker pull myapp:v2
# Layers 1-4: Already exists
# Downloading layer 5... (10KB)
```

### 4. 版本回滚

```bash
# 镜像就像 Git 的 commit
docker tag myapp:v2 myapp:latest
docker tag myapp:v1 myapp:rollback

# 回滚只需切换标签
docker stop app
docker run -d --name app myapp:rollback
```

---

## 实战案例

### 案例 1: 查看容器的文件系统变更

```bash
# 1. 运行容器
docker run -d --name test ubuntu:20.04 sleep 1000

# 2. 在容器中创建文件
docker exec test sh -c "echo 'test' > /tmp/myfile.txt"
docker exec test sh -c "apt update && apt install -y curl"

# 3. 查看容器层的变更
docker diff test

# 输出:
# A /tmp/myfile.txt           ← A = Added (新增)
# C /var                       ← C = Changed (修改)
# C /var/lib
# C /var/lib/apt
# A /usr/bin/curl              ← 新增的二进制
# ...

# 4. 查看容器层大小
docker ps -s --filter name=test
# SIZE: 50MB (容器层实际占用)
# VIRTUAL SIZE: 350MB (镜像 + 容器层)
```

### 案例 2: 手动构建分层文件系统

```bash
#!/bin/bash
# 模拟 Docker 镜像分层

# 1. 准备层目录
mkdir -p /tmp/docker-layers/{layer1,layer2,layer3,container,work,merged}

# 2. Layer 1: 基础系统文件
mkdir -p /tmp/docker-layers/layer1/{bin,lib,etc}
echo "Base system" > /tmp/docker-layers/layer1/etc/os-release
cp /bin/bash /tmp/docker-layers/layer1/bin/

# 3. Layer 2: 应用依赖
mkdir -p /tmp/docker-layers/layer2/usr/lib
echo "Python runtime" > /tmp/docker-layers/layer2/usr/lib/python.so

# 4. Layer 3: 应用代码
mkdir -p /tmp/docker-layers/layer3/app
cat > /tmp/docker-layers/layer3/app/main.py << 'EOF'
print("Hello from container!")
EOF

# 5. 挂载 OverlayFS (模拟容器启动)
sudo mount -t overlay overlay \
  -o lowerdir=/tmp/docker-layers/layer3:/tmp/docker-layers/layer2:/tmp/docker-layers/layer1,\
upperdir=/tmp/docker-layers/container,\
workdir=/tmp/docker-layers/work \
  /tmp/docker-layers/merged

# 6. 查看合并后的文件系统
ls -la /tmp/docker-layers/merged/
# bin/  lib/  etc/  usr/  app/  (所有层的内容)

# 7. 模拟容器运行时修改
echo "Runtime data" > /tmp/docker-layers/merged/app/data.txt

# 8. 查看各层
ls /tmp/docker-layers/layer3/app/
# main.py (应用代码,只读)

ls /tmp/docker-layers/container/app/
# data.txt (运行时数据,在容器层)

# 9. 清理
sudo umount /tmp/docker-layers/merged
```

### 案例 3: 优化 Docker 镜像大小

```dockerfile
# ❌ 不好的 Dockerfile (300MB)
FROM ubuntu:20.04
RUN apt update
RUN apt install -y python3 python3-pip
RUN pip3 install flask requests numpy
COPY app.py /app/
CMD ["python3", "/app/app.py"]

# ✅ 优化后的 Dockerfile (50MB)
FROM python:3.9-alpine  # 使用更小的基础镜像
RUN pip install --no-cache-dir flask requests numpy  # 合并层,不缓存
COPY app.py /app/
CMD ["python3", "/app/app.py"]

# 🚀 多阶段构建 (20MB)
# 阶段 1: 构建
FROM python:3.9 AS builder
WORKDIR /app
COPY requirements.txt .
RUN pip install --user --no-cache-dir -r requirements.txt

# 阶段 2: 运行时
FROM python:3.9-alpine
WORKDIR /app
COPY --from=builder /root/.local /root/.local  # 只复制需要的文件
COPY app.py .
CMD ["python3", "app.py"]
```

```bash
# 构建并查看大小
docker build -t app:bad -f Dockerfile.bad .
docker build -t app:good -f Dockerfile.good .
docker build -t app:best -f Dockerfile.best .

docker images | grep app
# app   bad    300MB
# app   good   50MB
# app   best   20MB
```

### 案例 4: 调试镜像分层

```bash
# 1. 使用 dive 工具分析镜像
# 安装 dive
wget https://github.com/wagoodman/dive/releases/download/v0.11.0/dive_0.11.0_linux_amd64.deb
sudo dpkg -i dive_0.11.0_linux_amd64.deb

# 2. 分析镜像
dive nginx:latest

# 输出:
# - 每层的大小
# - 每层添加/删除的文件
# - 浪费的空间
# - 优化建议

# 3. 或使用 docker history 查看详细信息
docker history --no-trunc nginx:latest > nginx-layers.txt
cat nginx-layers.txt
```

---

## 常见问题

### Q1: 为什么删除文件后镜像体积不变?

```dockerfile
# ❌ 错误示例
FROM ubuntu:20.04
RUN apt update && apt install -y build-essential  # Layer 1: +300MB
RUN apt remove build-essential                     # Layer 2: 创建 whiteout,不释放空间
```

**原因**: 文件在 Layer 1 中,Layer 2 只是标记删除,实际文件仍占用空间

**解决**:
```dockerfile
# ✅ 正确做法:在同一层删除
FROM ubuntu:20.04
RUN apt update && \
    apt install -y build-essential && \
    # ... 使用工具编译 ... && \
    apt remove -y build-essential && \
    apt autoremove -y && \
    rm -rf /var/lib/apt/lists/*
```

### Q2: 容器层数据会丢失吗?

容器停止后,容器层仍然存在:
```bash
docker run --name test nginx
docker stop test
# 容器层数据保留

docker start test
# 数据仍在

docker rm test
# 此时容器层才被删除
```

持久化数据使用 Volume:
```bash
docker run -v /host/data:/container/data nginx
```

### Q3: 如何减少镜像层数?

```dockerfile
# ❌ 6 层
FROM ubuntu
RUN apt update
RUN apt install python3
RUN apt install pip
COPY app.py /app/
CMD ["python3", "/app/app.py"]

# ✅ 3 层
FROM ubuntu
RUN apt update && apt install -y python3 pip && rm -rf /var/lib/apt/lists/*
COPY app.py /app/
CMD ["python3", "/app/app.py"]
```

---

## 总结

### 核心知识点

1. **UnionFS** - 联合文件系统,多层合并为一个视图
2. **OverlayFS** - Linux 内核原生实现,性能优秀
3. **Copy-on-Write** - 修改时才复制,节省空间
4. **Whiteout** - 删除标记机制
5. **分层存储** - 节省空间、快速部署、版本控制

### OverlayFS 文件操作流程

```
读文件: merged → upper (存在?) → lower
写文件: merged → CoW(lower→upper) → 修改 upper
删文件: merged → 检查位置 → upper 创建 whiteout
```

### Docker 镜像最佳实践

1. ✅ 使用小体积基础镜像 (alpine, slim)
2. ✅ 合并 RUN 指令,减少层数
3. ✅ 同一层内清理临时文件
4. ✅ 使用 .dockerignore 排除无关文件
5. ✅ 利用多阶段构建分离编译和运行环境
6. ✅ 使用构建缓存加速构建

### 下一步

继续学习 [05-容器网络详解](./05-容器网络详解.md) - 了解容器如何通信!

---

## 参考资源

- [OverlayFS 官方文档](https://www.kernel.org/doc/html/latest/filesystems/overlayfs.html)
- [Docker 存储驱动](https://docs.docker.com/storage/storagedriver/)
- [Docker 镜像层缓存](https://docs.docker.com/build/cache/)
- [Dive - 镜像分析工具](https://github.com/wagoodman/dive)

---

*更新日期: 2025-12-03*
*作者: Claude Code*
