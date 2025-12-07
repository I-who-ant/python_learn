# DaemonSet

> 节点守护进程、使用场景、更新策略

## 概述

【本文档是 Kubernetes 知识体系的一部分】

DaemonSet 确保集群中的每个(或部分)节点上都运行一个 Pod 副本。当节点加入集群时,会自动创建 Pod;节点离开时,Pod 会被回收。

## 核心概念

### 什么是 DaemonSet

DaemonSet 用于在集群的每个节点上部署系统级别的守护进程。

**核心特性:**
- **节点全覆盖**: 每个节点运行一个 Pod
- **自动调度**: 新节点自动创建 Pod
- **节点选择**: 可限定特定节点
- **系统级服务**: 日志、监控、网络等基础服务

### 为什么需要

在生产环境中,我们需要 DaemonSet 来部署:

1. **日志收集**: Fluentd、Filebeat、Logstash
2. **监控代理**: Node Exporter、Datadog Agent
3. **网络插件**: Calico、Flannel、Weave Net
4. **存储插件**: Ceph、GlusterFS
5. **安全代理**: 入侵检测、安全扫描

### DaemonSet vs Deployment

| 特性 | DaemonSet | Deployment |
|------|----------|-----------|
| 调度方式 | 每个节点一个 Pod | 随机分配 |
| 副本数 | 节点数量 | 手动指定 |
| 使用场景 | 系统守护进程 | 应用服务 |
| 节点选择 | 支持节点选择器 | 通过亲和性控制 |

## 基本使用

### 完整示例

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: fluentd-elasticsearch
  namespace: kube-system
  labels:
    app: fluentd
spec:
  # 选择器
  selector:
    matchLabels:
      app: fluentd

  # Pod 模板
  template:
    metadata:
      labels:
        app: fluentd
    spec:
      # 容忍控制平面污点 (可选)
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        effect: NoSchedule
      - key: node-role.kubernetes.io/master
        effect: NoSchedule

      containers:
      - name: fluentd
        image: fluent/fluentd-kubernetes-daemonset:v1-debian-elasticsearch
        env:
        - name: FLUENT_ELASTICSEARCH_HOST
          value: "elasticsearch-logging"
        - name: FLUENT_ELASTICSEARCH_PORT
          value: "9200"

        resources:
          requests:
            cpu: 100m
            memory: 200Mi
          limits:
            cpu: 200m
            memory: 400Mi

        volumeMounts:
        - name: varlog
          mountPath: /var/log
        - name: varlibdockercontainers
          mountPath: /var/lib/docker/containers
          readOnly: true

      # 挂载主机目录
      volumes:
      - name: varlog
        hostPath:
          path: /var/log
      - name: varlibdockercontainers
        hostPath:
          path: /var/lib/docker/containers
```

### kubectl 命令

```bash
# 创建 DaemonSet
kubectl apply -f daemonset.yaml

# 查看 DaemonSet
kubectl get daemonsets
kubectl get ds  # 简写

# 查看详情
kubectl describe daemonset fluentd-elasticsearch -n kube-system

# 查看 Pod 分布
kubectl get pods -o wide -l app=fluentd -n kube-system

# 更新 DaemonSet
kubectl set image daemonset/fluentd-elasticsearch fluentd=fluent/fluentd:v2 -n kube-system

# 查看更新状态
kubectl rollout status daemonset/fluentd-elasticsearch -n kube-system

# 回滚
kubectl rollout undo daemonset/fluentd-elasticsearch -n kube-system

# 删除 DaemonSet
kubectl delete daemonset fluentd-elasticsearch -n kube-system
```

## 节点选择

### nodeSelector

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: monitoring-agent
spec:
  selector:
    matchLabels:
      app: monitoring
  template:
    metadata:
      labels:
        app: monitoring
    spec:
      # 只在带有特定标签的节点运行
      nodeSelector:
        disktype: ssd

      containers:
      - name: agent
        image: monitoring-agent:latest
```

使用标签:
```bash
# 给节点打标签
kubectl label nodes node1 disktype=ssd
kubectl label nodes node2 disktype=ssd

# 查看节点标签
kubectl get nodes --show-labels

# 删除标签
kubectl label nodes node1 disktype-
```

### 节点亲和性

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: network-plugin
spec:
  selector:
    matchLabels:
      app: network
  template:
    metadata:
      labels:
        app: network
    spec:
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: kubernetes.io/os
                operator: In
                values:
                - linux
              - key: node-role.kubernetes.io/worker
                operator: Exists
      containers:
      - name: network
        image: network-plugin:latest
```

### 污点和容忍度

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: system-daemon
spec:
  selector:
    matchLabels:
      app: system
  template:
    metadata:
      labels:
        app: system
    spec:
      # 容忍特定污点
      tolerations:
      # 容忍控制平面节点
      - key: node-role.kubernetes.io/control-plane
        effect: NoSchedule
      # 容忍主节点
      - key: node-role.kubernetes.io/master
        effect: NoSchedule
      # 容忍所有污点
      - operator: Exists

      containers:
      - name: daemon
        image: system-daemon:latest
```

## 更新策略

### RollingUpdate (滚动更新)

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: monitoring-agent
spec:
  selector:
    matchLabels:
      app: monitoring

  # 更新策略
  updateStrategy:
    type: RollingUpdate
    rollingUpdate:
      maxUnavailable: 1  # 最多1个节点的Pod不可用

  template:
    metadata:
      labels:
        app: monitoring
    spec:
      containers:
      - name: agent
        image: monitoring-agent:v2
```

### OnDelete (手动更新)

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: storage-plugin
spec:
  selector:
    matchLabels:
      app: storage

  updateStrategy:
    type: OnDelete  # 手动删除 Pod 触发更新

  template:
    metadata:
      labels:
        app: storage
    spec:
      containers:
      - name: plugin
        image: storage-plugin:v2
```

手动更新流程:
```bash
# 更新 DaemonSet 配置
kubectl apply -f daemonset.yaml

# 逐个删除 Pod 触发更新
kubectl delete pod storage-plugin-xxxxx
kubectl delete pod storage-plugin-yyyyy
```

## 实战案例

### 案例 1: Node Exporter (监控)

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: node-exporter
  namespace: monitoring
  labels:
    app: node-exporter
spec:
  selector:
    matchLabels:
      app: node-exporter

  updateStrategy:
    type: RollingUpdate

  template:
    metadata:
      labels:
        app: node-exporter
    spec:
      hostNetwork: true
      hostPID: true

      tolerations:
      - effect: NoSchedule
        operator: Exists

      containers:
      - name: node-exporter
        image: prom/node-exporter:v1.5.0
        args:
        - --path.procfs=/host/proc
        - --path.sysfs=/host/sys
        - --path.rootfs=/host/root
        - --collector.filesystem.mount-points-exclude=^/(sys|proc|dev|host|etc)($$|/)

        ports:
        - containerPort: 9100
          hostPort: 9100
          name: metrics

        resources:
          requests:
            memory: 30Mi
            cpu: 100m
          limits:
            memory: 50Mi
            cpu: 200m

        volumeMounts:
        - name: proc
          mountPath: /host/proc
          readOnly: true
        - name: sys
          mountPath: /host/sys
          readOnly: true
        - name: root
          mountPath: /host/root
          readOnly: true

      volumes:
      - name: proc
        hostPath:
          path: /proc
      - name: sys
        hostPath:
          path: /sys
      - name: root
        hostPath:
          path: /
```

### 案例 2: Fluentd (日志收集)

```yaml
apiVersion: v1
kind: ServiceAccount
metadata:
  name: fluentd
  namespace: kube-system
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: fluentd
rules:
- apiGroups: [""]
  resources: ["pods", "namespaces"]
  verbs: ["get", "list", "watch"]
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: fluentd
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: fluentd
subjects:
- kind: ServiceAccount
  name: fluentd
  namespace: kube-system
---
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: fluentd
  namespace: kube-system
  labels:
    app: fluentd
spec:
  selector:
    matchLabels:
      app: fluentd

  template:
    metadata:
      labels:
        app: fluentd
    spec:
      serviceAccountName: fluentd
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        effect: NoSchedule

      containers:
      - name: fluentd
        image: fluent/fluentd-kubernetes-daemonset:v1-debian-elasticsearch
        env:
        - name: FLUENT_ELASTICSEARCH_HOST
          value: "elasticsearch"
        - name: FLUENT_ELASTICSEARCH_PORT
          value: "9200"
        - name: FLUENT_ELASTICSEARCH_SCHEME
          value: "http"

        resources:
          requests:
            cpu: 100m
            memory: 200Mi
          limits:
            cpu: 500m
            memory: 500Mi

        volumeMounts:
        - name: varlog
          mountPath: /var/log
        - name: varlibdockercontainers
          mountPath: /var/lib/docker/containers
          readOnly: true
        - name: config
          mountPath: /fluentd/etc/fluent.conf
          subPath: fluent.conf

      volumes:
      - name: varlog
        hostPath:
          path: /var/log
      - name: varlibdockercontainers
        hostPath:
          path: /var/lib/docker/containers
      - name: config
        configMap:
          name: fluentd-config
```

### 案例 3: Calico (网络插件)

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: calico-node
  namespace: kube-system
  labels:
    k8s-app: calico-node
spec:
  selector:
    matchLabels:
      k8s-app: calico-node

  updateStrategy:
    type: RollingUpdate
    rollingUpdate:
      maxUnavailable: 1

  template:
    metadata:
      labels:
        k8s-app: calico-node
    spec:
      hostNetwork: true
      tolerations:
      - effect: NoSchedule
        operator: Exists
      - key: CriticalAddonsOnly
        operator: Exists
      - effect: NoExecute
        operator: Exists

      serviceAccountName: calico-node
      priorityClassName: system-node-critical

      containers:
      - name: calico-node
        image: calico/node:v3.25.0
        env:
        - name: DATASTORE_TYPE
          value: "kubernetes"
        - name: CALICO_NETWORKING_BACKEND
          value: "bird"
        - name: CLUSTER_TYPE
          value: "k8s,bgp"
        - name: IP
          value: "autodetect"
        - name: CALICO_IPV4POOL_IPIP
          value: "Always"

        securityContext:
          privileged: true

        resources:
          requests:
            cpu: 250m
            memory: 512Mi

        livenessProbe:
          httpGet:
            path: /liveness
            port: 9099
          periodSeconds: 10
          initialDelaySeconds: 10

        readinessProbe:
          httpGet:
            path: /readiness
            port: 9099
          periodSeconds: 10

        volumeMounts:
        - name: lib-modules
          mountPath: /lib/modules
          readOnly: true
        - name: var-run-calico
          mountPath: /var/run/calico
        - name: var-lib-calico
          mountPath: /var/lib/calico
        - name: xtables-lock
          mountPath: /run/xtables.lock

      volumes:
      - name: lib-modules
        hostPath:
          path: /lib/modules
      - name: var-run-calico
        hostPath:
          path: /var/run/calico
      - name: var-lib-calico
        hostPath:
          path: /var/lib/calico
      - name: xtables-lock
        hostPath:
          path: /run/xtables.lock
          type: FileOrCreate
```

## 最佳实践

### 1. 资源限制

```yaml
resources:
  requests:
    cpu: 100m
    memory: 200Mi
  limits:
    cpu: 200m
    memory: 400Mi
```

### 2. 健康检查

```yaml
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
  initialDelaySeconds: 5
  periodSeconds: 5
```

### 3. 优先级

```yaml
spec:
  template:
    spec:
      priorityClassName: system-node-critical  # 系统级优先级
```

### 4. 安全上下文

```yaml
securityContext:
  privileged: true  # 仅在必要时使用
  capabilities:
    add: ["NET_ADMIN", "SYS_TIME"]
```

### 5. 节点选择

```yaml
tolerations:
- effect: NoSchedule
  operator: Exists

nodeSelector:
  kubernetes.io/os: linux
```

## 常见问题

### Q1: DaemonSet Pod 未在所有节点运行?

**排查:**
```bash
# 查看 DaemonSet 状态
kubectl get daemonset -n kube-system

# 查看 Pod 分布
kubectl get pods -o wide -l app=myapp -n kube-system

# 检查节点污点
kubectl describe nodes | grep Taints

# 查看事件
kubectl describe daemonset myapp -n kube-system
```

**常见原因:**
- 节点有污点,DaemonSet 没有相应的容忍度
- nodeSelector 不匹配
- 资源不足

### Q2: 如何只在工作节点运行?

```yaml
spec:
  template:
    spec:
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: node-role.kubernetes.io/control-plane
                operator: DoesNotExist
```

### Q3: 如何进行灰度更新?

使用 OnDelete 策略:
```yaml
updateStrategy:
  type: OnDelete
```

然后手动控制:
```bash
# 在一个节点上测试
kubectl delete pod daemonset-pod-on-node1

# 验证没问题后,继续其他节点
kubectl delete pod daemonset-pod-on-node2
```

### Q4: DaemonSet vs Deployment 如何选择?

**使用 DaemonSet:**
- 每个节点需要一个实例
- 日志、监控、网络等基础设施
- 需要访问主机资源

**使用 Deployment:**
- 应用服务
- 需要多副本负载均衡
- 不关心运行在哪个节点

## 总结

- ✅ 理解 DaemonSet 的作用和使用场景
- ✅ 掌握节点选择和污点容忍
- ✅ 了解更新策略和滚动更新
- ✅ 能够部署监控、日志等系统服务
- ✅ 掌握故障排查方法

## 参考资源

- [DaemonSet 官方文档](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/daemonset/)
- [污点和容忍度](https://kubernetes.io/zh-cn/docs/concepts/scheduling-eviction/taint-and-toleration/)
- [节点亲和性](https://kubernetes.io/zh-cn/docs/concepts/scheduling-eviction/assign-pod-node/)

---

*更新日期: 2025-12-03*
