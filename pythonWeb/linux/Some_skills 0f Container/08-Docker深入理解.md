# 08 - Docker深入理解

> 工业级容器平台深度解析:架构、镜像、网络、存储和最佳实践

## 目录

- [Docker 架构深度解析](#docker-架构深度解析)
- [Docker 镜像深入](#docker-镜像深入)
- [Docker 网络详解](#docker-网络详解)
- [Docker 存储详解](#docker-存储详解)
- [Dockerfile 最佳实践](#dockerfile-最佳实践)
- [Docker Compose 实战](#docker-compose-实战)
- [Docker 安全](#docker-安全)
- [性能优化](#性能优化)

---

## Docker 架构深度解析

【本文档是容器技术学习系列的第 08 篇】

### Docker 整体架构

```
┌─────────────────────────────────────────────────────┐
│                   Docker 生态系统                    │
├─────────────────────────────────────────────────────┤
│                                                      │
│  ┌──────────────┐      ┌──────────────┐           │
│  │ Docker Client│◄────►│ Docker Hub   │           │
│  │  (docker)    │      │  (Registry)  │           │
│  └──────┬───────┘      └──────────────┘           │
│         │                                           │
│         │ REST API                                  │
│         ▼                                           │
│  ┌──────────────────────────────────────┐         │
│  │       Docker Daemon (dockerd)        │         │
│  ├──────────────────────────────────────┤         │
│  │  • Image Management                  │         │
│  │  • Container Lifecycle               │         │
│  │  • Network Management                │         │
│  │  • Volume Management                 │         │
│  └──────────────┬───────────────────────┘         │
│                 │                                   │
│                 ▼                                   │
│  ┌──────────────────────────────────────┐         │
│  │         containerd                    │         │
│  │  • Container Supervision             │         │
│  │  • Image Transfer                    │         │
│  └──────────────┬───────────────────────┘         │
│                 │                                   │
│                 ▼                                   │
│  ┌──────────────────────────────────────┐         │
│  │  containerd-shim (per container)     │         │
│  └──────────────┬───────────────────────┘         │
│                 │                                   │
│                 ▼                                   │
│  ┌──────────────────────────────────────┐         │
│  │           runc                        │         │
│  │  • Create namespaces                 │         │
│  │  • Setup cgroups                     │         │
│  │  • Start container                   │         │
│  └──────────────────────────────────────┘         │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### Docker 组件详解

**1. Docker Client (docker)**
```bash
# 用户界面,发送命令到 daemon
docker run nginx
docker ps
docker stop container_id

# 客户端配置
cat ~/.docker/config.json
```

**2. Docker Daemon (dockerd)**
```bash
# 后台守护进程,核心管理组件
sudo dockerd --debug --log-level=debug

# 查看 daemon 配置
cat /etc/docker/daemon.json

# 示例配置
{
  "storage-driver": "overlay2",
  "log-driver": "json-file",
  "log-opts": {
    "max-size": "10m",
    "max-file": "3"
  },
  "registry-mirrors": [
    "https://registry.docker-cn.com"
  ]
}
```

**3. containerd**
```bash
# 容器生命周期管理
sudo systemctl status containerd

# 使用 ctr 直接与 containerd 交互
sudo ctr containers ls
sudo ctr images ls
```

**4. runc**
```bash
# OCI 运行时,实际创建容器
which runc
# /usr/bin/runc

runc --version
# runc version 1.1.10
```

### Docker 工作流程

```
┌────────────────────────────────────────────────────┐
│  docker run nginx:latest                           │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  1. Client 解析命令,发送 REST API 请求到 daemon   │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  2. Daemon 检查本地是否有镜像                      │
│     - 有:跳到步骤 3                                │
│     - 无:从 Registry 拉取                          │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  3. Daemon 调用 containerd 创建容器                │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  4. containerd 启动 containerd-shim                │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  5. shim 调用 runc 创建容器                        │
│     - 创建 namespace                                │
│     - 配置 cgroups                                  │
│     - 挂载 rootfs                                   │
│     - 配置网络                                      │
└────────────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────────────┐
│  6. runc 启动容器进程后退出                        │
│     shim 继续管理容器                              │
└────────────────────────────────────────────────────┘
```

---

## Docker 镜像深入

### 镜像命名规范

```
完整格式:
registry/repository:tag

示例:
docker.io/library/nginx:1.21-alpine
│         │       │     │    └─ 镜像标签
│         │       │     └─ 镜像名称
│         │       └─ 仓库路径
│         └─ 命名空间
└─ 镜像仓库地址

简写:
nginx:latest  →  docker.io/library/nginx:latest
myapp         →  docker.io/library/myapp:latest
```

### 镜像操作进阶

```bash
# ============ 镜像拉取 ============

# 1. 拉取镜像
docker pull nginx:1.21

# 2. 拉取所有标签
docker pull --all-tags nginx

# 3. 拉取特定平台镜像
docker pull --platform linux/arm64 nginx

# 4. 使用镜像摘要拉取(不可变)
docker pull nginx@sha256:abc123...


# ============ 镜像查看 ============

# 1. 列出镜像
docker images
docker images -a    # 包含中间层
docker images -q    # 只显示 ID

# 2. 查看镜像详情
docker inspect nginx:latest

# 3. 查看镜像历史
docker history nginx:latest
docker history --no-trunc nginx  # 显示完整命令

# 4. 查看镜像分层
docker image inspect nginx:latest | jq '.[0].RootFS.Layers'


# ============ 镜像导入导出 ============

# 1. 导出镜像为 tar
docker save nginx:latest -o nginx.tar
docker save nginx:latest | gzip > nginx.tar.gz

# 2. 从 tar 导入镜像
docker load -i nginx.tar
docker load < nginx.tar.gz

# 3. 导出容器为镜像
docker export container_id > container.tar
docker import container.tar myimage:v1

# save vs export 的区别
# save:   保存镜像(包含历史和元数据)
# export: 导出容器(只有文件系统快照)


# ============ 镜像标签 ============

# 1. 创建标签
docker tag nginx:latest myregistry.com/nginx:v1.0

# 2. 推送到私有仓库
docker push myregistry.com/nginx:v1.0

# 3. 删除标签(不删除镜像层)
docker rmi myregistry.com/nginx:v1.0


# ============ 镜像清理 ============

# 1. 删除镜像
docker rmi nginx:latest

# 2. 删除悬空镜像(dangling)
docker image prune

# 3. 删除所有未使用镜像
docker image prune -a

# 4. 删除所有镜像
docker rmi $(docker images -q)

# 5. 强制删除
docker rmi -f $(docker images -q)
```

### 镜像构建优化

```dockerfile
# ============ 多阶段构建 ============
# 减少最终镜像大小

FROM golang:1.20 AS builder
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -o /myapp

# 最终镜像只包含编译结果
FROM alpine:latest
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=builder /myapp .
CMD ["./myapp"]

# 结果:
# - builder 阶段: 800MB (golang 镜像)
# - 最终镜像:    10MB  (alpine + 二进制)


# ============ 构建缓存优化 ============

# ❌ 错误:每次都重新安装依赖
FROM node:18
WORKDIR /app
COPY . .                      # 任何文件变化都导致后续重建
RUN npm install               # 频繁重建
CMD ["npm", "start"]

# ✅ 正确:利用缓存层
FROM node:18
WORKDIR /app
COPY package*.json ./         # 只在依赖变化时重建
RUN npm install               # 被缓存
COPY . .                      # 代码变化不影响上层
CMD ["npm", "start"]


# ============ .dockerignore ============
# 减少构建上下文大小

# .dockerignore 文件内容:
node_modules/
npm-debug.log
.git/
.gitignore
README.md
.env
*.md
.vscode/
.idea/
*.log


# ============ 基础镜像选择 ============

# 大小对比:
ubuntu:20.04        →  72MB
debian:slim         →  80MB
alpine:latest       →  5MB
scratch             →  0MB (空镜像)

# 选择建议:
# - 开发环境: ubuntu/debian (工具完整)
# - 生产环境: alpine (体积小)
# - 静态编译: scratch (最小化)


# ============ BuildKit 加速 ============
# Docker 新构建引擎

# 启用 BuildKit
export DOCKER_BUILDKIT=1

# 或永久启用
echo '{"features": {"buildkit": true}}' > ~/.docker/config.json

# BuildKit 特性:
# - 并行构建
# - 构建缓存优化
# - 秘密挂载
# - SSH 转发

# 示例:挂载缓存
# syntax=docker/dockerfile:1
FROM node:18
WORKDIR /app
COPY package*.json ./
RUN --mount=type=cache,target=/root/.npm \
    npm install
```

---

## Docker 网络详解

### Docker 网络模式

```bash
# 查看网络类型
docker network ls
# NETWORK ID     NAME      DRIVER    SCOPE
# abc123         bridge    bridge    local
# def456         host      host      local
# ghi789         none      null      local

# 1. Bridge 模式(默认)
docker run -d --name web1 nginx
# 容器获得独立 IP,通过 NAT 访问外网

# 2. Host 模式
docker run -d --network host --name web2 nginx
# 容器共享宿主机网络栈,无端口映射

# 3. None 模式
docker run -d --network none --name web3 alpine
# 容器无网络,只有 lo

# 4. Container 模式
docker run -d --name web4 nginx
docker run -d --network container:web4 alpine
# 两容器共享网络栈

# 5. 自定义 Bridge
docker network create mynet
docker run -d --network mynet --name web5 nginx
```

### 自定义网络

```bash
# ============ 创建网络 ============

# 1. 创建简单网络
docker network create myapp-net

# 2. 创建指定子网的网络
docker network create \
  --driver bridge \
  --subnet 172.20.0.0/16 \
  --gateway 172.20.0.1 \
  myapp-net

# 3. 创建多子网网络
docker network create \
  --driver bridge \
  --subnet 172.20.0.0/16 \
  --subnet 192.168.50.0/24 \
  multi-net


# ============ 网络管理 ============

# 1. 查看网络详情
docker network inspect myapp-net

# 2. 连接容器到网络
docker network connect myapp-net web1

# 3. 断开连接
docker network disconnect myapp-net web1

# 4. 删除网络
docker network rm myapp-net

# 5. 清理未使用网络
docker network prune


# ============ 容器间通信 ============

# 创建网络和容器
docker network create app-tier

# 后端容器
docker run -d \
  --name backend \
  --network app-tier \
  myapp-backend

# 前端容器(可以直接用容器名访问后端)
docker run -d \
  --name frontend \
  --network app-tier \
  -e BACKEND_URL=http://backend:8080 \
  myapp-frontend

# 测试连通性
docker exec frontend ping backend
docker exec frontend curl http://backend:8080


# ============ 端口映射 ============

# 1. 映射单个端口
docker run -d -p 8080:80 nginx
# 宿主机 8080 → 容器 80

# 2. 映射所有端口
docker run -d -P nginx
# 随机端口 → 容器暴露的端口

# 3. 指定 IP 映射
docker run -d -p 127.0.0.1:8080:80 nginx
# 只在 localhost 监听

# 4. 映射 UDP 端口
docker run -d -p 53:53/udp dns-server

# 5. 映射多个端口
docker run -d \
  -p 80:80 \
  -p 443:443 \
  -p 8080:8080 \
  web-app

# 6. 查看端口映射
docker port web-app


# ============ DNS 配置 ============

# 1. 自定义 DNS
docker run -d \
  --dns 8.8.8.8 \
  --dns 8.8.4.4 \
  nginx

# 2. 添加 hosts 记录
docker run -d \
  --add-host mydb:192.168.1.100 \
  myapp

# 3. 自定义容器名解析
docker run -d \
  --name db \
  --network mynet \
  --network-alias database \
  postgres

docker run -d \
  --name app \
  --network mynet \
  -e DB_HOST=database \  # 可以解析到 db 容器
  myapp
```

### 网络故障排查

```bash
# 1. 查看容器网络配置
docker inspect web1 | grep -A 20 Networks

# 2. 进入容器网络命名空间
docker exec web1 ip addr
docker exec web1 ip route
docker exec web1 netstat -tlnp

# 3. 抓包调试
docker exec web1 tcpdump -i eth0 -n

# 4. 测试网络连通性
docker exec web1 ping 8.8.8.8
docker exec web1 curl https://google.com
docker exec web1 nslookup google.com

# 5. 查看 iptables 规则
sudo iptables -t nat -L -n -v | grep docker

# 6. 查看网桥信息
ip link show docker0
brctl show docker0

# 7. 查看网络流量
docker stats web1 --no-stream
```

---

## Docker 存储详解

### 存储驱动

```bash
# 查看存储驱动
docker info | grep "Storage Driver"
# Storage Driver: overlay2

# 存储驱动对比
# overlay2:  推荐,性能好,主流
# aufs:      老旧,Ubuntu 早期默认
# devicemapper: 块设备,性能一般
# btrfs:     文件系统级,支持快照
# zfs:       企业级,功能强大

# 查看存储位置
docker info | grep "Docker Root Dir"
# Docker Root Dir: /var/lib/docker

# 存储目录结构
ls -la /var/lib/docker/
# containers/  - 容器元数据
# image/       - 镜像层
# overlay2/    - overlay2 存储
# volumes/     - 数据卷
# network/     - 网络配置
```

### Volume (数据卷)

```bash
# ============ Volume 管理 ============

# 1. 创建 volume
docker volume create mydata

# 2. 列出 volumes
docker volume ls

# 3. 查看 volume 详情
docker volume inspect mydata
# {
#     "Driver": "local",
#     "Mountpoint": "/var/lib/docker/volumes/mydata/_data",
#     ...
# }

# 4. 删除 volume
docker volume rm mydata

# 5. 清理未使用 volumes
docker volume prune


# ============ 使用 Volume ============

# 1. 匿名 volume
docker run -d -v /data nginx
# Docker 自动创建 volume

# 2. 命名 volume
docker run -d \
  --name web \
  -v mydata:/usr/share/nginx/html \
  nginx

# 3. 只读 volume
docker run -d \
  -v mydata:/data:ro \
  nginx

# 4. 多个 volumes
docker run -d \
  -v data1:/data1 \
  -v data2:/data2 \
  myapp


# ============ Bind Mount (挂载目录) ============

# 1. 挂载宿主机目录
docker run -d \
  -v /host/path:/container/path \
  nginx

# 2. 只读挂载
docker run -d \
  -v /host/path:/container/path:ro \
  nginx

# 3. 挂载当前目录
docker run -d \
  -v $(pwd):/app \
  -w /app \
  node:18 \
  npm start


# ============ tmpfs Mount (内存存储) ============

# 1. 挂载 tmpfs
docker run -d \
  --tmpfs /tmp:rw,size=100m,mode=1777 \
  nginx

# 2. 使用场景
# - 临时文件
# - 敏感数据(不落盘)
# - 高性能缓存


# ============ Volume 驱动 ============

# 1. 使用 NFS volume
docker volume create \
  --driver local \
  --opt type=nfs \
  --opt o=addr=192.168.1.100,rw \
  --opt device=:/export/nfs \
  nfs-vol

# 2. 使用 CIFS/SMB
docker volume create \
  --driver local \
  --opt type=cifs \
  --opt o=username=user,password=pass \
  --opt device=//192.168.1.100/share \
  smb-vol


# ============ 数据备份与恢复 ============

# 1. 备份 volume
docker run --rm \
  -v mydata:/data \
  -v $(pwd):/backup \
  alpine \
  tar czf /backup/mydata-backup.tar.gz /data

# 2. 恢复 volume
docker run --rm \
  -v mydata:/data \
  -v $(pwd):/backup \
  alpine \
  tar xzf /backup/mydata-backup.tar.gz -C /
```

### 存储最佳实践

```bash
# ============ 性能优化 ============

# 1. 对于 I/O 密集型应用,使用 volume
docker run -d \
  -v dbdata:/var/lib/postgresql/data \
  postgres
# volume 性能 > bind mount

# 2. 开发环境使用 bind mount
docker run -d \
  -v $(pwd)/src:/app/src \
  myapp
# 代码实时同步

# 3. 临时数据使用 tmpfs
docker run -d \
  --tmpfs /tmp \
  myapp
# 内存存储,最快


# ============ 数据持久化策略 ============

# 1. 数据库容器
docker run -d \
  --name db \
  -v db-data:/var/lib/mysql \
  -e MYSQL_ROOT_PASSWORD=secret \
  mysql:8.0

# 2. 应用容器(无状态)
docker run -d \
  --name app \
  myapp
# 不使用 volume,应用无状态

# 3. 共享配置
docker volume create config
docker run -d \
  -v config:/etc/myapp \
  --name app1 \
  myapp
docker run -d \
  -v config:/etc/myapp:ro \
  --name app2 \
  myapp


# ============ 容器间共享数据 ============

# 1. 使用相同 volume
docker run -d --name writer -v shared:/data alpine
docker run -d --name reader -v shared:/data:ro alpine

# 2. 使用 volumes-from (已废弃,不推荐)
docker run -d --name data-container -v /data alpine
docker run -d --volumes-from data-container app


# ============ 清理策略 ============

# 1. 删除容器时删除匿名 volumes
docker run -d --rm -v /data nginx
# 容器删除时,volume 自动删除

# 2. 保留命名 volumes
docker run -d --name app -v mydata:/data app
docker rm app
# mydata volume 仍然保留

# 3. 定期清理
docker volume prune -f
# 删除未使用的 volumes
```

---

## Dockerfile 最佳实践

### 指令详解

```dockerfile
# ============ FROM ============
# 基础镜像

FROM node:18-alpine AS base
FROM scratch  # 空镜像,用于静态编译程序


# ============ ARG ============
# 构建时变量

ARG VERSION=1.0
ARG BUILD_DATE
FROM node:${VERSION}


# ============ ENV ============
# 运行时环境变量

ENV NODE_ENV=production \
    PORT=3000 \
    DB_HOST=localhost


# ============ WORKDIR ============
# 设置工作目录

WORKDIR /app
# 相当于 mkdir -p /app && cd /app


# ============ COPY vs ADD ============

# COPY: 推荐,简单文件复制
COPY package*.json ./
COPY src/ ./src/

# ADD: 高级功能,支持 URL 和自动解压
ADD https://example.com/file.tar.gz /tmp/
ADD archive.tar.gz /app/  # 自动解压


# ============ RUN ============
# 构建时执行命令

# ❌ 不好:多层
RUN apt-get update
RUN apt-get install -y python3
RUN apt-get clean

# ✅ 好:合并层
RUN apt-get update && \
    apt-get install -y \
        python3 \
        python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*


# ============ CMD vs ENTRYPOINT ============

# CMD: 可被覆盖
CMD ["nginx", "-g", "daemon off;"]
# docker run myimage /bin/sh  ← 覆盖 CMD

# ENTRYPOINT: 不可覆盖(除非 --entrypoint)
ENTRYPOINT ["nginx"]
CMD ["-g", "daemon off;"]  # 默认参数
# docker run myimage -t  ← 变成 nginx -t


# ============ EXPOSE ============
# 声明端口(仅文档作用)

EXPOSE 80 443


# ============ VOLUME ============
# 声明挂载点

VOLUME ["/data", "/logs"]


# ============ USER ============
# 切换用户

RUN adduser -D myuser
USER myuser
# 后续命令以 myuser 运行


# ============ HEALTHCHECK ============
# 健康检查

HEALTHCHECK --interval=30s --timeout=3s --retries=3 \
  CMD curl -f http://localhost/ || exit 1


# ============ LABEL ============
# 添加元数据

LABEL maintainer="user@example.com" \
      version="1.0" \
      description="My App"


# ============ ONBUILD ============
# 触发器(子镜像构建时执行)

ONBUILD COPY . /app
ONBUILD RUN npm install


# ============ SHELL ============
# 指定 shell

SHELL ["/bin/bash", "-c"]
RUN echo "Using bash"


# ============ STOPSIGNAL ============
# 停止信号

STOPSIGNAL SIGTERM
```

### 完整最佳实践示例

```dockerfile
# ============================================
# 多阶段构建 - Node.js 应用
# ============================================

# 语法版本(启用 BuildKit 特性)
# syntax=docker/dockerfile:1

# ============ 阶段 1: 依赖安装 ============
FROM node:18-alpine AS dependencies

WORKDIR /app

# 复制依赖文件
COPY package*.json ./

# 安装依赖(挂载缓存加速)
RUN --mount=type=cache,target=/root/.npm \
    npm ci --only=production


# ============ 阶段 2: 构建 ============
FROM node:18-alpine AS build

WORKDIR /app

# 复制源代码
COPY . .

# 安装所有依赖(包括devDependencies)
RUN --mount=type=cache,target=/root/.npm \
    npm ci

# 构建应用
RUN npm run build


# ============ 阶段 3: 生产镜像 ============
FROM node:18-alpine AS production

# 安全:使用非 root 用户
RUN addgroup -g 1001 -S nodejs && \
    adduser -S nodejs -u 1001

WORKDIR /app

# 复制生产依赖
COPY --from=dependencies --chown=nodejs:nodejs /app/node_modules ./node_modules

# 复制构建产物
COPY --from=build --chown=nodejs:nodejs /app/dist ./dist
COPY --from=build --chown=nodejs:nodejs /app/package.json ./

# 切换用户
USER nodejs

# 环境变量
ENV NODE_ENV=production \
    PORT=3000

# 暴露端口
EXPOSE 3000

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD node -e "require('http').get('http://localhost:3000/health', (r) => {process.exit(r.statusCode === 200 ? 0 : 1)})"

# 元数据
LABEL maintainer="devops@example.com" \
      version="1.0.0" \
      description="Production Node.js App"

# 启动命令
CMD ["node", "dist/index.js"]


# ============================================
# 结果:
# - 构建阶段: 800MB (包含dev工具)
# - 最终镜像: 150MB (只有运行时需要的)
# ============================================
```

### 构建技巧

```bash
# ============ 构建命令 ============

# 1. 基础构建
docker build -t myapp:v1.0 .

# 2. 指定 Dockerfile
docker build -t myapp -f Dockerfile.prod .

# 3. 构建参数
docker build \
  --build-arg VERSION=1.0 \
  --build-arg BUILD_DATE=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
  -t myapp .

# 4. 多平台构建
docker buildx build \
  --platform linux/amd64,linux/arm64 \
  -t myapp:multiarch \
  --push \
  .

# 5. 不使用缓存
docker build --no-cache -t myapp .

# 6. 构建特定阶段
docker build --target build -t myapp:builder .

# 7. 查看构建历史
docker history myapp:v1.0


# ============ .dockerignore ============

# .dockerignore 文件
node_modules/
npm-debug.log*
yarn-debug.log*
yarn-error.log*

.git/
.gitignore
.dockerignore

README.md
LICENSE
*.md

.env
.env.*

.vscode/
.idea/

dist/
build/
coverage/
.cache/

*.log
*.tmp
*.bak


# ============ 镜像优化 ============

# 1. 使用 dive 分析镜像
dive myapp:v1.0

# 2. 使用 docker-slim 瘦身
docker-slim build --target myapp:v1.0

# 3. 使用 hadolint 检查 Dockerfile
hadolint Dockerfile
```

---

## Docker Compose 实战

### Compose 文件详解

```yaml
# docker-compose.yml
version: '3.8'  # Compose 文件格式版本

services:
  # ============ Web 服务 ============
  web:
    image: nginx:alpine
    container_name: my-web
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./html:/usr/share/nginx/html:ro
      - ./conf/nginx.conf:/etc/nginx/nginx.conf:ro
      - web-logs:/var/log/nginx
    networks:
      - frontend
    depends_on:
      - app
    restart: unless-stopped
    environment:
      - TZ=Asia/Shanghai
    healthcheck:
      test: ["CMD", "wget", "--spider", "-q", "http://localhost"]
      interval: 30s
      timeout: 3s
      retries: 3
      start_period: 10s
    deploy:
      resources:
        limits:
          cpus: '0.5'
          memory: 512M
        reservations:
          cpus: '0.25'
          memory: 256M


  # ============ 应用服务 ============
  app:
    build:
      context: ./app
      dockerfile: Dockerfile
      args:
        - NODE_ENV=production
        - VERSION=1.0.0
      target: production
    image: myapp:latest
    ports:
      - "3000:3000"
    volumes:
      - app-data:/app/data
      - ./config:/app/config:ro
    networks:
      - frontend
      - backend
    depends_on:
      db:
        condition: service_healthy
      redis:
        condition: service_started
    environment:
      - NODE_ENV=production
      - DB_HOST=db
      - DB_PORT=5432
      - DB_NAME=mydb
      - DB_USER=user
      - DB_PASSWORD=password
      - REDIS_HOST=redis
      - REDIS_PORT=6379
    env_file:
      - .env
      - .env.production
    restart: always
    command: ["node", "dist/index.js"]
    logging:
      driver: "json-file"
      options:
        max-size: "10m"
        max-file: "3"


  # ============ 数据库服务 ============
  db:
    image: postgres:15-alpine
    container_name: postgres-db
    volumes:
      - db-data:/var/lib/postgresql/data
      - ./init.sql:/docker-entrypoint-initdb.d/init.sql:ro
    networks:
      - backend
    environment:
      - POSTGRES_USER=user
      - POSTGRES_PASSWORD=password
      - POSTGRES_DB=mydb
    restart: always
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U user -d mydb"]
      interval: 10s
      timeout: 3s
      retries: 3
    ports:
      - "5432:5432"


  # ============ Redis 缓存 ============
  redis:
    image: redis:7-alpine
    container_name: redis-cache
    command: redis-server --appendonly yes
    volumes:
      - redis-data:/data
    networks:
      - backend
    restart: always
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 10s
      timeout: 3s
      retries: 3


# ============ 网络定义 ============
networks:
  frontend:
    driver: bridge
    ipam:
      driver: default
      config:
        - subnet: 172.20.0.0/16
  backend:
    driver: bridge
    internal: true  # 内部网络,不能访问外网


# ============ 卷定义 ============
volumes:
  web-logs:
  app-data:
  db-data:
  redis-data:


# ============ 配置和密钥 ============
configs:
  nginx_config:
    file: ./conf/nginx.conf

secrets:
  db_password:
    file: ./secrets/db_password.txt
```

### Compose 命令

```bash
# ============ 启动和停止 ============

# 1. 启动所有服务
docker-compose up

# 2. 后台启动
docker-compose up -d

# 3. 启动特定服务
docker-compose up -d web app

# 4. 重新构建并启动
docker-compose up --build

# 5. 强制重建
docker-compose up --build --force-recreate

# 6. 停止服务
docker-compose stop

# 7. 停止并删除容器
docker-compose down

# 8. 停止并删除容器、网络、卷
docker-compose down -v


# ============ 服务管理 ============

# 1. 查看服务状态
docker-compose ps

# 2. 查看服务日志
docker-compose logs
docker-compose logs -f          # 实时日志
docker-compose logs -f web      # 特定服务
docker-compose logs --tail=100  # 最后100行

# 3. 进入容器
docker-compose exec web sh
docker-compose exec app bash

# 4. 运行一次性命令
docker-compose run --rm app npm install

# 5. 重启服务
docker-compose restart
docker-compose restart web

# 6. 暂停和恢复
docker-compose pause
docker-compose unpause


# ============ 扩容和负载均衡 ============

# 1. 扩容服务
docker-compose up -d --scale app=3

# 2. 查看扩容后的服务
docker-compose ps

# 3. 配置 nginx 负载均衡
# upstream backend {
#     server app_1:3000;
#     server app_2:3000;
#     server app_3:3000;
# }


# ============ 其他命令 ============

# 1. 验证配置文件
docker-compose config

# 2. 查看镜像
docker-compose images

# 3. 查看进程
docker-compose top

# 4. 查看服务事件
docker-compose events

# 5. 拉取镜像
docker-compose pull

# 6. 推送镜像
docker-compose push

# 7. 构建镜像
docker-compose build
docker-compose build --no-cache
```

### 实战案例:完整Web应用

```yaml
# docker-compose.yml - LNMP 架构

version: '3.8'

services:
  # Nginx
  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./www:/var/www/html:ro
      - ./nginx/conf.d:/etc/nginx/conf.d:ro
      - ./nginx/ssl:/etc/nginx/ssl:ro
      - nginx-logs:/var/log/nginx
    networks:
      - web
    depends_on:
      - php
    restart: unless-stopped

  # PHP-FPM
  php:
    build: ./php
    volumes:
      - ./www:/var/www/html
    networks:
      - web
      - database
    environment:
      - DB_HOST=mysql
      - DB_PORT=3306
      - DB_NAME=myapp
      - REDIS_HOST=redis
    restart: unless-stopped

  # MySQL
  mysql:
    image: mysql:8.0
    volumes:
      - mysql-data:/var/lib/mysql
      - ./mysql/init:/docker-entrypoint-initdb.d:ro
    environment:
      - MYSQL_ROOT_PASSWORD=rootpassword
      - MYSQL_DATABASE=myapp
      - MYSQL_USER=user
      - MYSQL_PASSWORD=password
    networks:
      - database
    command: --default-authentication-plugin=mysql_native_password
    restart: unless-stopped

  # Redis
  redis:
    image: redis:7-alpine
    volumes:
      - redis-data:/data
    networks:
      - database
    restart: unless-stopped

  # PHPMyAdmin (可选)
  phpmyadmin:
    image: phpmyadmin/phpmyadmin
    environment:
      - PMA_HOST=mysql
      - PMA_PORT=3306
    ports:
      - "8080:80"
    networks:
      - web
      - database
    depends_on:
      - mysql
    restart: unless-stopped

networks:
  web:
  database:
    internal: true

volumes:
  nginx-logs:
  mysql-data:
  redis-data:
```

---

## Docker 安全

### 安全最佳实践

```bash
# ============ 镜像安全 ============

# 1. 使用官方镜像
FROM node:18-alpine  # ✅
FROM someuser/node   # ❌

# 2. 指定镜像版本
FROM node:18.17.1-alpine  # ✅
FROM node:latest          # ❌ 不可预测

# 3. 最小化基础镜像
FROM alpine  # 5MB,攻击面小
FROM ubuntu  # 72MB,工具多

# 4. 扫描镜像漏洞
docker scan nginx:latest
trivy image nginx:latest
snyk container test nginx:latest


# ============ 用户权限 ============

# Dockerfile
FROM node:18-alpine

# 创建非 root 用户
RUN addgroup -g 1001 -S appuser && \
    adduser -S appuser -u 1001

WORKDIR /app
COPY --chown=appuser:appuser . .

# 切换用户
USER appuser

CMD ["node", "index.js"]


# ============ 运行时安全 ============

# 1. 只读根文件系统
docker run --read-only -v /tmp:/tmp nginx

# 2. 删除危险能力
docker run --cap-drop=ALL --cap-add=NET_BIND_SERVICE nginx

# 3. 禁止特权模式
docker run --privileged nginx  # ❌ 危险!

# 4. 限制资源
docker run \
  --memory="512m" \
  --cpus="0.5" \
  --pids-limit=100 \
  nginx

# 5. 使用安全选项
docker run \
  --security-opt=no-new-privileges:true \
  --security-opt=apparmor=docker-default \
  nginx

# 6. 设置 user namespace
dockerd --userns-remap=default


# ============ 网络安全 ============

# 1. 使用自定义网络
docker network create --internal private-net

# 2. 不暴露不必要的端口
docker run -p 127.0.0.1:3000:3000 myapp  # ✅ 只本机访问
docker run -p 3000:3000 myapp            # ❌ 所有接口

# 3. 使用 secrets 管理敏感信息
echo "db_password" | docker secret create db_pass -
docker service create \
  --secret db_pass \
  --env DB_PASSWORD_FILE=/run/secrets/db_pass \
  myapp


# ============ Dockerfile 安全 ============

# ❌ 不安全
FROM ubuntu
RUN apt-get update && apt-get install -y \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 暴露敏感信息
ENV DB_PASSWORD=secret123
COPY .env /app/

# 以 root 运行
CMD ["./app"]


# ✅ 安全
FROM ubuntu:22.04@sha256:abc123...  # 使用摘要

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget=1.21.3-1ubuntu1 \    # 固定版本
    curl=7.88.1-10ubuntu0.5 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 使用 secret 或 build-arg
ARG DB_PASSWORD
RUN --mount=type=secret,id=db_pass \
    DB_PASS=$(cat /run/secrets/db_pass) && \
    # 使用密码...

# 非 root 用户
USER nobody
CMD ["./app"]


# ============ 日志和审计 ============

# 1. 启用日志
docker run -d \
  --log-driver=json-file \
  --log-opt max-size=10m \
  --log-opt max-file=3 \
  nginx

# 2. 使用 Syslog
docker run -d \
  --log-driver=syslog \
  --log-opt syslog-address=tcp://192.168.1.100:514 \
  nginx

# 3. 审计 Docker 事件
docker events --filter 'type=container'
```

---

## 性能优化

```bash
# ============ 镜像优化 ============

# 1. 减小镜像大小
# 使用多阶段构建
# 清理缓存文件
# 合并 RUN 指令

# 2. 使用构建缓存
docker build --cache-from myapp:latest -t myapp:v2 .

# 3. 并行构建
docker buildx build --progress=plain .


# ============ 容器优化 ============

# 1. 限制日志大小
docker run -d \
  --log-opt max-size=10m \
  --log-opt max-file=3 \
  nginx

# 2. 使用 tmpfs
docker run -d \
  --tmpfs /tmp:rw,size=100m \
  myapp

# 3. 调整存储驱动
# /etc/docker/daemon.json
{
  "storage-driver": "overlay2",
  "storage-opts": [
    "overlay2.override_kernel_check=true"
  ]
}


# ============ 网络优化 ============

# 1. 使用 host 网络(性能最好)
docker run -d --network host nginx

# 2. 调整网络 MTU
docker network create --opt com.docker.network.driver.mtu=9000 mynet

# 3. 禁用 iptables (高级)
dockerd --iptables=false


# ============ 资源监控 ============

# 1. 实时监控
docker stats

# 2. 查看特定容器
docker stats web1 web2 --no-stream

# 3. 使用 cAdvisor
docker run -d \
  --name=cadvisor \
  -p 8080:8080 \
  -v /:/rootfs:ro \
  -v /var/run:/var/run:ro \
  -v /sys:/sys:ro \
  -v /var/lib/docker/:/var/lib/docker:ro \
  google/cadvisor
```

---

## 总结

### Docker 核心概念

```
镜像 (Image)
  └─ 分层存储,只读模板

容器 (Container)
  └─ 镜像的运行实例,可写层

仓库 (Registry)
  └─ 镜像存储和分发中心

网络 (Network)
  └─ 容器通信机制

卷 (Volume)
  └─ 数据持久化方案
```

### 最佳实践清单

**镜像**
- ✅ 使用官方镜像
- ✅ 固定版本标签
- ✅ 多阶段构建
- ✅ 最小化层数
- ✅ 清理临时文件

**容器**
- ✅ 单一进程
- ✅ 无状态设计
- ✅ 非 root 用户
- ✅ 资源限制
- ✅ 健康检查

**网络**
- ✅ 使用自定义网络
- ✅ 服务发现
- ✅ 最小暴露端口
- ✅ 网络隔离

**存储**
- ✅ Volume 持久化
- ✅ 定期备份
- ✅ 清理策略

**安全**
- ✅ 镜像扫描
- ✅ Secret 管理
- ✅ 最小权限
- ✅ 审计日志

### 下一步

恭喜!你已经完成容器技术系列学习!

继续探索:
- Kubernetes 容器编排
- Service Mesh (Istio, Linkerd)
- CI/CD 集成
- 监控和日志 (Prometheus, ELK)

---

## 参考资源

- [Docker 官方文档](https://docs.docker.com/)
- [Docker Hub](https://hub.docker.com/)
- [Dockerfile Best Practices](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/)
- [Docker Security](https://docs.docker.com/engine/security/)
- [Play with Docker](https://labs.play-with-docker.com/)

---

*更新日期: 2025-12-03*
*作者: Claude Code*
