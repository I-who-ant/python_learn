# Deployment

> 无状态应用、滚动更新、回滚、扩缩容

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Deployment 是 Kubernetes 中最常用的工作负载资源,用于管理无状态应用的声明式更新。它提供了滚动更新、回滚、扩缩容等强大功能。

## 核心概念

### 什么是 Deployment

Deployment 为 Pod 和 ReplicaSet 提供声明式更新能力,是管理无状态应用的标准方式。

**核心特性:**
- **声明式更新**: 描述期望状态,K8s 自动达成
- **滚动更新**: 零停机更新应用
- **版本回滚**: 快速回退到历史版本
- **自动扩缩容**: 手动或自动调整副本数
- **自愈能力**: 自动替换失败的 Pod

### 为什么需要

在生产环境中,我们需要 Deployment 来解决以下问题:

1. **应用更新**: 无缝升级应用版本
2. **高可用**: 保证多副本运行
3. **弹性伸缩**: 根据负载调整副本数
4. **故障恢复**: 自动重启失败的 Pod
5. **版本管理**: 管理应用的多个版本

### Deployment 架构

```
┌─────────────────────────────────┐
│        Deployment               │
│  (期望状态: replicas=3)         │
└─────────┬───────────────────────┘
          │ 管理
          ↓
┌─────────────────────────────────┐
│        ReplicaSet               │
│  (确保 Pod 副本数)               │
└─────────┬───────────────────────┘
          │ 创建
          ↓
    ┌─────┴─────┬─────┐
    ↓           ↓     ↓
  ┌───┐       ┌───┐ ┌───┐
  │Pod│       │Pod│ │Pod│
  └───┘       └───┘ └───┘
```

## 基本使用

### 创建 Deployment

**方式 1: 命令行创建**
```bash
# 快速创建 Deployment
kubectl create deployment nginx --image=nginx:1.21 --replicas=3

# 暴露端口
kubectl create deployment web --image=nginx:1.21 --port=80 --replicas=3
```

**方式 2: YAML 配置**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-deployment
  labels:
    app: nginx
spec:
  # 副本数
  replicas: 3

  # 选择器 (必须匹配 template.labels)
  selector:
    matchLabels:
      app: nginx

  # Pod 模板
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
```

应用配置:
```bash
kubectl apply -f nginx-deployment.yaml
```

### 完整配置示例

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web-app
  namespace: default
  labels:
    app: web
    tier: frontend
  annotations:
    description: "Web application deployment"

spec:
  # 副本数
  replicas: 3

  # 选择器
  selector:
    matchLabels:
      app: web
      tier: frontend

  # 更新策略
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 1        # 最多多出1个Pod
      maxUnavailable: 1  # 最多1个Pod不可用

  # 最小就绪时间
  minReadySeconds: 10

  # 保留的历史版本数
  revisionHistoryLimit: 10

  # Pod 模板
  template:
    metadata:
      labels:
        app: web
        tier: frontend
        version: "1.0"
    spec:
      containers:
      - name: web
        image: nginx:1.21
        ports:
        - containerPort: 80
          name: http

        # 资源限制
        resources:
          requests:
            memory: "128Mi"
            cpu: "250m"
          limits:
            memory: "256Mi"
            cpu: "500m"

        # 健康检查
        livenessProbe:
          httpGet:
            path: /healthz
            port: 80
          initialDelaySeconds: 15
          periodSeconds: 10

        readinessProbe:
          httpGet:
            path: /ready
            port: 80
          initialDelaySeconds: 5
          periodSeconds: 5

        # 环境变量
        env:
        - name: ENV
          value: "production"

        # 卷挂载
        volumeMounts:
        - name: config
          mountPath: /etc/nginx/conf.d

      # 卷
      volumes:
      - name: config
        configMap:
          name: nginx-config
```

### kubectl 命令

**查看 Deployment**
```bash
# 列出所有 Deployment
kubectl get deployments
kubectl get deploy

# 详细信息
kubectl get deployment nginx-deployment -o wide

# 查看详情
kubectl describe deployment nginx-deployment

# 查看 YAML
kubectl get deployment nginx-deployment -o yaml
```

**更新 Deployment**
```bash
# 更新镜像
kubectl set image deployment/nginx-deployment nginx=nginx:1.22

# 编辑 Deployment
kubectl edit deployment nginx-deployment

# 应用 YAML 更新
kubectl apply -f nginx-deployment.yaml
```

**扩缩容**
```bash
# 扩容到 5 个副本
kubectl scale deployment nginx-deployment --replicas=5

# 根据条件自动扩缩容
kubectl autoscale deployment nginx-deployment --min=3 --max=10 --cpu-percent=80
```

**查看更新状态**
```bash
# 查看滚动更新状态
kubectl rollout status deployment/nginx-deployment

# 查看更新历史
kubectl rollout history deployment/nginx-deployment

# 查看特定版本详情
kubectl rollout history deployment/nginx-deployment --revision=2
```

**回滚**
```bash
# 回滚到上一个版本
kubectl rollout undo deployment/nginx-deployment

# 回滚到指定版本
kubectl rollout undo deployment/nginx-deployment --to-revision=2
```

**暂停和恢复**
```bash
# 暂停更新 (用于批量更改)
kubectl rollout pause deployment/nginx-deployment

# 进行多次更改
kubectl set image deployment/nginx-deployment nginx=nginx:1.22
kubectl set resources deployment/nginx-deployment -c=nginx --limits=cpu=200m,memory=512Mi

# 恢复更新
kubectl rollout resume deployment/nginx-deployment
```

**删除 Deployment**
```bash
# 删除 Deployment (同时删除 Pod)
kubectl delete deployment nginx-deployment

# 删除但不删除 Pod
kubectl delete deployment nginx-deployment --cascade=orphan
```

## 更新策略

### 滚动更新 (RollingUpdate)

默认策略,逐步替换旧 Pod:

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: rolling-update-demo
spec:
  replicas: 10
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 3        # 允许超出期望副本数的最大值
      maxUnavailable: 2  # 允许不可用的最大 Pod 数

  selector:
    matchLabels:
      app: demo

  template:
    metadata:
      labels:
        app: demo
    spec:
      containers:
      - name: app
        image: myapp:1.0
        ports:
        - containerPort: 8080
```

**更新流程:**
```
初始: 10 个 v1.0 Pod 运行中
步骤1: 创建 3 个 v1.1 Pod (maxSurge=3)
      删除 2 个 v1.0 Pod (maxUnavailable=2)
      [8 个 v1.0 + 3 个 v1.1]

步骤2: 删除 3 个 v1.0 Pod
      创建 3 个 v1.1 Pod
      [5 个 v1.0 + 6 个 v1.1]

步骤3: 继续直到所有 Pod 都是 v1.1
最终: 10 个 v1.1 Pod 运行中
```

### 重建 (Recreate)

先删除所有旧 Pod,再创建新 Pod:

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: recreate-demo
spec:
  replicas: 5
  strategy:
    type: Recreate  # 重建策略

  selector:
    matchLabels:
      app: demo

  template:
    metadata:
      labels:
        app: demo
    spec:
      containers:
      - name: app
        image: myapp:1.0
```

**更新流程:**
```
初始: 5 个 v1.0 Pod 运行中
步骤1: 删除所有 5 个 v1.0 Pod
步骤2: 等待所有 Pod 终止
步骤3: 创建 5 个 v1.1 Pod
最终: 5 个 v1.1 Pod 运行中

⚠️ 注意: 更新期间服务会中断
```

**使用场景:**
- 应用不支持多版本并存
- 需要完全清理旧版本
- 可以接受短暂停机

## 实战案例

### 案例 1: Web 应用部署

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web-app
  labels:
    app: web
    env: production
spec:
  replicas: 5
  selector:
    matchLabels:
      app: web

  # 滚动更新策略
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 2
      maxUnavailable: 1

  minReadySeconds: 10

  template:
    metadata:
      labels:
        app: web
        version: "1.2.3"
    spec:
      containers:
      - name: nginx
        image: nginx:1.21
        ports:
        - containerPort: 80

        # 存活探针
        livenessProbe:
          httpGet:
            path: /
            port: 80
          initialDelaySeconds: 30
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 3

        # 就绪探针
        readinessProbe:
          httpGet:
            path: /
            port: 80
          initialDelaySeconds: 5
          periodSeconds: 5
          timeoutSeconds: 3
          successThreshold: 1

        # 资源限制
        resources:
          requests:
            memory: "256Mi"
            cpu: "500m"
          limits:
            memory: "512Mi"
            cpu: "1000m"

        # 优雅关闭
        lifecycle:
          preStop:
            exec:
              command: ["/bin/sh", "-c", "sleep 15"]

      # 终止宽限期
      terminationGracePeriodSeconds: 30
---
# Service
apiVersion: v1
kind: Service
metadata:
  name: web-service
spec:
  selector:
    app: web
  ports:
  - port: 80
    targetPort: 80
  type: LoadBalancer
```

部署和更新:
```bash
# 应用配置
kubectl apply -f web-app.yaml

# 查看状态
kubectl get deployment web-app
kubectl get pods -l app=web

# 更新镜像
kubectl set image deployment/web-app nginx=nginx:1.22

# 监控更新
kubectl rollout status deployment/web-app

# 如果有问题,立即回滚
kubectl rollout undo deployment/web-app
```

### 案例 2: 金丝雀发布 (Canary)

```yaml
# 稳定版本 (90% 流量)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-stable
  labels:
    app: myapp
    version: stable
spec:
  replicas: 9
  selector:
    matchLabels:
      app: myapp
      version: stable
  template:
    metadata:
      labels:
        app: myapp
        version: stable
    spec:
      containers:
      - name: app
        image: myapp:1.0
        ports:
        - containerPort: 8080
---
# 金丝雀版本 (10% 流量)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-canary
  labels:
    app: myapp
    version: canary
spec:
  replicas: 1
  selector:
    matchLabels:
      app: myapp
      version: canary
  template:
    metadata:
      labels:
        app: myapp
        version: canary
    spec:
      containers:
      - name: app
        image: myapp:2.0  # 新版本
        ports:
        - containerPort: 8080
---
# Service (同时路由到两个版本)
apiVersion: v1
kind: Service
metadata:
  name: myapp-service
spec:
  selector:
    app: myapp  # 匹配两个版本
  ports:
  - port: 80
    targetPort: 8080
```

金丝雀流程:
```bash
# 1. 部署稳定版本
kubectl apply -f app-stable.yaml

# 2. 部署金丝雀版本 (小流量)
kubectl apply -f app-canary.yaml

# 3. 监控金丝雀版本
kubectl logs -f -l version=canary
kubectl top pods -l version=canary

# 4. 如果正常,逐步增加金丝雀副本
kubectl scale deployment app-canary --replicas=3
kubectl scale deployment app-stable --replicas=7

# 5. 继续增加
kubectl scale deployment app-canary --replicas=5
kubectl scale deployment app-stable --replicas=5

# 6. 全部切换到新版本
kubectl scale deployment app-canary --replicas=10
kubectl scale deployment app-stable --replicas=0

# 7. 删除旧版本
kubectl delete deployment app-stable
```

### 案例 3: 蓝绿部署 (Blue-Green)

```yaml
# 蓝色环境 (当前生产)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-blue
  labels:
    app: myapp
    env: blue
spec:
  replicas: 5
  selector:
    matchLabels:
      app: myapp
      env: blue
  template:
    metadata:
      labels:
        app: myapp
        env: blue
    spec:
      containers:
      - name: app
        image: myapp:1.0
        ports:
        - containerPort: 8080
---
# 绿色环境 (新版本)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-green
  labels:
    app: myapp
    env: green
spec:
  replicas: 5
  selector:
    matchLabels:
      app: myapp
      env: green
  template:
    metadata:
      labels:
        app: myapp
        env: green
    spec:
      containers:
      - name: app
        image: myapp:2.0  # 新版本
        ports:
        - containerPort: 8080
---
# Service (初始指向蓝色)
apiVersion: v1
kind: Service
metadata:
  name: myapp-service
spec:
  selector:
    app: myapp
    env: blue  # 当前生产流量
  ports:
  - port: 80
    targetPort: 8080
  type: LoadBalancer
```

蓝绿切换:
```bash
# 1. 部署蓝色环境 (生产)
kubectl apply -f blue-deployment.yaml

# 2. 部署绿色环境 (新版本)
kubectl apply -f green-deployment.yaml

# 3. 测试绿色环境
kubectl port-forward deployment/app-green 8080:8080

# 4. 切换流量到绿色 (瞬间切换)
kubectl patch service myapp-service -p '{"spec":{"selector":{"env":"green"}}}'

# 5. 验证绿色环境
# 监控日志、指标

# 6. 如果有问题,快速回滚到蓝色
kubectl patch service myapp-service -p '{"spec":{"selector":{"env":"blue"}}}'

# 7. 绿色稳定后,删除蓝色环境
kubectl delete deployment app-blue
```

### 案例 4: 多环境管理

```yaml
# 开发环境
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-dev
  namespace: development
  labels:
    app: myapp
    env: dev
spec:
  replicas: 1  # 开发环境少量副本
  selector:
    matchLabels:
      app: myapp
  template:
    metadata:
      labels:
        app: myapp
    spec:
      containers:
      - name: app
        image: myapp:dev
        env:
        - name: ENV
          value: "development"
        resources:
          requests:
            memory: "128Mi"
            cpu: "100m"
---
# 测试环境
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-staging
  namespace: staging
  labels:
    app: myapp
    env: staging
spec:
  replicas: 2
  selector:
    matchLabels:
      app: myapp
  template:
    metadata:
      labels:
        app: myapp
    spec:
      containers:
      - name: app
        image: myapp:v1.2.3-rc1
        env:
        - name: ENV
          value: "staging"
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
---
# 生产环境
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-prod
  namespace: production
  labels:
    app: myapp
    env: prod
spec:
  replicas: 10  # 生产环境多副本
  selector:
    matchLabels:
      app: myapp
  template:
    metadata:
      labels:
        app: myapp
    spec:
      containers:
      - name: app
        image: myapp:1.2.3  # 稳定版本
        env:
        - name: ENV
          value: "production"
        resources:
          requests:
            memory: "512Mi"
            cpu: "500m"
          limits:
            memory: "1Gi"
            cpu: "1000m"
        # 生产环境严格的健康检查
        livenessProbe:
          httpGet:
            path: /healthz
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /ready
            port: 8080
          initialDelaySeconds: 10
          periodSeconds: 5
```

## 最佳实践

### 1. 副本数设置

```yaml
# 生产环境建议
spec:
  replicas: 3  # 最少 3 个副本,确保高可用

  # 跨可用区部署
  template:
    spec:
      affinity:
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchLabels:
                  app: myapp
              topologyKey: topology.kubernetes.io/zone
```

### 2. 资源限制

```yaml
resources:
  requests:  # 调度器保证的最小资源
    memory: "256Mi"
    cpu: "250m"
  limits:    # 容器可使用的最大资源
    memory: "512Mi"
    cpu: "500m"
```

### 3. 健康检查

```yaml
# 存活探针: 容器是否健康
livenessProbe:
  httpGet:
    path: /healthz
    port: 8080
  initialDelaySeconds: 30
  periodSeconds: 10
  timeoutSeconds: 5
  failureThreshold: 3

# 就绪探针: 容器是否可以接收流量
readinessProbe:
  httpGet:
    path: /ready
    port: 8080
  initialDelaySeconds: 5
  periodSeconds: 5
  successThreshold: 1
```

### 4. 更新策略

```yaml
strategy:
  type: RollingUpdate
  rollingUpdate:
    maxSurge: 25%       # 或具体数字如 2
    maxUnavailable: 25% # 或具体数字如 1

minReadySeconds: 10  # Pod 就绪后等待时间
```

### 5. 标签管理

```yaml
metadata:
  labels:
    app: myapp
    version: "1.2.3"
    env: production
    tier: frontend

template:
  metadata:
    labels:
      app: myapp  # 必须匹配 selector
      version: "1.2.3"
      commit: "abc123"  # Git commit
```

### 6. 优雅终止

```yaml
spec:
  terminationGracePeriodSeconds: 30

  containers:
  - name: app
    lifecycle:
      preStop:
        exec:
          command: ["/bin/sh", "-c", "sleep 15"]
```

## 常见问题

### Q1: Deployment 更新卡住?

**排查步骤:**
```bash
# 查看 Deployment 状态
kubectl rollout status deployment/myapp

# 查看 Pod 状态
kubectl get pods -l app=myapp

# 查看失败原因
kubectl describe deployment myapp
kubectl describe pod <pod-name>

# 查看事件
kubectl get events --sort-by='.lastTimestamp'
```

**常见原因:**
- 镜像拉取失败
- 就绪探针失败
- 资源不足
- Pod 无法调度

### Q2: 如何实现零停机更新?

**关键配置:**
```yaml
spec:
  minReadySeconds: 10  # 等待 Pod 稳定

  strategy:
    rollingUpdate:
      maxUnavailable: 0  # 始终保持所有 Pod 可用
      maxSurge: 1        # 逐个更新

  template:
    spec:
      containers:
      - name: app
        readinessProbe:  # 必须配置就绪探针
          httpGet:
            path: /ready
            port: 8080
        lifecycle:
          preStop:       # 优雅终止
            exec:
              command: ["/bin/sh", "-c", "sleep 15"]
```

### Q3: 如何快速回滚?

```bash
# 立即回滚到上一版本
kubectl rollout undo deployment/myapp

# 回滚到特定版本
kubectl rollout history deployment/myapp
kubectl rollout undo deployment/myapp --to-revision=3
```

### Q4: Deployment vs ReplicaSet vs Pod?

| 对比项 | Deployment | ReplicaSet | Pod |
|-------|-----------|-----------|-----|
| 用途 | 管理无状态应用 | 维护 Pod 副本数 | 最小部署单元 |
| 更新 | 支持滚动更新 | 不支持更新 | 不可变 |
| 回滚 | 支持版本回滚 | 不支持 | 不支持 |
| 使用 | 直接使用 | 由 Deployment 管理 | 通常不直接创建 |

## 总结

- ✅ 理解 Deployment 的作用和架构
- ✅ 掌握创建、更新、扩缩容操作
- ✅ 熟悉滚动更新和回滚机制
- ✅ 了解金丝雀、蓝绿部署模式
- ✅ 能够配置健康检查和资源限制
- ✅ 实现零停机更新

## 参考资源

- [Deployment 官方文档](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/deployment/)
- [更新策略](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/deployment/#strategy)
- [滚动更新](https://kubernetes.io/zh-cn/docs/tutorials/kubernetes-basics/update/update-intro/)
- [金丝雀部署](https://kubernetes.io/docs/concepts/cluster-administration/manage-deployment/#canary-deployments)

---

*更新日期: 2025-12-03*
