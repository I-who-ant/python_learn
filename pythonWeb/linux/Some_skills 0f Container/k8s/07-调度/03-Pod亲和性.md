# Pod亲和性

> podAffinity、podAntiAffinity、拓扑域、共同定位

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Pod 亲和性(Pod Affinity)和反亲和性(Pod Anti-Affinity)用于根据已运行的 Pod 的标签来决定新 Pod 的调度位置。通过这些机制,可以实现 Pod 的共同定位或分散部署。

## 核心概念

### 什么是 Pod 亲和性

**Pod 亲和性(Pod Affinity):**
- 让 Pod 调度到运行特定 Pod 的节点上
- 实现服务间的共同定位
- 减少网络延迟

**Pod 反亲和性(Pod Anti-Affinity):**
- 让 Pod 避开运行特定 Pod 的节点
- 实现 Pod 的分散部署
- 提高可用性和容错能力

### 工作原理

```
            Pod Affinity
               ↓
  ┌──────────────────────┐
  │  前端 Pod 想要调度到  │
  │  运行后端 Pod 的节点  │
  └──────────────────────┘
               ↓
    ┌────────────────────┐
    │  Node 1            │
    │  - backend-pod-1   │
    │  - frontend-pod-1  │  ✅ 亲和性满足
    └────────────────────┘

         Pod Anti-Affinity
               ↓
  ┌──────────────────────┐
  │  前端 Pod 想要避开     │
  │  运行其他前端 Pod     │
  │  的节点               │
  └──────────────────────┘
               ↓
    ┌────────────────────┐     ┌────────────────────┐
    │  Node 1            │     │  Node 2            │
    │  - frontend-pod-1  │     │  - frontend-pod-2  │
    └────────────────────┘     └────────────────────┘
         ✅ 反亲和性满足 - Pod 分散到不同节点
```

### 为什么需要

在生产环境中,我们需要 Pod 亲和性来实现:

1. **服务共同定位**: 前端和后端部署在一起,减少延迟
2. **高可用部署**: 多副本分散到不同节点/可用区
3. **性能优化**: 数据密集型服务共同定位
4. **合规要求**: 特定服务必须/不能部署在一起
5. **资源优化**: 共享本地缓存或数据

### 亲和性类型

| 类型 | 约束性 | 作用 |
|------|--------|------|
| requiredDuringSchedulingIgnoredDuringExecution | 硬性要求 | 必须满足,否则不调度 |
| preferredDuringSchedulingIgnoredDuringExecution | 软性偏好 | 尽量满足,但不保证 |

## 基本使用

### Pod 亲和性(共同定位)

**硬性要求:**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: frontend-pod
spec:
  affinity:
    podAffinity:
      requiredDuringSchedulingIgnoredDuringExecution:
      - labelSelector:
          matchExpressions:
          - key: app
            operator: In
            values:
            - backend
        topologyKey: kubernetes.io/hostname  # 在同一节点

  containers:
  - name: frontend
    image: frontend:latest
```

**工作流程:**
```
1. 查找带有标签 app=backend 的 Pod
2. 获取这些 Pod 所在的节点
3. 将 frontend-pod 调度到相同节点上
4. 如果没有符合条件的节点,Pod 保持 Pending
```

**软性偏好:**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: cache-pod
spec:
  affinity:
    podAffinity:
      preferredDuringSchedulingIgnoredDuringExecution:
      - weight: 100  # 权重 1-100
        podAffinityTerm:
          labelSelector:
            matchExpressions:
            - key: app
              operator: In
              values:
              - database
          topologyKey: kubernetes.io/hostname

  containers:
  - name: cache
    image: redis:6
```

### Pod 反亲和性(分散部署)

**硬性要求:**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web-app
spec:
  replicas: 3
  selector:
    matchLabels:
      app: web

  template:
    metadata:
      labels:
        app: web
    spec:
      affinity:
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: app
                operator: In
                values:
                - web
            topologyKey: kubernetes.io/hostname  # 不同节点

      containers:
      - name: web
        image: nginx:1.21
```

**效果:**
```
Pod-1 → Node-1
Pod-2 → Node-2  (不会调度到 Node-1)
Pod-3 → Node-3  (不会调度到 Node-1 或 Node-2)
Pod-4 → Pending (如果只有 3 个节点)
```

**软性偏好:**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: api-server
spec:
  replicas: 5
  selector:
    matchLabels:
      app: api

  template:
    metadata:
      labels:
        app: api
    spec:
      affinity:
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - api
              topologyKey: kubernetes.io/hostname

      containers:
      - name: api
        image: api-server:latest
```

## 拓扑域(topologyKey)

拓扑域定义了亲和性/反亲和性的作用范围。

### 常用拓扑键

```yaml
# 节点级别(同一主机)
topologyKey: kubernetes.io/hostname

# 可用区级别
topologyKey: topology.kubernetes.io/zone

# 区域级别
topologyKey: topology.kubernetes.io/region

# 自定义拓扑
topologyKey: custom-label/rack
```

### 拓扑域示例

**场景 1: 同节点部署**
```yaml
podAffinity:
  requiredDuringSchedulingIgnoredDuringExecution:
  - labelSelector:
      matchLabels:
        app: database
    topologyKey: kubernetes.io/hostname  # 同一节点
```

**场景 2: 同可用区但不同节点**
```yaml
spec:
  affinity:
    # 亲和性: 同一可用区
    podAffinity:
      requiredDuringSchedulingIgnoredDuringExecution:
      - labelSelector:
          matchLabels:
            app: backend
        topologyKey: topology.kubernetes.io/zone

    # 反亲和性: 不同节点
    podAntiAffinity:
      requiredDuringSchedulingIgnoredDuringExecution:
      - labelSelector:
          matchLabels:
            app: frontend
        topologyKey: kubernetes.io/hostname
```

**场景 3: 跨可用区分散**
```yaml
podAntiAffinity:
  preferredDuringSchedulingIgnoredDuringExecution:
  - weight: 100
    podAffinityTerm:
      labelSelector:
        matchLabels:
          app: critical-app
      topologyKey: topology.kubernetes.io/zone  # 不同可用区
```

## 标签选择器

### matchLabels (精确匹配)

```yaml
labelSelector:
  matchLabels:
    app: backend
    tier: database
    version: v1.0
```

等价于 AND 条件: `app=backend AND tier=database AND version=v1.0`

### matchExpressions (表达式匹配)

```yaml
labelSelector:
  matchExpressions:
  # In: 在列表中
  - key: app
    operator: In
    values:
    - backend
    - api

  # NotIn: 不在列表中
  - key: env
    operator: NotIn
    values:
    - test
    - dev

  # Exists: 键存在
  - key: tier
    operator: Exists

  # DoesNotExist: 键不存在
  - key: deprecated
    operator: DoesNotExist
```

### 组合使用

```yaml
labelSelector:
  # AND 关系
  matchLabels:
    app: backend
  matchExpressions:
  - key: version
    operator: In
    values:
    - v1.0
    - v2.0
  - key: env
    operator: NotIn
    values:
    - test
```

## 实战案例

### 案例 1: 前后端共同定位

```yaml
# 后端 Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: backend
spec:
  replicas: 3
  selector:
    matchLabels:
      app: backend
      tier: api

  template:
    metadata:
      labels:
        app: backend
        tier: api
    spec:
      containers:
      - name: backend
        image: backend:latest
        ports:
        - containerPort: 8080
---
# 前端 Deployment (与后端共同定位)
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
      affinity:
        # 亲和性: 与后端在同一节点
        podAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - backend
              topologyKey: kubernetes.io/hostname

        # 反亲和性: 前端 Pod 分散
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - frontend
              topologyKey: kubernetes.io/hostname

      containers:
      - name: frontend
        image: frontend:latest
        ports:
        - containerPort: 80
```

### 案例 2: 高可用部署(跨可用区)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: critical-service
spec:
  replicas: 6
  selector:
    matchLabels:
      app: critical

  template:
    metadata:
      labels:
        app: critical
    spec:
      affinity:
        # 反亲和性: 分散到不同节点
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: app
                operator: In
                values:
                - critical
            topologyKey: kubernetes.io/hostname

          # 偏好: 分散到不同可用区
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - critical
              topologyKey: topology.kubernetes.io/zone

      containers:
      - name: app
        image: critical-app:latest
        resources:
          requests:
            memory: "512Mi"
            cpu: "500m"
          limits:
            memory: "1Gi"
            cpu: "1000m"

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

### 案例 3: 数据库和缓存共同定位

```yaml
# Redis 缓存
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: redis
spec:
  serviceName: redis
  replicas: 3
  selector:
    matchLabels:
      app: redis

  template:
    metadata:
      labels:
        app: redis
        component: cache
    spec:
      containers:
      - name: redis
        image: redis:6
        ports:
        - containerPort: 6379
---
# 应用服务器 (与 Redis 共同定位)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-server
spec:
  replicas: 3
  selector:
    matchLabels:
      app: app-server

  template:
    metadata:
      labels:
        app: app-server
    spec:
      affinity:
        # 亲和性: 与 Redis 在同一节点
        podAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: component
                operator: In
                values:
                - cache
            topologyKey: kubernetes.io/hostname

      containers:
      - name: app
        image: app-server:latest
        env:
        - name: REDIS_HOST
          value: "localhost"  # 同节点,低延迟
        - name: REDIS_PORT
          value: "6379"
```

### 案例 4: 多层应用拓扑

```yaml
# Layer 1: 数据库 (分散部署)
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: database
spec:
  serviceName: database
  replicas: 3
  selector:
    matchLabels:
      tier: database

  template:
    metadata:
      labels:
        tier: database
        app: myapp
    spec:
      affinity:
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: tier
                operator: In
                values:
                - database
            topologyKey: topology.kubernetes.io/zone

      containers:
      - name: postgres
        image: postgres:13
        ports:
        - containerPort: 5432
---
# Layer 2: API 服务 (与数据库亲和,自身分散)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: api-service
spec:
  replicas: 6
  selector:
    matchLabels:
      tier: backend
      app: myapp

  template:
    metadata:
      labels:
        tier: backend
        app: myapp
    spec:
      affinity:
        # 亲和性: 偏好与数据库同区域
        podAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: tier
                  operator: In
                  values:
                  - database
              topologyKey: topology.kubernetes.io/zone

        # 反亲和性: API 服务分散到不同节点
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: tier
                  operator: In
                  values:
                  - backend
              topologyKey: kubernetes.io/hostname

      containers:
      - name: api
        image: api-service:latest
---
# Layer 3: 前端 (与 API 亲和)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: frontend
spec:
  replicas: 6
  selector:
    matchLabels:
      tier: frontend
      app: myapp

  template:
    metadata:
      labels:
        tier: frontend
        app: myapp
    spec:
      affinity:
        # 亲和性: 与 API 服务同节点
        podAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: tier
                  operator: In
                  values:
                  - backend
              topologyKey: kubernetes.io/hostname

        # 反亲和性: 前端分散
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: tier
                  operator: In
                  values:
                  - frontend
              topologyKey: kubernetes.io/hostname

      containers:
      - name: nginx
        image: nginx:1.21
```

### 案例 5: Kafka 和 Zookeeper 拓扑

```yaml
# ZooKeeper (反亲和,分散部署)
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: zookeeper
spec:
  serviceName: zookeeper
  replicas: 3
  selector:
    matchLabels:
      app: zookeeper

  template:
    metadata:
      labels:
        app: zookeeper
        component: coordination
    spec:
      affinity:
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: app
                operator: In
                values:
                - zookeeper
            topologyKey: kubernetes.io/hostname

      containers:
      - name: zookeeper
        image: zookeeper:3.7
        ports:
        - containerPort: 2181
---
# Kafka (与 ZooKeeper 亲和,自身分散)
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: kafka
spec:
  serviceName: kafka
  replicas: 3
  selector:
    matchLabels:
      app: kafka

  template:
    metadata:
      labels:
        app: kafka
        component: messaging
    spec:
      affinity:
        # 亲和性: 偏好与 ZooKeeper 同节点
        podAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: component
                  operator: In
                  values:
                  - coordination
              topologyKey: kubernetes.io/hostname

        # 反亲和性: Kafka broker 分散
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchExpressions:
              - key: app
                operator: In
                values:
                - kafka
            topologyKey: kubernetes.io/hostname

      containers:
      - name: kafka
        image: kafka:3.0
        ports:
        - containerPort: 9092
```

## 最佳实践

### 1. 优先使用 preferredDuringScheduling

```yaml
# ✅ 推荐: 软性偏好
affinity:
  podAntiAffinity:
    preferredDuringSchedulingIgnoredDuringExecution:
    - weight: 100
      podAffinityTerm:
        labelSelector:
          matchLabels:
            app: myapp
        topologyKey: kubernetes.io/hostname

# ❌ 避免: 过度使用硬性要求
affinity:
  podAntiAffinity:
    requiredDuringSchedulingIgnoredDuringExecution:
    - labelSelector:
        matchLabels:
          app: myapp
      topologyKey: kubernetes.io/hostname
```

**原因:**
- 硬性要求可能导致 Pod 无法调度
- 软性偏好更灵活,可以根据资源情况调整
- 在节点数量有限时,硬性反亲和可能导致死锁

### 2. 合理设置权重

```yaml
affinity:
  podAntiAffinity:
    preferredDuringSchedulingIgnoredDuringExecution:
    # 优先级1: 不同节点 (权重高)
    - weight: 100
      podAffinityTerm:
        labelSelector:
          matchLabels:
            app: myapp
        topologyKey: kubernetes.io/hostname

    # 优先级2: 不同可用区 (权重较低)
    - weight: 50
      podAffinityTerm:
        labelSelector:
          matchLabels:
            app: myapp
        topologyKey: topology.kubernetes.io/zone
```

### 3. 标签策略

```yaml
metadata:
  labels:
    # 应用标识
    app: myapp
    # 组件类型
    component: backend
    # 版本
    version: v1.0
    # 环境
    env: production
    # 团队
    team: platform
```

在亲和性中使用:
```yaml
labelSelector:
  matchLabels:
    app: myapp
    component: backend
  matchExpressions:
  - key: env
    operator: In
    values:
    - production
    - staging
```

### 4. 结合拓扑分布约束

从 Kubernetes 1.19+ 开始,推荐使用 topologySpreadConstraints:

```yaml
spec:
  topologySpreadConstraints:
  # 跨节点均匀分布
  - maxSkew: 1
    topologyKey: kubernetes.io/hostname
    whenUnsatisfiable: DoNotSchedule
    labelSelector:
      matchLabels:
        app: myapp

  # 跨可用区均匀分布
  - maxSkew: 1
    topologyKey: topology.kubernetes.io/zone
    whenUnsatisfiable: ScheduleAnyway
    labelSelector:
      matchLabels:
        app: myapp
```

### 5. 性能考虑

```yaml
# ❌ 避免: 过于复杂的选择器
labelSelector:
  matchExpressions:
  - key: label1
    operator: In
    values: [v1, v2, v3, v4, v5]
  - key: label2
    operator: NotIn
    values: [x1, x2, x3]
  - key: label3
    operator: Exists
  # ... 更多条件

# ✅ 推荐: 简洁的选择器
labelSelector:
  matchLabels:
    app: myapp
    tier: backend
```

## 常见问题

### Q1: Pod 一直 Pending,提示亲和性不满足?

**问题:**
```
Events:
  Warning  FailedScheduling  0/3 nodes are available: 3 node(s) didn't match pod affinity rules.
```

**排查步骤:**
```bash
# 1. 查看 Pod 的亲和性配置
kubectl get pod <pod-name> -o yaml | grep -A 20 affinity

# 2. 查看目标 Pod 是否存在
kubectl get pods -l app=backend

# 3. 查看目标 Pod 的节点分布
kubectl get pods -l app=backend -o wide

# 4. 检查拓扑键是否正确
kubectl get nodes --show-labels | grep topology
```

**解决方法:**
```yaml
# 方法1: 改为软性偏好
preferredDuringSchedulingIgnoredDuringExecution:
  - weight: 100
    podAffinityTerm: ...

# 方法2: 确保目标 Pod 存在
kubectl scale deployment backend --replicas=1

# 方法3: 检查标签选择器
labelSelector:
  matchLabels:
    app: backend  # 确保标签正确
```

### Q2: 反亲和性导致 Pod 无法调度?

**场景:**
```yaml
replicas: 5  # 5 个副本
podAntiAffinity:
  requiredDuringSchedulingIgnoredDuringExecution:
  - ...
    topologyKey: kubernetes.io/hostname
```

如果只有 3 个节点,第 4、5 个 Pod 会 Pending。

**解决方法:**
```yaml
# 方法1: 改为软性偏好
preferredDuringSchedulingIgnoredDuringExecution:
- weight: 100
  podAffinityTerm:
    labelSelector:
      matchLabels:
        app: myapp
    topologyKey: kubernetes.io/hostname

# 方法2: 使用更粗粒度的拓扑键
topologyKey: topology.kubernetes.io/zone  # 可用区级别
```

### Q3: 如何实现"尽量分散但允许共存"?

```yaml
affinity:
  podAntiAffinity:
    # 硬性要求: 不同可用区
    requiredDuringSchedulingIgnoredDuringExecution:
    - labelSelector:
        matchLabels:
          app: myapp
      topologyKey: topology.kubernetes.io/zone

    # 软性偏好: 不同节点
    preferredDuringSchedulingIgnoredDuringExecution:
    - weight: 100
      podAffinityTerm:
        labelSelector:
          matchLabels:
            app: myapp
        topologyKey: kubernetes.io/hostname
```

### Q4: 亲和性 vs 反亲和性 vs nodeSelector 如何选择?

| 需求 | 使用方案 |
|------|---------|
| 选择特定节点 | nodeSelector 或 nodeAffinity |
| Pod 共同定位 | podAffinity |
| Pod 分散部署 | podAntiAffinity |
| 避免特定节点 | nodeAffinity(NotIn) 或污点 |

### Q5: IgnoredDuringExecution 是什么意思?

```
requiredDuringSchedulingIgnoredDuringExecution
                              ^^^^^^^^^^^^^^^^^^^^
```

- **DuringScheduling**: 调度时必须满足
- **IgnoredDuringExecution**: 运行时忽略

即:只在调度时生效,Pod 运行后即使条件不满足也不会被驱逐。

## 总结

- ✅ 理解 Pod 亲和性和反亲和性的区别
- ✅ 掌握硬性要求和软性偏好的使用
- ✅ 熟悉拓扑域和标签选择器
- ✅ 能够实现共同定位和高可用部署
- ✅ 了解最佳实践和性能优化

## 参考资源

- [亲和性和反亲和性官方文档](https://kubernetes.io/zh-cn/docs/concepts/scheduling-eviction/assign-pod-node/#affinity-and-anti-affinity)
- [拓扑分布约束](https://kubernetes.io/zh-cn/docs/concepts/scheduling-eviction/topology-spread-constraints/)
- [标签和选择器](https://kubernetes.io/zh-cn/docs/concepts/overview/working-with-objects/labels/)

---

*更新日期: 2025-12-03*
