# Pod详解

> Pod 概念、Pod 网络、Pod 存储、Pod 模板

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Pod 是 Kubernetes 中最小的部署单元,理解 Pod 的概念和设计原理是掌握 K8s 的基础。

## 核心概念

### 什么是 Pod

Pod 是 Kubernetes 中可以创建和管理的最小可部署的计算单元。

**核心特性:**
- **共享资源**: Pod 内的容器共享网络命名空间和存储卷
- **原子调度**: Pod 作为整体被调度到节点上
- **短暂性**: Pod 是临时的,不应该依赖 Pod 的持久性
- **唯一 IP**: 每个 Pod 拥有唯一的 IP 地址

### 为什么需要 Pod

在生产环境中,我们需要 Pod 来解决以下问题:

1. **容器编排单元**: 提供比单个容器更高级的抽象
2. **资源共享**: 让紧密耦合的容器能够共享资源
3. **简化管理**: 将相关容器作为一个整体管理
4. **网络隔离**: 每个 Pod 有独立的网络栈

### Pod 设计模式

**单容器 Pod**
```
┌─────────────┐
│    Pod      │
│  ┌────────┐ │
│  │Container│ │
│  └────────┘ │
└─────────────┘
```

**多容器 Pod (Sidecar 模式)**
```
┌─────────────────────────┐
│         Pod             │
│  ┌────────┐  ┌────────┐ │
│  │  Main  │  │Sidecar │ │
│  │Container│  │Container│ │
│  └────────┘  └────────┘ │
└─────────────────────────┘
```

## Pod 网络

### 网络模型

Kubernetes Pod 遵循以下网络原则:

1. **Pod-to-Pod**: 所有 Pod 可以直接通信,无需 NAT
2. **节点-to-Pod**: 节点可以与所有 Pod 通信
3. **Pod 内容器**: 共享网络命名空间,通过 localhost 通信

### 网络配置示例

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: nginx-pod
spec:
  containers:
  - name: nginx
    image: nginx:1.21
    ports:
    - containerPort: 80
      protocol: TCP
  - name: sidecar
    image: busybox
    command: ['sh', '-c', 'while true; do wget -q -O- localhost:80; sleep 5; done']
```

### 主机网络模式

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: host-network-pod
spec:
  hostNetwork: true  # 使用宿主机网络
  containers:
  - name: nginx
    image: nginx:1.21
```

## Pod 存储

### Volume 类型

**emptyDir - 临时存储**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-with-emptydir
spec:
  containers:
  - name: writer
    image: busybox
    command: ['sh', '-c', 'echo "Hello" > /data/hello.txt; sleep 3600']
    volumeMounts:
    - name: shared-data
      mountPath: /data
  - name: reader
    image: busybox
    command: ['sh', '-c', 'cat /data/hello.txt; sleep 3600']
    volumeMounts:
    - name: shared-data
      mountPath: /data
  volumes:
  - name: shared-data
    emptyDir: {}
```

**hostPath - 挂载主机目录**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-with-hostpath
spec:
  containers:
  - name: nginx
    image: nginx:1.21
    volumeMounts:
    - name: host-logs
      mountPath: /var/log/nginx
  volumes:
  - name: host-logs
    hostPath:
      path: /tmp/nginx-logs
      type: DirectoryOrCreate
```

**configMap - 配置文件**
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
data:
  app.properties: |
    server.port=8080
    app.name=myapp
---
apiVersion: v1
kind: Pod
metadata:
  name: pod-with-configmap
spec:
  containers:
  - name: app
    image: busybox
    command: ['sh', '-c', 'cat /config/app.properties; sleep 3600']
    volumeMounts:
    - name: config-volume
      mountPath: /config
  volumes:
  - name: config-volume
    configMap:
      name: app-config
```

**secret - 敏感数据**
```yaml
apiVersion: v1
kind: Secret
metadata:
  name: db-secret
type: Opaque
stringData:
  username: admin
  password: P@ssw0rd
---
apiVersion: v1
kind: Pod
metadata:
  name: pod-with-secret
spec:
  containers:
  - name: app
    image: busybox
    command: ['sh', '-c', 'echo "User: $(cat /secrets/username)"; sleep 3600']
    volumeMounts:
    - name: secret-volume
      mountPath: /secrets
      readOnly: true
  volumes:
  - name: secret-volume
    secret:
      secretName: db-secret
```

## 基本使用

### 完整 Pod 配置示例

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: web-app
  namespace: default
  labels:
    app: web
    tier: frontend
  annotations:
    description: "Web application pod"
spec:
  # 容器配置
  containers:
  - name: nginx
    image: nginx:1.21
    ports:
    - containerPort: 80
      name: http
      protocol: TCP
    # 环境变量
    env:
    - name: ENV
      value: "production"
    # 资源限制
    resources:
      requests:
        memory: "64Mi"
        cpu: "250m"
      limits:
        memory: "128Mi"
        cpu: "500m"
    # 存活探针
    livenessProbe:
      httpGet:
        path: /healthz
        port: 80
      initialDelaySeconds: 3
      periodSeconds: 10
    # 就绪探针
    readinessProbe:
      httpGet:
        path: /ready
        port: 80
      initialDelaySeconds: 5
      periodSeconds: 5
    # 卷挂载
    volumeMounts:
    - name: config
      mountPath: /etc/nginx/conf.d
    - name: logs
      mountPath: /var/log/nginx

  # 重启策略
  restartPolicy: Always

  # DNS 策略
  dnsPolicy: ClusterFirst

  # 节点选择
  nodeSelector:
    disktype: ssd

  # 卷定义
  volumes:
  - name: config
    configMap:
      name: nginx-config
  - name: logs
    emptyDir: {}
```

### kubectl 命令

```bash
# 创建 Pod
kubectl create -f pod.yaml
kubectl run nginx --image=nginx:1.21

# 查看 Pod
kubectl get pods
kubectl get pods -o wide  # 显示更多信息
kubectl get pods -w       # 监听变化

# Pod 详细信息
kubectl describe pod nginx-pod

# Pod 日志
kubectl logs nginx-pod
kubectl logs nginx-pod -c sidecar  # 多容器 Pod
kubectl logs -f nginx-pod          # 实时查看

# 进入 Pod
kubectl exec -it nginx-pod -- /bin/bash
kubectl exec nginx-pod -- ls /etc  # 执行命令

# 端口转发
kubectl port-forward nginx-pod 8080:80

# 复制文件
kubectl cp nginx-pod:/var/log/nginx/access.log ./access.log

# 删除 Pod
kubectl delete pod nginx-pod
kubectl delete -f pod.yaml
```

## 实战案例

### 案例 1: Web 应用 + 日志收集

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: web-with-logging
spec:
  containers:
  # 主应用容器
  - name: nginx
    image: nginx:1.21
    ports:
    - containerPort: 80
    volumeMounts:
    - name: logs
      mountPath: /var/log/nginx

  # 日志收集 Sidecar
  - name: log-collector
    image: busybox
    command:
    - sh
    - -c
    - |
      while true; do
        if [ -f /logs/access.log ]; then
          tail -f /logs/access.log | grep -E "GET|POST"
        fi
        sleep 5
      done
    volumeMounts:
    - name: logs
      mountPath: /logs

  volumes:
  - name: logs
    emptyDir: {}
```

### 案例 2: 应用 + 配置热更新

```yaml
# ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
data:
  config.json: |
    {
      "server": {
        "port": 8080,
        "host": "0.0.0.0"
      },
      "database": {
        "host": "db.example.com",
        "port": 5432
      }
    }
---
# Pod
apiVersion: v1
kind: Pod
metadata:
  name: app-with-config
spec:
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: config
      mountPath: /app/config
      readOnly: true
    command: ['sh', '-c']
    args:
    - |
      while true; do
        echo "Loading config from /app/config/config.json"
        cat /app/config/config.json
        sleep 30
      done
  volumes:
  - name: config
    configMap:
      name: app-config
```

### 案例 3: 初始化容器 + 主容器

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: init-demo
spec:
  # 初始化容器
  initContainers:
  - name: init-db
    image: busybox
    command: ['sh', '-c']
    args:
    - |
      echo "Waiting for database..."
      until nc -z db-service 5432; do
        echo "Database not ready, waiting..."
        sleep 2
      done
      echo "Database is ready!"

  - name: init-cache
    image: busybox
    command: ['sh', '-c', 'echo "Warming up cache..." && sleep 3']

  # 主容器
  containers:
  - name: app
    image: myapp:1.0
    ports:
    - containerPort: 8080
```

## Pod 资源管理

### 资源请求和限制

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: resource-demo
spec:
  containers:
  - name: app
    image: nginx:1.21
    resources:
      requests:          # 最小保证资源
        memory: "128Mi"
        cpu: "250m"
      limits:           # 最大使用资源
        memory: "256Mi"
        cpu: "500m"
```

### QoS 类别

**Guaranteed (最高优先级)**
```yaml
resources:
  requests:
    memory: "256Mi"
    cpu: "500m"
  limits:
    memory: "256Mi"
    cpu: "500m"
```

**Burstable (中等优先级)**
```yaml
resources:
  requests:
    memory: "128Mi"
    cpu: "250m"
  limits:
    memory: "256Mi"
    cpu: "500m"
```

**BestEffort (最低优先级)**
```yaml
# 不设置 requests 和 limits
```

## 最佳实践

### 1. 生产环境建议

- ✅ **始终设置资源限制**: 防止单个 Pod 占用过多资源
- ✅ **使用标签**: 便于管理和选择
- ✅ **设置健康检查**: liveness 和 readiness 探针
- ✅ **使用命名空间**: 隔离不同环境
- ✅ **避免使用 latest 标签**: 使用具体版本号

```yaml
# 推荐配置
apiVersion: v1
kind: Pod
metadata:
  name: production-app
  labels:
    app: myapp
    env: production
    version: "1.2.3"
spec:
  containers:
  - name: app
    image: myapp:1.2.3  # 明确版本
    resources:
      requests:
        memory: "256Mi"
        cpu: "500m"
      limits:
        memory: "512Mi"
        cpu: "1000m"
    livenessProbe:
      httpGet:
        path: /health
        port: 8080
    readinessProbe:
      httpGet:
        path: /ready
        port: 8080
```

### 2. 性能优化

- 🚀 **合理设置资源**: 避免过度分配或不足
- 🚀 **使用节点亲和性**: 将 Pod 调度到合适的节点
- 🚀 **避免 hostPath**: 除非必要,使用 PV/PVC
- 🚀 **减少容器数量**: 每个 Pod 不要超过 3-4 个容器

### 3. 安全加固

- 🔒 **非 root 用户运行**: 使用 securityContext
- 🔒 **只读根文件系统**: readOnlyRootFilesystem
- 🔒 **限制特权**: 避免 privileged 模式
- 🔒 **使用 Secret**: 存储敏感数据

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secure-pod
spec:
  securityContext:
    runAsNonRoot: true
    runAsUser: 1000
    fsGroup: 2000
  containers:
  - name: app
    image: myapp:1.0
    securityContext:
      allowPrivilegeEscalation: false
      readOnlyRootFilesystem: true
      capabilities:
        drop:
        - ALL
```

### 4. 监控告警

```bash
# 查看资源使用
kubectl top pod nginx-pod

# 查看事件
kubectl get events --field-selector involvedObject.name=nginx-pod

# 监控 Pod 状态
kubectl get pods -w
```

## 常见问题

### Q1: Pod 一直处于 Pending 状态?

**可能原因:**
- 资源不足 (CPU/内存)
- 节点不满足 nodeSelector
- PVC 无法绑定
- 镜像拉取失败

**排查方法:**
```bash
kubectl describe pod <pod-name>
kubectl get events
```

### Q2: Pod 频繁重启?

**可能原因:**
- 应用崩溃
- 健康检查失败
- OOM (内存溢出)

**排查方法:**
```bash
kubectl logs <pod-name> --previous  # 查看上一次日志
kubectl describe pod <pod-name>      # 查看重启原因
```

### Q3: 如何调试运行中的 Pod?

```bash
# 进入容器
kubectl exec -it <pod-name> -- sh

# 复制文件出来
kubectl cp <pod-name>:/path/to/file ./local-file

# 临时运行调试容器
kubectl run -it debug --image=busybox --rm -- sh
```

### Q4: 多容器 Pod 中如何共享数据?

使用共享卷:
```yaml
volumes:
- name: shared-data
  emptyDir: {}
```

每个容器挂载同一个卷到不同路径。

## 总结

- ✅ 理解 Pod 是 K8s 的最小调度单元
- ✅ 掌握 Pod 网络模型和存储方式
- ✅ 熟悉 Pod 生命周期和管理命令
- ✅ 了解资源管理和安全最佳实践
- ✅ 能够设计多容器 Pod 架构
- ✅ 能够排查 Pod 常见问题

## 参考资源

- [Kubernetes Pod 官方文档](https://kubernetes.io/docs/concepts/workloads/pods/)
- [Pod 网络](https://kubernetes.io/docs/concepts/cluster-administration/networking/)
- [Pod 存储](https://kubernetes.io/docs/concepts/storage/volumes/)
- [资源管理](https://kubernetes.io/docs/concepts/configuration/manage-resources-containers/)

---

*更新日期: 2025-12-03*
