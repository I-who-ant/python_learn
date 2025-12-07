# Kubernetes对象

> 声明式 API、对象规范、对象状态

## 什么是 Kubernetes 对象

Kubernetes 对象是持久化的实体，用来表示集群的状态。

### 核心理解

```
K8s 对象 = 意图的记录

你告诉 K8s:
  "我要 3 个 Nginx Pod"

K8s 确保:
  "好的，我会一直保持 3 个 Nginx Pod 运行"
```

## 对象的特征

### 1. 持久性

```yaml
# 创建对象后，K8s 会持续维护它
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx
spec:
  replicas: 3  # K8s 会确保始终有 3 个副本
```

### 2. 描述性

对象描述：
- **期望状态** (Desired State): 你想要什么
- **当前状态** (Current Status): 实际是什么

```
期望状态: replicas: 3
当前状态: replicas: 2  ❌ 不匹配

K8s 行动: 创建 1 个新 Pod
结果: replicas: 3  ✅ 匹配
```

### 3. API 驱动

```bash
# 所有对象都通过 API 管理
kubectl get pods
  ↓
API Server
  ↓
etcd (存储对象)
```

## 对象的结构

### 必需字段

每个 K8s 对象必须包含：

```yaml
apiVersion: apps/v1      # API 版本
kind: Deployment         # 对象类型
metadata:                # 元数据
  name: my-app
  namespace: default
  labels:
    app: myapp
spec:                    # 规格（期望状态）
  replicas: 3
  template:
    spec:
      containers:
      - name: nginx
        image: nginx:latest
```

#### 1. apiVersion

指定 API 版本：

```yaml
# 核心 API
apiVersion: v1
kind: Pod

# apps API 组
apiVersion: apps/v1
kind: Deployment

# batch API 组
apiVersion: batch/v1
kind: Job

# 查看所有 API 版本
kubectl api-versions
```

#### 2. kind

对象类型：

```yaml
# 工作负载
kind: Pod
kind: Deployment
kind: StatefulSet
kind: DaemonSet
kind: Job
kind: CronJob

# 服务和网络
kind: Service
kind: Ingress
kind: NetworkPolicy

# 配置和存储
kind: ConfigMap
kind: Secret
kind: PersistentVolume
kind: PersistentVolumeClaim

# 查看所有类型
kubectl api-resources
```

#### 3. metadata

元数据信息：

```yaml
metadata:
  # 必需
  name: my-app                    # 对象名称
  namespace: production           # 命名空间
  
  # 可选
  labels:                         # 标签
    app: myapp
    version: v1.0
    environment: prod
  
  annotations:                    # 注解
    description: "My application"
    contact: "admin@example.com"
  
  # 系统生成
  uid: "a1b2c3d4-..."           # 唯一ID
  resourceVersion: "12345"        # 版本号
  creationTimestamp: "2025-..."   # 创建时间
```

#### 4. spec

期望状态（不同对象类型不同）：

```yaml
# Pod 的 spec
spec:
  containers:
  - name: nginx
    image: nginx:1.19
    ports:
    - containerPort: 80

# Deployment 的 spec
spec:
  replicas: 3
  selector:
    matchLabels:
      app: myapp
  template:
    # Pod 模板
    spec:
      containers:
      - name: nginx
        image: nginx:1.19

# Service 的 spec
spec:
  selector:
    app: myapp
  ports:
  - port: 80
    targetPort: 80
  type: LoadBalancer
```

#### 5. status

当前状态（系统生成，只读）：

```yaml
# Pod 的 status
status:
  phase: Running
  conditions:
  - type: Ready
    status: "True"
  podIP: "10.244.0.5"
  containerStatuses:
  - name: nginx
    ready: true
    restartCount: 0

# Deployment 的 status
status:
  replicas: 3
  updatedReplicas: 3
  readyReplicas: 3
  availableReplicas: 3
  conditions:
  - type: Available
    status: "True"
```

## 对象管理方式

### 1. 命令式命令

```bash
# 直接使用命令
kubectl run nginx --image=nginx
kubectl expose pod nginx --port=80
kubectl scale deployment nginx --replicas=5

# 优点: 快速简单
# 缺点: 不可追溯，难以版本控制
```

### 2. 命令式对象配置


```bash
# 使用配置文件
kubectl create -f nginx.yaml
kubectl delete -f nginx.yaml
kubectl replace -f nginx.yaml

# 优点: 可追溯，可版本控制
# 缺点: 需要完整配置文件
```


### 3. 声明式对象配置（推荐）

```bash
# 声明期望状态
kubectl apply -f nginx.yaml
kubectl apply -f configs/

# K8s 自动计算差异并应用

# 优点: 
# - GitOps 友好
# - 支持目录操作
# - 智能合并
# - 生产推荐
```

### 对比示例

```bash
# 命令式
kubectl scale deployment nginx --replicas=5

# 声明式
# nginx.yaml
spec:
  replicas: 5

kubectl apply -f nginx.yaml

# 差异:
# 命令式: 告诉 K8s"做什么"
# 声明式: 告诉 K8s"要什么"
```

## 对象的生命周期

### 创建

```bash
# 1. 编写 YAML
cat > pod.yaml << 'EOF'
apiVersion: v1
kind: Pod
metadata:
  name: nginx
spec:
  containers:
  - name: nginx
    image: nginx
EOF

# 2. 创建对象
kubectl apply -f pod.yaml

# 3. 验证
kubectl get pod nginx
kubectl describe pod nginx
```

### 更新

```bash
# 修改 YAML
spec:
  containers:
  - name: nginx
    image: nginx:1.19  # 更新镜像

# 应用更改
kubectl apply -f pod.yaml

# K8s 自动:
# 1. 比较期望状态和当前状态
# 2. 计算需要的变更
# 3. 执行变更
```

### 删除

```bash
# 删除对象
kubectl delete -f pod.yaml
# 或
kubectl delete pod nginx

# 级联删除
kubectl delete deployment nginx
# 自动删除关联的 ReplicaSet 和 Pods
```

## 对象关系

### 属主关系（Owner Reference）

```
Deployment
    ↓ (owns)
ReplicaSet
    ↓ (owns)
Pods
```

```yaml
# Pod 的 metadata 中自动添加
metadata:
  ownerReferences:
  - apiVersion: apps/v1
    kind: ReplicaSet
    name: nginx-7848d4b86f
    uid: a1b2c3...
    controller: true
```

### 标签选择器

```yaml
# Deployment 选择 Pod
spec:
  selector:
    matchLabels:
      app: nginx

# Service 选择 Pod
spec:
  selector:
    app: nginx
```

## 常用对象类型

### 工作负载对象

```yaml
# Pod - 最小部署单元
apiVersion: v1
kind: Pod

# Deployment - 无状态应用
apiVersion: apps/v1
kind: Deployment

# StatefulSet - 有状态应用
apiVersion: apps/v1
kind: StatefulSet

# DaemonSet - 每个节点一个
apiVersion: apps/v1
kind: DaemonSet

# Job - 一次性任务
apiVersion: batch/v1
kind: Job

# CronJob - 定时任务
apiVersion: batch/v1
kind: CronJob
```

### 服务发现对象

```yaml
# Service - 服务暴露
apiVersion: v1
kind: Service

# Ingress - HTTP 路由
apiVersion: networking.k8s.io/v1
kind: Ingress

# EndpointSlice - 端点
apiVersion: discovery.k8s.io/v1
kind: EndpointSlice
```

### 配置对象

```yaml
# ConfigMap - 配置
apiVersion: v1
kind: ConfigMap

# Secret - 密钥
apiVersion: v1
kind: Secret
```

### 存储对象

```yaml
# PersistentVolume - 持久卷
apiVersion: v1
kind: PersistentVolume

# PersistentVolumeClaim - 持久卷声明
apiVersion: v1
kind: PersistentVolumeClaim

# StorageClass - 存储类
apiVersion: storage.k8s.io/v1
kind: StorageClass
```

## 对象命名

### 命名规则

```yaml
metadata:
  name: my-app-v1  # 必须符合 DNS 子域名规范
  
# 规则:
# - 最长 253 字符
# - 只能包含小写字母、数字、'-' 和 '.'
# - 开头和结尾必须是字母或数字
```

### 命名最佳实践

```yaml
# ✅ 好的命名
name: frontend-service
name: mysql-db-v2
name: payment-api

# ❌ 不好的命名
name: Service1
name: test
name: my_app  # 下划线不允许
```

## 对象字段操作

### 查看对象

```bash
# 列表
kubectl get pods
kubectl get deployments
kubectl get all

# 详细信息
kubectl describe pod nginx
kubectl get pod nginx -o yaml
kubectl get pod nginx -o json

# 自定义列
kubectl get pods -o custom-columns=NAME:.metadata.name,IMAGE:.spec.containers[0].image
```

### 编辑对象

```bash
# 直接编辑
kubectl edit deployment nginx

# 打补丁
kubectl patch deployment nginx -p '{"spec":{"replicas":5}}'

# JSON patch
kubectl patch pod nginx --type='json' -p='[{"op": "replace", "path": "/spec/containers/0/image", "value":"nginx:1.20"}]'
```

### 标签操作

```bash
# 添加标签
kubectl label pod nginx env=prod

# 修改标签
kubectl label pod nginx env=staging --overwrite

# 删除标签
kubectl label pod nginx env-

# 基于标签选择
kubectl get pods -l env=prod
kubectl get pods -l 'env in (prod,staging)'
```

## 实战示例

### 完整的应用栈

```yaml
# 1. Namespace
apiVersion: v1
kind: Namespace
metadata:
  name: myapp

---
# 2. ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
  namespace: myapp
data:
  database_url: "postgres://db:5432"

---
# 3. Secret
apiVersion: v1
kind: Secret
metadata:
  name: app-secret
  namespace: myapp
type: Opaque
data:
  db_password: cGFzc3dvcmQ=

---
# 4. Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app
  namespace: myapp
spec:
  replicas: 3
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
        image: myapp:v1.0
        env:
        - name: DATABASE_URL
          valueFrom:
            configMapKeyRef:
              name: app-config
              key: database_url
        - name: DB_PASSWORD
          valueFrom:
            secretKeyRef:
              name: app-secret
              key: db_password

---
# 5. Service
apiVersion: v1
kind: Service
metadata:
  name: app-service
  namespace: myapp
spec:
  selector:
    app: myapp
  ports:
  - port: 80
    targetPort: 8080
  type: LoadBalancer
```

### 部署和管理

```bash
# 创建
kubectl apply -f app.yaml

# 查看
kubectl get all -n myapp

# 监控
kubectl get pods -n myapp --watch

# 日志
kubectl logs -f deployment/app -n myapp

# 清理
kubectl delete -f app.yaml
```

## 总结

### 核心概念
- K8s 对象是集群状态的持久化实体
- 对象包含期望状态和当前状态
- K8s 持续工作使两者保持一致

### 对象结构
```
对象 = apiVersion + kind + metadata + spec
状态 = status (系统生成)
```

### 管理方式
- ✅ 声明式配置（生产推荐）
- ⚠️ 命令式配置（开发调试）
- ❌ 直接命令（快速测试）

---

**下一步**: 学习 [对象管理](./02-对象管理.md)

*更新日期: 2025-12-03*
