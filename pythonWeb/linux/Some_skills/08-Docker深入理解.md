# 08 - Docker深入理解

> 工业级容器实现

## 核心概念

【本文档是容器技术学习系列的第 08 篇】


## Docker 架构

```
docker命令 -> dockerd -> containerd -> runc
```

## Dockerfile 最佳实践

```dockerfile
FROM alpine:latest
WORKDIR /app
COPY . .
USER nobody
CMD ["./app"]
```

## Docker Compose

```yaml
version: '3'
services:
  web:
    image: nginx
    ports:
      - "80:80"
```

## 下一步

学习 [09-Kubernetes架构](./09-Kubernetes架构.md)

---

*更新日期: 2025-12-02*
