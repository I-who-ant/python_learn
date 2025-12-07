# Service详解

> Service 类型、服务发现、Endpoint

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Service 是 Kubernetes 中用于将运行在一组 Pod 上的应用程序公开为网络服务的抽象方法。它为一组功能相同的 Pod 提供一个统一的访问入口,解决了 Pod IP 动态变化的问题。

## 核心概念

### 什么是 Service

Service 是 Kubernetes 的核心资源对象,主要功能包括:

- **服务发现**: 为一组 Pod 提供稳定的访问入口
- **负载均衡**: 在多个 Pod 之间分发流量
- **服务抽象**: 将后端 Pod 的实现细节与前端访问者解耦

### 为什么需要 Service

在生产环境中,我们需要 Service 来解决以下问题:

1. **Pod IP 不固定**: Pod 重启后 IP 会变化,无法直接通过 IP 访问
2. **多副本负载均衡**: 需要在多个 Pod 副本间分发流量
3. **服务发现**: 前端应用需要发现和连接后端服务
4. **外部访问**: 需要将集群内服务暴露给外部用户

### Service 类型

#### 1. ClusterIP (默认)

集群内部 IP,只能在集群内访问。

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-service
spec:
  type: ClusterIP
  selector:
    app: myapp
  ports:
    - protocol: TCP
      port: 80
      targetPort: 8080
```

#### 2. NodePort

通过每个节点的 IP 和静态端口暴露服务。

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-nodeport-service
spec:
  type: NodePort
  selector:
    app: myapp
  ports:
    - protocol: TCP
      port: 80
      targetPort: 8080
      nodePort: 30080  # 范围: 30000-32767
```

#### 3. LoadBalancer

使用云提供商的负载均衡器对外暴露服务。

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-lb-service
spec:
  type: LoadBalancer
  selector:
    app: myapp
  ports:
    - protocol: TCP
      port: 80
      targetPort: 8080
```

#### 4. ExternalName

将服务映射到外部 DNS 名称。

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-external-service
spec:
  type: ExternalName
  externalName: api.example.com
```

### Endpoint

Endpoint 是 Service 关联的后端 Pod IP 和端口列表。

```bash
# 查看 Service 的 Endpoint
kubectl get endpoints my-service
```

### 服务发现

Kubernetes 提供两种服务发现方式:

1. **环境变量**: Pod 创建时自动注入 Service 相关的环境变量
2. **DNS**: 通过 CoreDNS 实现,推荐使用

DNS 名称格式:
- 同命名空间: `<service-name>`
- 跨命名空间: `<service-name>.<namespace>.svc.cluster.local`

## 基本使用

### 示例配置

```yaml
# nginx-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-deployment
spec:
  replicas: 3
  selector:
    matchLabels:
      app: nginx
  template:
    metadata:
      labels:
        app: nginx
    spec:
      containers:
      - name: nginx
        image: nginx:1.21
        ports:
        - containerPort: 80

---
# nginx-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: nginx-service
spec:
  selector:
    app: nginx
  ports:
    - protocol: TCP
      port: 80
      targetPort: 80
```

### kubectl 命令

```bash
# 创建 Service
kubectl create -f service.yaml

# 查看 Service
kubectl get svc
kubectl get service

# 查看详细信息
kubectl describe svc nginx-service

# 查看 Endpoint
kubectl get endpoints nginx-service

# 删除 Service
kubectl delete svc nginx-service

# 通过 YAML 创建 Service 和 Deployment
kubectl apply -f nginx-deployment.yaml

# 快速暴露 Deployment 为 Service
kubectl expose deployment nginx-deployment --port=80 --type=NodePort

# 查看 Service 的详细信息(包括 Endpoints)
kubectl get svc nginx-service -o wide
```

## 实战案例

### 案例 1: ClusterIP 服务

创建一个集群内部访问的 Web 服务:

```yaml
# backend-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: backend
spec:
  replicas: 2
  selector:
    matchLabels:
      app: backend
  template:
    metadata:
      labels:
        app: backend
    spec:
      containers:
      - name: backend
        image: python:3.9-slim
        command: ["python", "-m", "http.server", "8000"]
        ports:
        - containerPort: 8000

---
apiVersion: v1
kind: Service
metadata:
  name: backend-service
spec:
  type: ClusterIP
  selector:
    app: backend
  ports:
    - port: 80
      targetPort: 8000
```

测试访问:

```bash
# 应用配置
kubectl apply -f backend-deployment.yaml

# 创建测试 Pod
kubectl run test-pod --image=busybox --rm -it -- sh

# 在 test-pod 中测试访问
wget -O- backend-service
```

### 案例 2: NodePort 对外暴露

```yaml
# frontend-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: frontend
spec:
  replicas: 3
  selector:
    matchLabels:
      app: frontend
  template:
    metadata:
      labels:
        app: frontend
    spec:
      containers:
      - name: nginx
        image: nginx:1.21
        ports:
        - containerPort: 80

---
apiVersion: v1
kind: Service
metadata:
  name: frontend-service
spec:
  type: NodePort
  selector:
    app: frontend
  ports:
    - port: 80
      targetPort: 80
      nodePort: 30080
```

访问服务:

```bash
# 应用配置
kubectl apply -f frontend-deployment.yaml

# 获取节点 IP
kubectl get nodes -o wide

# 访问服务
curl http://<NODE-IP>:30080
```

### 案例 3: Headless Service

用于 StatefulSet,不需要负载均衡:

```yaml
apiVersion: v1
kind: Service
metadata:
  name: mysql-headless
spec:
  clusterIP: None  # Headless Service
  selector:
    app: mysql
  ports:
    - port: 3306
      targetPort: 3306
```

这种配置下,DNS 查询会返回所有 Pod 的 IP,而不是单个 Service IP。

### 案例 4: 多端口 Service

```yaml
apiVersion: v1
kind: Service
metadata:
  name: multi-port-service
spec:
  selector:
    app: myapp
  ports:
    - name: http
      protocol: TCP
      port: 80
      targetPort: 8080
    - name: https
      protocol: TCP
      port: 443
      targetPort: 8443
    - name: metrics
      protocol: TCP
      port: 9090
      targetPort: 9090
```

## 最佳实践

1. **生产环境建议**
   - 使用 ClusterIP 作为默认类型,配合 Ingress 对外暴露
   - 避免直接使用 NodePort,端口管理困难
   - LoadBalancer 类型会产生云服务费用,谨慎使用

2. **性能优化**
   - 合理设置 `sessionAffinity: ClientIP` 保持会话亲和性
   - 使用 Headless Service 减少负载均衡开销(适用于 StatefulSet)
   - 配置就绪探针确保只有健康 Pod 接收流量

3. **安全加固**
   - 限制 Service 的访问来源(配合 NetworkPolicy)
   - 不要暴露不必要的端口
   - 使用命名空间隔离不同环境的服务

4. **监控告警**
   - 监控 Endpoint 数量,及时发现 Pod 异常
   - 监控 Service 的连接数和响应时间
   - 配置告警规则,Service 无可用 Endpoint 时报警

5. **命名规范**
   - 使用描述性名称: `frontend-service`, `api-service`
   - 多端口时必须命名每个端口
   - 保持 Service 名称和 Deployment 名称一致性

## 常见问题

### Q1: Service 无法访问怎么办?

A: 排查步骤:
1. 检查 Service selector 是否匹配 Pod labels
   ```bash
   kubectl get pods --show-labels
   kubectl describe svc <service-name>
   ```
2. 检查 Endpoint 是否正常
   ```bash
   kubectl get endpoints <service-name>
   ```
3. 检查 Pod 是否就绪
   ```bash
   kubectl get pods
   ```
4. 检查端口配置是否正确
5. 测试 Pod 网络连通性

### Q2: 为什么 Endpoint 为空?

A: 可能原因:
- Service 的 selector 与 Pod 的 labels 不匹配
- Pod 未处于 Ready 状态
- Pod 的 containerPort 与 Service 的 targetPort 不一致
- Pod 还在启动过程中

### Q3: ClusterIP 和 NodePort 的区别?

A:
- **ClusterIP**: 只能在集群内部访问,是默认类型
- **NodePort**: 在 ClusterIP 基础上,额外在每个节点上开放端口,可从集群外访问

### Q4: 如何实现服务的会话保持?

A: 配置 sessionAffinity:
```yaml
spec:
  sessionAffinity: ClientIP
  sessionAffinityConfig:
    clientIP:
      timeoutSeconds: 10800  # 3小时
```

### Q5: Service 和 Ingress 的区别?

A:
- **Service**: 四层负载均衡,基于 IP 和端口
- **Ingress**: 七层负载均衡,基于 HTTP/HTTPS,支持域名、路径路由

推荐架构: 外部流量 → Ingress → Service → Pod

## 总结

- ✅ 理解 Service 的四种类型及使用场景
- ✅ 掌握服务发现机制(DNS/环境变量)
- ✅ 理解 Endpoint 的概念和作用
- ✅ 掌握 Service 的创建和管理
- ✅ 了解负载均衡和会话保持
- ✅ 能够排查 Service 访问问题

## 参考资源

- [Kubernetes Service 官方文档](https://kubernetes.io/docs/concepts/services-networking/service/)
- [Service 和 Endpoint 详解](https://kubernetes.io/docs/concepts/services-networking/endpoint-slices/)
- [DNS for Services](https://kubernetes.io/docs/concepts/services-networking/dns-pod-service/)

---

*更新日期: 2025-12-03*
