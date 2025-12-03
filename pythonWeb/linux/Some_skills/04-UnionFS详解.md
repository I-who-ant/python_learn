# 04 - UnionFS详解

> 容器镜像的分层存储原理

## 核心概念

【本文档是容器技术学习系列的第 04 篇】


## UnionFS 原理

UnionFS 将多个目录联合挂载,形成分层文件系统。

```bash
# OverlayFS 示例
mkdir -p /tmp/overlay/{{lower,upper,work,merged}}
echo "base" > /tmp/overlay/lower/file.txt
sudo mount -t overlay overlay -o lowerdir=/tmp/overlay/lower,upperdir=/tmp/overlay/upper,workdir=/tmp/overlay/work /tmp/overlay/merged
```

## Docker 镜像分层

每个 Dockerfile 指令创建一层，所有层叠加形成最终镜像。

```bash
docker history nginx
docker inspect nginx | grep -A 10 GraphDriver
```

## 下一步

继续学习 [05-容器网络详解](./05-容器网络详解.md)

---

*更新日期: 2025-12-02*
