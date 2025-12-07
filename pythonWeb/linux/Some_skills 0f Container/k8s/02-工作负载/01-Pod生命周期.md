# Pod生命周期

> Pod 阶段、容器状态、重启策略、生命周期回调

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Pod 的生命周期是理解 Kubernetes 工作负载管理的关键。本文详细介绍 Pod 从创建到终止的完整过程,以及如何控制和优化这个过程。

## 核心概念

### 什么是 Pod 生命周期

Pod 生命周期描述了 Pod 从创建到删除的完整状态转换过程,包括:
- **Pod 阶段 (Phase)**: Pod 的整体状态
- **容器状态**: 单个容器的运行状态
- **Pod 条件 (Condition)**: Pod 的详细状态信息
- **重启策略**: 容器失败后的重启行为

### 为什么需要

在生产环境中,理解 Pod 生命周期帮助我们:

1. **健康管理**: 实时监控 Pod 和容器状态
2. **故障恢复**: 配置合适的重启策略
3. **优雅终止**: 确保应用正常关闭
4. **生命周期钩子**: 在关键时刻执行自定义逻辑

## Pod 阶段

### 五种阶段

Pod 的 `status.phase` 字段描述其当前所处的阶段:

| 阶段 | 说明 |
|------|------|
| **Pending** | Pod 已被接受,但容器镜像未创建 |
| **Running** | Pod 已绑定到节点,所有容器已创建,至少一个容器正在运行 |
| **Succeeded** | Pod 中所有容器已成功终止,且不会重启 |
| **Failed** | Pod 中所有容器已终止,至少一个容器失败 |
| **Unknown** | 无法获取 Pod 状态,通常是通信问题 |

### 状态转换图

```
        [创建 Pod]
            ↓
        ┌─────────┐
        │ Pending │ ← 调度中、拉取镜像
        └─────────┘
            ↓
        ┌─────────┐
        │ Running │ ← 容器运行中
        └─────────┘
          ↙     ↘
    ┌──────────┐  ┌────────┐
    │Succeeded │  │ Failed │
    └──────────┘  └────────┘
```

### 查看 Pod 阶段

```bash
# 查看 Pod 状态
kubectl get pods
kubectl get pod nginx-pod -o jsonpath='{.status.phase}'

# 监控 Pod 状态变化
kubectl get pods -w
```

## 容器状态

### 三种状态

每个容器都有自己的状态,存储在 `status.containerStatuses` 中:

**1. Waiting (等待中)**
```yaml
state:
  waiting:
    reason: ContainerCreating
    message: "拉取镜像中..."
```

**2. Running (运行中)**
```yaml
state:
  running:
    startedAt: "2025-12-03T09:00:00Z"
```

**3. Terminated (已终止)**
```yaml
state:
  terminated:
    exitCode: 0
    reason: Completed
    startedAt: "2025-12-03T09:00:00Z"
    finishedAt: "2025-12-03T09:05:00Z"
```

### 查看容器状态

```bash
# 查看容器详细状态
kubectl describe pod nginx-pod

# 查看容器状态(JSON 格式)
kubectl get pod nginx-pod -o jsonpath='{.status.containerStatuses[*].state}'
```

## Pod 条件

### 四种条件类型

Pod 的 `status.conditions` 提供更详细的状态信息:

| 条件类型 | 说明 |
|---------|------|
| **PodScheduled** | Pod 已被调度到节点 |
| **ContainersReady** | 所有容器已就绪 |
| **Initialized** | 所有 Init 容器已成功完成 |
| **Ready** | Pod 已就绪,可以接收流量 |

### 条件示例

```yaml
status:
  conditions:
  - type: Initialized
    status: "True"
    lastTransitionTime: "2025-12-03T09:00:00Z"
  - type: Ready
    status: "True"
    lastTransitionTime: "2025-12-03T09:00:05Z"
  - type: ContainersReady
    status: "True"
    lastTransitionTime: "2025-12-03T09:00:05Z"
  - type: PodScheduled
    status: "True"
    lastTransitionTime: "2025-12-03T09:00:00Z"
```

## 重启策略

### 三种策略

通过 `spec.restartPolicy` 控制容器失败后的行为:

**Always (默认)**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-restart-always
spec:
  restartPolicy: Always  # 总是重启
  containers:
  - name: app
    image: nginx:1.21
```
- 适用于长期运行的服务
- 容器退出后总是重启

**OnFailure**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-restart-onfailure
spec:
  restartPolicy: OnFailure  # 失败时重启
  containers:
  - name: job
    image: busybox
    command: ['sh', '-c', 'exit 1']
```
- 适用于批处理任务
- 只有非 0 退出码才重启

**Never**
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-restart-never
spec:
  restartPolicy: Never  # 从不重启
  containers:
  - name: task
    image: busybox
    command: ['sh', '-c', 'echo "Done"']
```
- 适用于一次性任务
- 容器退出后不重启

### 重启延迟

Kubernetes 使用指数退避策略重启容器:

```
10s → 20s → 40s → 80s → 160s → 300s (最大)
```

查看重启次数:
```bash
kubectl get pod nginx-pod -o jsonpath='{.status.containerStatuses[0].restartCount}'
```

## 生命周期钩子

### PostStart 钩子

容器启动后立即执行:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: lifecycle-demo
spec:
  containers:
  - name: nginx
    image: nginx:1.21
    lifecycle:
      postStart:
        exec:
          command:
          - sh
          - -c
          - |
            echo "Container started at $(date)" > /var/log/startup.log
            echo "Initializing application..."
```

**注意事项:**
- PostStart 与容器的 ENTRYPOINT 异步执行
- 如果钩子失败,容器会被杀死并根据重启策略重启
- 钩子至少会被调用一次

### PreStop 钩子

容器终止前执行(优雅关闭):

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: graceful-shutdown
spec:
  terminationGracePeriodSeconds: 30  # 终止宽限期
  containers:
  - name: nginx
    image: nginx:1.21
    lifecycle:
      preStop:
        exec:
          command:
          - sh
          - -c
          - |
            echo "Shutting down gracefully..."
            nginx -s quit
            sleep 15
    ports:
    - containerPort: 80
```

**执行流程:**
1. Pod 收到删除请求
2. 执行 PreStop 钩子
3. 等待钩子完成或超时(terminationGracePeriodSeconds)
4. 发送 SIGTERM 信号给容器
5. 等待宽限期结束
6. 如果容器还在运行,发送 SIGKILL 强制终止

### HTTP 钩子

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: http-hook-demo
spec:
  containers:
  - name: app
    image: myapp:1.0
    lifecycle:
      postStart:
        httpGet:
          path: /api/init
          port: 8080
          scheme: HTTP
      preStop:
        httpGet:
          path: /api/shutdown
          port: 8080
          scheme: HTTP
```

## 实战案例

### 案例 1: 监控 Pod 完整生命周期

```bash
# 创建 Pod
kubectl run lifecycle-demo --image=nginx:1.21

# 实时监控
kubectl get pods -w

# 查看详细事件
kubectl describe pod lifecycle-demo

# 查看容器日志
kubectl logs lifecycle-demo

# 删除 Pod(观察优雅终止)
kubectl delete pod lifecycle-demo
```

### 案例 2: 带生命周期钩子的 Web 服务

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: web-app-lifecycle
  labels:
    app: web
spec:
  terminationGracePeriodSeconds: 60
  containers:
  - name: nginx
    image: nginx:1.21
    ports:
    - containerPort: 80

    # 生命周期钩子
    lifecycle:
      postStart:
        exec:
          command:
          - sh
          - -c
          - |
            # 启动后初始化
            echo "[$(date)] Container started" >> /var/log/lifecycle.log
            echo "Warming up cache..."
            sleep 2

      preStop:
        exec:
          command:
          - sh
          - -c
          - |
            # 优雅关闭
            echo "[$(date)] Received shutdown signal" >> /var/log/lifecycle.log
            # 停止接收新请求
            nginx -s quit
            # 等待现有请求处理完成
            sleep 20
            echo "[$(date)] Shutdown complete" >> /var/log/lifecycle.log

    # 健康检查
    livenessProbe:
      httpGet:
        path: /
        port: 80
      initialDelaySeconds: 10
      periodSeconds: 10

    readinessProbe:
      httpGet:
        path: /
        port: 80
      initialDelaySeconds: 5
      periodSeconds: 5

    # 资源限制
    resources:
      requests:
        memory: "128Mi"
        cpu: "250m"
      limits:
        memory: "256Mi"
        cpu: "500m"

    # 挂载日志卷
    volumeMounts:
    - name: logs
      mountPath: /var/log

  volumes:
  - name: logs
    emptyDir: {}
```

部署和测试:
```bash
# 应用配置
kubectl apply -f web-app-lifecycle.yaml

# 查看 Pod 启动过程
kubectl get pods -w

# 查看生命周期日志
kubectl exec web-app-lifecycle -- cat /var/log/lifecycle.log

# 测试优雅关闭
kubectl delete pod web-app-lifecycle
# 在另一个终端查看日志
kubectl logs web-app-lifecycle -f
```

### 案例 3: 数据库 Pod 的备份钩子

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: postgres-with-backup
spec:
  terminationGracePeriodSeconds: 120
  containers:
  - name: postgres
    image: postgres:13
    env:
    - name: POSTGRES_PASSWORD
      value: "mysecretpassword"

    lifecycle:
      preStop:
        exec:
          command:
          - sh
          - -c
          - |
            # 优雅关闭前备份数据库
            echo "Starting backup before shutdown..."
            pg_dump -U postgres mydb > /backup/final-backup-$(date +%Y%m%d-%H%M%S).sql
            echo "Backup completed"
            # 等待备份完成
            sleep 5

    volumeMounts:
    - name: data
      mountPath: /var/lib/postgresql/data
    - name: backup
      mountPath: /backup

  volumes:
  - name: data
    emptyDir: {}
  - name: backup
    hostPath:
      path: /tmp/postgres-backups
      type: DirectoryOrCreate
```

## 最佳实践

### 1. 优雅关闭

✅ **设置合理的终止宽限期**
```yaml
spec:
  terminationGracePeriodSeconds: 60  # 根据应用调整
```

✅ **实现 PreStop 钩子**
```yaml
lifecycle:
  preStop:
    exec:
      command: ["/bin/sh", "-c", "sleep 15; kill -SIGTERM 1"]
```

✅ **应用层处理 SIGTERM**
```python
import signal
import sys

def graceful_shutdown(signum, frame):
    print("Shutting down gracefully...")
    # 完成现有请求
    # 关闭数据库连接
    sys.exit(0)

signal.signal(signal.SIGTERM, graceful_shutdown)
```

### 2. 重启策略选择

| 工作负载类型 | 推荐策略 |
|------------|---------|
| Web 服务 | Always |
| 批处理任务 | OnFailure |
| 一次性任务 | Never |
| 数据库 | Always |

### 3. 生命周期钩子

⚠️ **注意事项:**
- 钩子执行时间计入 terminationGracePeriodSeconds
- PostStart 失败会导致容器重启
- PreStop 超时会被强制终止
- 钩子应该幂等且快速

### 4. 监控和调试

```bash
# 查看 Pod 事件
kubectl get events --field-selector involvedObject.name=pod-name

# 查看容器重启原因
kubectl describe pod pod-name | grep -A 5 "Last State"

# 查看上一次容器日志
kubectl logs pod-name --previous

# 监控 Pod 状态变化
kubectl get pods -w -o wide
```

## 常见问题

### Q1: Pod 一直处于 Pending 状态?

**可能原因:**
- 节点资源不足
- 镜像拉取失败
- PVC 绑定失败
- 节点选择器不匹配

**排查:**
```bash
kubectl describe pod <pod-name>
# 查看 Events 部分的错误信息
```

### Q2: 容器频繁重启?

**可能原因:**
- 应用崩溃
- 健康检查失败
- OOMKilled (内存溢出)
- PostStart 钩子失败

**排查:**
```bash
# 查看退出码和原因
kubectl get pod <pod-name> -o jsonpath='{.status.containerStatuses[0].lastState.terminated}'

# 查看上次日志
kubectl logs <pod-name> --previous
```

常见退出码:
- `0`: 正常退出
- `1`: 应用错误
- `137`: SIGKILL (OOMKilled)
- `143`: SIGTERM (正常终止)

### Q3: PreStop 钩子未执行?

**检查项:**
1. terminationGracePeriodSeconds 是否足够
2. 钩子命令是否正确
3. 容器是否已经崩溃

```bash
kubectl describe pod <pod-name>
# 查看 Events 中的终止信息
```

### Q4: 如何实现零停机更新?

组合使用:
1. **就绪探针**: 确保新 Pod 就绪后才接收流量
2. **PreStop 钩子**: 优雅关闭旧 Pod
3. **滚动更新**: Deployment 的更新策略

```yaml
readinessProbe:
  httpGet:
    path: /health
    port: 8080
  initialDelaySeconds: 5
  periodSeconds: 3

lifecycle:
  preStop:
    exec:
      command: ["/bin/sh", "-c", "sleep 15"]
```

## 总结

- ✅ 理解 Pod 的五个阶段及其转换
- ✅ 掌握容器的三种状态
- ✅ 合理选择重启策略
- ✅ 使用生命周期钩子实现优雅启动和关闭
- ✅ 设置合适的终止宽限期
- ✅ 能够排查 Pod 生命周期问题

## 参考资源

- [Pod 生命周期](https://kubernetes.io/zh-cn/docs/concepts/workloads/pods/pod-lifecycle/)
- [容器生命周期钩子](https://kubernetes.io/zh-cn/docs/concepts/containers/container-lifecycle-hooks/)
- [Pod 阶段](https://kubernetes.io/zh-cn/docs/concepts/workloads/pods/pod-lifecycle/#pod-phase)
- [容器探针](https://kubernetes.io/zh-cn/docs/concepts/workloads/pods/pod-lifecycle/#container-probes)

---

*更新日期: 2025-12-03*
