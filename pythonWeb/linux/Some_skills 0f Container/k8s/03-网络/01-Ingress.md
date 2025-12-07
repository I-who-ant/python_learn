# Ingress

> HTTP 路由、TLS 终止、虚拟主机

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Ingress 是 Kubernetes 中用于管理集群外部访问的 API 对象,提供 HTTP 和 HTTPS 路由功能。与 Service 不同,Ingress 工作在七层(应用层),可以根据域名、路径等信息进行流量路由。

## 核心概念

### 什么是 Ingress

Ingress 是 Kubernetes 的资源对象,主要功能包括:

- **HTTP/HTTPS 路由**: 基于域名和路径的智能路由
- **TLS/SSL 终止**: 在 Ingress 层处理 HTTPS 加密
- **虚拟主机**: 通过不同域名访问不同服务
- **负载均衡**: 七层负载均衡和流量分发

### 为什么需要 Ingress

在生产环境中,我们需要 Ingress 来解决以下问题:

1. **统一入口**: 避免为每个服务创建 LoadBalancer,节省成本
2. **域名路由**: 通过不同域名或路径访问不同服务
3. **SSL/TLS 管理**: 集中管理 HTTPS 证书
4. **高级路由**: 支持基于 Header、Cookie 等的路由规则

### Ingress 架构

```
Internet
    |
[Ingress Controller]
    |
[Ingress Rules]
    |
[Services] --> [Pods]
```

**关键组件**:
- **Ingress 资源**: 定义路由规则的 YAML 配置
- **Ingress Controller**: 实际执行路由的控制器(如 Nginx Ingress Controller)

## 基本使用

### 前置条件

必须先安装 Ingress Controller,常见选择:
- **Nginx Ingress Controller** (最流行)
- Traefik
- HAProxy Ingress
- Kong Ingress

### 示例配置

#### 1. 简单的 Ingress 配置

```yaml
# simple-ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: simple-ingress
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /
spec:
  rules:
  - host: example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: web-service
            port:
              number: 80
```

#### 2. 多路径路由

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: multi-path-ingress
spec:
  rules:
  - host: api.example.com
    http:
      paths:
      - path: /v1
        pathType: Prefix
        backend:
          service:
            name: api-v1-service
            port:
              number: 8080
      - path: /v2
        pathType: Prefix
        backend:
          service:
            name: api-v2-service
            port:
              number: 8080
```

#### 3. 多域名配置

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: multi-host-ingress
spec:
  rules:
  - host: www.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: frontend-service
            port:
              number: 80
  - host: api.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: api-service
            port:
              number: 8080
```

#### 4. TLS/HTTPS 配置

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: tls-ingress
spec:
  tls:
  - hosts:
    - secure.example.com
    secretName: tls-secret  # 包含证书的 Secret
  rules:
  - host: secure.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: secure-service
            port:
              number: 443
```

创建 TLS Secret:

```bash
# 创建 TLS Secret
kubectl create secret tls tls-secret \
  --cert=path/to/cert.crt \
  --key=path/to/cert.key
```

### kubectl 命令

```bash
# 创建 Ingress
kubectl create -f ingress.yaml
kubectl apply -f ingress.yaml

# 查看 Ingress
kubectl get ingress
kubectl get ing

# 查看详细信息
kubectl describe ingress simple-ingress

# 查看 Ingress 的访问地址
kubectl get ingress simple-ingress -o wide

# 编辑 Ingress
kubectl edit ingress simple-ingress

# 删除 Ingress
kubectl delete ingress simple-ingress

# 查看 Ingress Controller 日志
kubectl logs -n ingress-nginx <ingress-controller-pod>

# 查看所有命名空间的 Ingress
kubectl get ingress --all-namespaces
```

## pathType 说明

Kubernetes 支持三种路径匹配类型:

1. **Prefix**: 前缀匹配(最常用)
   - `/foo` 匹配 `/foo`, `/foo/`, `/foo/bar`

2. **Exact**: 精确匹配
   - `/foo` 只匹配 `/foo`

3. **ImplementationSpecific**: 由 Ingress Controller 决定

## 实战案例

### 案例 1: 基于路径的微服务路由

部署多个微服务并通过 Ingress 统一访问:

```yaml
# microservices-ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: microservices-ingress
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /$2
spec:
  rules:
  - host: app.example.com
    http:
      paths:
      - path: /auth(/|$)(.*)
        pathType: ImplementationSpecific
        backend:
          service:
            name: auth-service
            port:
              number: 8080
      - path: /user(/|$)(.*)
        pathType: ImplementationSpecific
        backend:
          service:
            name: user-service
            port:
              number: 8080
      - path: /order(/|$)(.*)
        pathType: ImplementationSpecific
        backend:
          service:
            name: order-service
            port:
              number: 8080
```

访问示例:
- `http://app.example.com/auth/login` → auth-service
- `http://app.example.com/user/profile` → user-service
- `http://app.example.com/order/list` → order-service

### 案例 2: 蓝绿部署

通过 Ingress 实现流量切换:

```yaml
# blue-green-ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: blue-green-ingress
  annotations:
    nginx.ingress.kubernetes.io/canary: "false"
spec:
  rules:
  - host: app.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: app-blue  # 当前生产版本
            port:
              number: 80

---
# 金丝雀测试版本
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: canary-ingress
  annotations:
    nginx.ingress.kubernetes.io/canary: "true"
    nginx.ingress.kubernetes.io/canary-weight: "10"  # 10% 流量
spec:
  rules:
  - host: app.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: app-green  # 新版本
            port:
              number: 80
```

### 案例 3: 基于 Header 的路由

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: header-based-ingress
  annotations:
    nginx.ingress.kubernetes.io/canary: "true"
    nginx.ingress.kubernetes.io/canary-by-header: "X-Beta-Version"
    nginx.ingress.kubernetes.io/canary-by-header-value: "true"
spec:
  rules:
  - host: app.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: app-beta
            port:
              number: 80
```

测试:
```bash
# 正常访问(生产版本)
curl http://app.example.com

# 带 Header 访问(测试版本)
curl -H "X-Beta-Version: true" http://app.example.com
```

### 案例 4: 限流和认证

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: rate-limit-ingress
  annotations:
    # 限流配置
    nginx.ingress.kubernetes.io/limit-rps: "10"
    nginx.ingress.kubernetes.io/limit-burst-multiplier: "2"

    # Basic Auth
    nginx.ingress.kubernetes.io/auth-type: basic
    nginx.ingress.kubernetes.io/auth-secret: basic-auth
    nginx.ingress.kubernetes.io/auth-realm: "Authentication Required"
spec:
  rules:
  - host: protected.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: protected-service
            port:
              number: 80
```

创建 Basic Auth Secret:
```bash
# 安装 htpasswd
sudo apt-get install apache2-utils

# 创建密码文件
htpasswd -c auth admin

# 创建 Secret
kubectl create secret generic basic-auth --from-file=auth
```

## 常用 Annotations

Nginx Ingress Controller 常用注解:

```yaml
metadata:
  annotations:
    # 路径重写
    nginx.ingress.kubernetes.io/rewrite-target: /

    # SSL 重定向
    nginx.ingress.kubernetes.io/ssl-redirect: "true"

    # CORS 配置
    nginx.ingress.kubernetes.io/enable-cors: "true"
    nginx.ingress.kubernetes.io/cors-allow-origin: "*"

    # 超时设置
    nginx.ingress.kubernetes.io/proxy-connect-timeout: "30"
    nginx.ingress.kubernetes.io/proxy-send-timeout: "30"
    nginx.ingress.kubernetes.io/proxy-read-timeout: "30"

    # 请求体大小限制
    nginx.ingress.kubernetes.io/proxy-body-size: "100m"

    # 白名单
    nginx.ingress.kubernetes.io/whitelist-source-range: "10.0.0.0/8,192.168.0.0/16"
```

## 最佳实践

1. **生产环境建议**
   - 使用 Ingress 作为集群的统一入口
   - 为不同环境使用不同的域名
   - 配置健康检查确保服务可用性

2. **性能优化**
   - 启用 gzip 压缩减少传输数据量
   - 合理配置连接超时时间
   - 使用 HTTP/2 提升性能

3. **安全加固**
   - 强制使用 HTTPS (ssl-redirect: "true")
   - 配置 IP 白名单限制访问
   - 使用 cert-manager 自动管理证书
   - 启用 Rate Limiting 防止 DDoS

4. **监控告警**
   - 监控 Ingress Controller 的资源使用
   - 监控响应时间和错误率
   - 配置日志收集便于排查问题

5. **高可用性**
   - 部署多个 Ingress Controller 副本
   - 使用 HPA 自动扩缩容
   - 配置反亲和性避免单点故障

## 常见问题

### Q1: Ingress 创建后无法访问?

A: 排查步骤:
1. 检查 Ingress Controller 是否正常运行
   ```bash
   kubectl get pods -n ingress-nginx
   ```
2. 检查 Ingress 配置是否正确
   ```bash
   kubectl describe ingress <ingress-name>
   ```
3. 检查 Service 和 Pod 是否正常
4. 检查 DNS 解析是否正确
5. 查看 Ingress Controller 日志

### Q2: 如何调试 Ingress 路由问题?

A:
```bash
# 查看 Ingress 详情
kubectl describe ingress <name>

# 查看 Ingress Controller 日志
kubectl logs -n ingress-nginx <controller-pod> --tail=100

# 查看生成的 Nginx 配置
kubectl exec -n ingress-nginx <controller-pod> -- cat /etc/nginx/nginx.conf
```

### Q3: 502/504 错误如何解决?

A: 常见原因:
- **502 Bad Gateway**: 后端 Service 或 Pod 不可用
  - 检查 Pod 状态和日志
  - 检查 Service Endpoints

- **504 Gateway Timeout**: 后端响应超时
  - 增加超时配置
  - 优化后端服务性能

### Q4: 如何配置默认后端?

A:
```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: ingress-with-default
spec:
  defaultBackend:
    service:
      name: default-service
      port:
        number: 80
  rules:
  - host: app.example.com
    # ... 其他规则
```

### Q5: Ingress 和 Service LoadBalancer 的区别?

A:
- **Ingress**: 七层负载均衡,支持域名/路径路由,多个服务共享一个入口
- **LoadBalancer**: 四层负载均衡,每个服务需要独立的公网 IP,成本高

推荐: 使用 Ingress 作为统一入口,后端连接 ClusterIP Service

## 总结

- ✅ 理解 Ingress 的概念和架构
- ✅ 掌握 Ingress 的基本配置和路由规则
- ✅ 理解 TLS/HTTPS 配置方法
- ✅ 掌握基于路径、域名、Header 的高级路由
- ✅ 了解常用 Annotations 和最佳实践
- ✅ 能够排查 Ingress 相关问题

## 参考资源

- [Kubernetes Ingress 官方文档](https://kubernetes.io/docs/concepts/services-networking/ingress/)
- [Nginx Ingress Controller 文档](https://kubernetes.github.io/ingress-nginx/)
- [Ingress Annotations](https://kubernetes.github.io/ingress-nginx/user-guide/nginx-configuration/annotations/)

---

*更新日期: 2025-12-03*
