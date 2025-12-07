# PV和PVC

> PersistentVolume、PersistentVolumeClaim、生命周期

## 概述

【本文档是 Kubernetes 知识体系的一部分】

PV (PersistentVolume) 和 PVC (PersistentVolumeClaim) 是 Kubernetes 中实现存储资源管理的核心机制。PV 是集群级别的存储资源,PVC 是用户对存储资源的请求。这种设计将存储的提供者和使用者解耦,便于集中管理存储资源。

## 核心概念

### 什么是 PV 和 PVC

**PersistentVolume (PV)**:
- 集群级别的存储资源
- 由管理员创建或动态制备
- 独立于 Pod 生命周期
- 包含存储的具体实现细节

**PersistentVolumeClaim (PVC)**:
- 命名空间级别的存储请求
- 由用户创建
- 声明所需的存储大小和访问模式
- 绑定到满足条件的 PV

### 为什么需要 PV/PVC

在生产环境中,我们需要 PV/PVC 来解决以下问题:

1. **存储抽象**: 用户无需了解底层存储细节
2. **资源复用**: 存储资源可以被多次使用
3. **动态制备**: 自动创建存储资源
4. **数据持久化**: 数据独立于 Pod 生命周期
5. **集中管理**: 统一管理集群存储资源

### PV/PVC 工作流程

\`\`\`
1. 管理员创建 PV (或通过 StorageClass 动态创建)
          ↓
2. 用户创建 PVC 请求存储
          ↓
3. Kubernetes 绑定 PVC 到合适的 PV
          ↓
4. Pod 通过 PVC 使用存储
          ↓
5. 删除 PVC,根据回收策略处理 PV
\`\`\`

## PV 详解

### PV 配置示例

\`\`\`yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: pv-nfs
spec:
  capacity:
    storage: 10Gi
  volumeMode: Filesystem
  accessModes:
    - ReadWriteMany
  persistentVolumeReclaimPolicy: Retain
  storageClassName: nfs-storage
  nfs:
    server: 192.168.1.100
    path: /exports/data
\`\`\`

### 访问模式 (AccessModes)

| 模式 | 缩写 | 说明 |
|------|------|------|
| ReadWriteOnce | RWO | 单节点读写 |
| ReadOnlyMany | ROX | 多节点只读 |
| ReadWriteMany | RWX | 多节点读写 |
| ReadWriteOncePod | RWOP | 单 Pod 读写(1.22+) |

**存储类型支持**:

| 存储类型 | RWO | ROX | RWX |
|---------|-----|-----|-----|
| NFS | ✅ | ✅ | ✅ |
| AWS EBS | ✅ | ❌ | ❌ |
| GCE PD | ✅ | ✅ | ❌ |
| Azure Disk | ✅ | ❌ | ❌ |
| Ceph RBD | ✅ | ✅ | ❌ |
| CephFS | ✅ | ✅ | ✅ |

### 回收策略 (ReclaimPolicy)

| 策略 | 说明 |
|------|------|
| Retain | 保留数据,需手动清理 |
| Delete | 自动删除 PV 和存储 |
| Recycle | 废弃,已移除 |

### 卷模式 (VolumeMode)

- **Filesystem**: 默认,作为文件系统挂载
- **Block**: 作为块设备挂载

### PV 状态

| 状态 | 说明 |
|------|------|
| Available | 可用,未绑定 |
| Bound | 已绑定到 PVC |
| Released | PVC 已删除,但未回收 |
| Failed | 自动回收失败 |

## PVC 详解

### PVC 配置示例

\`\`\`yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: pvc-web
  namespace: default
spec:
  accessModes:
    - ReadWriteOnce
  volumeMode: Filesystem
  resources:
    requests:
      storage: 5Gi
  storageClassName: fast-ssd
  selector:
    matchLabels:
      environment: production
\`\`\`

### PVC 选择器

使用标签选择特定的 PV:

\`\`\`yaml
spec:
  selector:
    matchLabels:
      type: fast
      zone: cn-north-1
    matchExpressions:
    - key: environment
      operator: In
      values:
      - production
      - staging
\`\`\`

### PVC 扩容

某些存储类型支持动态扩容:

\`\`\`yaml
# 1. StorageClass 必须启用扩容
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: expandable
provisioner: kubernetes.io/aws-ebs
allowVolumeExpansion: true

---
# 2. 修改 PVC 容量
spec:
  resources:
    requests:
      storage: 20Gi  # 从 10Gi 扩容到 20Gi
\`\`\`

## kubectl 命令

\`\`\`bash
# PV 操作
kubectl get pv
kubectl describe pv <pv-name>
kubectl delete pv <pv-name>

# PVC 操作
kubectl get pvc
kubectl get pvc -n <namespace>
kubectl describe pvc <pvc-name>
kubectl delete pvc <pvc-name>

# 查看绑定关系
kubectl get pv,pvc

# 查看 PVC 事件
kubectl get events --field-selector involvedObject.name=<pvc-name>

# 查看 PVC 使用情况
kubectl get pvc -o custom-columns=NAME:.metadata.name,CAPACITY:.spec.resources.requests.storage,STATUS:.status.phase

# 编辑 PVC
kubectl edit pvc <pvc-name>

# 强制删除 PVC
kubectl delete pvc <pvc-name> --force --grace-period=0
\`\`\`

## 实战案例

### 案例 1: NFS 静态制备

\`\`\`yaml
# 1. 创建 PV
apiVersion: v1
kind: PersistentVolume
metadata:
  name: pv-nfs-web
  labels:
    type: nfs
    app: web
spec:
  capacity:
    storage: 10Gi
  accessModes:
    - ReadWriteMany
  persistentVolumeReclaimPolicy: Retain
  nfs:
    server: 192.168.1.100
    path: /data/web

---
# 2. 创建 PVC
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: pvc-web
spec:
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 10Gi
  selector:
    matchLabels:
      app: web

---
# 3. Pod 使用 PVC
apiVersion: v1
kind: Pod
metadata:
  name: web-pod
spec:
  containers:
  - name: nginx
    image: nginx
    volumeMounts:
    - name: web-data
      mountPath: /usr/share/nginx/html
  volumes:
  - name: web-data
    persistentVolumeClaim:
      claimName: pvc-web
\`\`\`

### 案例 2: 本地存储

\`\`\`yaml
# 本地存储 PV
apiVersion: v1
kind: PersistentVolume
metadata:
  name: local-pv
spec:
  capacity:
    storage: 100Gi
  volumeMode: Filesystem
  accessModes:
  - ReadWriteOnce
  persistentVolumeReclaimPolicy: Delete
  storageClassName: local-storage
  local:
    path: /mnt/disks/ssd1
  nodeAffinity:
    required:
      nodeSelectorTerms:
      - matchExpressions:
        - key: kubernetes.io/hostname
          operator: In
          values:
          - node1  # 绑定到特定节点
\`\`\`

### 案例 3: StatefulSet 使用 PVC

\`\`\`yaml
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: mysql
spec:
  serviceName: mysql
  replicas: 3
  selector:
    matchLabels:
      app: mysql
  template:
    metadata:
      labels:
        app: mysql
    spec:
      containers:
      - name: mysql
        image: mysql:8.0
        ports:
        - containerPort: 3306
        volumeMounts:
        - name: data
          mountPath: /var/lib/mysql
  # VolumeClaimTemplate 自动为每个副本创建 PVC
  volumeClaimTemplates:
  - metadata:
      name: data
    spec:
      accessModes: ["ReadWriteOnce"]
      storageClassName: fast-ssd
      resources:
        requests:
          storage: 20Gi
\`\`\`

### 案例 4: 块设备存储

\`\`\`yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: block-pv
spec:
  capacity:
    storage: 50Gi
  volumeMode: Block  # 块设备模式
  accessModes:
    - ReadWriteOnce
  persistentVolumeReclaimPolicy: Delete
  storageClassName: fast-block
  awsElasticBlockStore:
    volumeID: vol-0123456789abcdef
    fsType: ext4

---
apiVersion: v1
kind: Pod
metadata:
  name: block-pod
spec:
  containers:
  - name: app
    image: myapp
    volumeDevices:  # 注意是 volumeDevices 而非 volumeMounts
    - name: data
      devicePath: /dev/xvda
  volumes:
  - name: data
    persistentVolumeClaim:
      claimName: block-pvc
\`\`\`

### 案例 5: PV 备份和恢复

\`\`\`bash
# 1. 创建 PV 快照(如果存储支持)
kubectl create -f snapshot.yaml

# snapshot.yaml
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshot
metadata:
  name: mysql-snapshot
spec:
  volumeSnapshotClassName: csi-snapshot-class
  source:
    persistentVolumeClaimName: mysql-pvc

# 2. 从快照恢复
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: mysql-pvc-restore
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: fast-ssd
  dataSource:
    name: mysql-snapshot
    kind: VolumeSnapshot
    apiGroup: snapshot.storage.k8s.io
  resources:
    requests:
      storage: 20Gi
\`\`\`

## 生命周期管理

### 1. 绑定 (Binding)

\`\`\`
PVC 创建 → 查找匹配的 PV → 绑定
\`\`\`

匹配条件:
- 容量满足
- 访问模式匹配
- StorageClass 匹配
- 选择器匹配(如果指定)

### 2. 使用 (Using)

Pod 通过 PVC 使用 PV,PV 状态为 Bound。

### 3. 回收 (Reclaiming)

PVC 删除后,根据回收策略处理:

**Retain (保留)**:
\`\`\`bash
# 手动清理
kubectl delete pv <pv-name>
# 清理底层存储数据
\`\`\`

**Delete (删除)**:
自动删除 PV 和底层存储。

## 最佳实践

1. **生产环境建议**
   - 使用 StorageClass 动态制备
   - 设置合理的回收策略
   - 为重要数据使用 Retain 策略
   - 定期备份数据

2. **性能优化**
   - 根据工作负载选择存储类型
   - 使用 SSD 提升性能
   - 合理设置 IOPS 和吞吐量
   - 避免过度分配存储

3. **安全加固**
   - 使用 RBAC 控制 PVC 访问
   - 加密敏感数据
   - 定期审计存储使用
   - 设置资源配额限制

4. **容量管理**
   - 监控存储使用率
   - 启用 PVC 自动扩容
   - 设置存储配额
   - 及时清理无用 PVC

5. **高可用**
   - 使用支持多节点的存储
   - 配置存储副本
   - 定期测试恢复流程
   - 多可用区部署

## 常见问题

### Q1: PVC 一直处于 Pending 状态?

A: 可能原因:
\`\`\`bash
# 1. 没有匹配的 PV
kubectl get pv

# 2. StorageClass 不存在
kubectl get storageclass

# 3. 容量不足
kubectl describe pvc <pvc-name>

# 4. 访问模式不匹配
kubectl get pv -o custom-columns=NAME:.metadata.name,CAPACITY:.spec.capacity.storage,ACCESS:.spec.accessModes
\`\`\`

解决方法:
- 创建满足条件的 PV
- 检查 StorageClass 配置
- 增加 PV 容量
- 调整访问模式

### Q2: 如何删除被 Pod 使用的 PVC?

A:
\`\`\`bash
# 1. 先删除使用 PVC 的 Pod
kubectl delete pod <pod-name>

# 2. 再删除 PVC
kubectl delete pvc <pvc-name>

# 或者强制删除(谨慎使用)
kubectl delete pvc <pvc-name> --force --grace-period=0
\`\`\`

### Q3: PV 如何重用?

A: 对于 Retain 策略的 PV:
\`\`\`bash
# 1. 删除旧 PVC
kubectl delete pvc <old-pvc>

# 2. 清理 PV 的 claimRef
kubectl patch pv <pv-name> -p '{"spec":{"claimRef": null}}'

# 3. PV 状态变为 Available

# 4. 创建新 PVC 绑定
\`\`\`

### Q4: 如何扩容 PVC?

A:
\`\`\`bash
# 1. 确认 StorageClass 支持扩容
kubectl get sc <sc-name> -o yaml | grep allowVolumeExpansion

# 2. 编辑 PVC
kubectl edit pvc <pvc-name>
# 修改 spec.resources.requests.storage

# 3. 检查扩容状态
kubectl describe pvc <pvc-name>

# 4. 某些存储需要重启 Pod
kubectl delete pod <pod-name>
\`\`\`

### Q5: PV 和 PVC 的区别?

A:
| 特性 | PV | PVC |
|------|----|----|
| **级别** | 集群级别 | 命名空间级别 |
| **创建者** | 管理员 | 用户 |
| **用途** | 提供存储 | 请求存储 |
| **生命周期** | 独立 | 与 Pod 相关 |

## 监控和调试

### 查看 PV/PVC 绑定

\`\`\`bash
# 查看绑定关系
kubectl get pv -o custom-columns=NAME:.metadata.name,CLAIM:.spec.claimRef.name,STATUS:.status.phase

# 查看 PVC 详情
kubectl describe pvc <pvc-name>

# 查看事件
kubectl get events --sort-by='.lastTimestamp' | grep -i persistent
\`\`\`

### 查看存储使用

\`\`\`bash
# 查看 PVC 容量
kubectl get pvc -o custom-columns=NAME:.metadata.name,CAPACITY:.status.capacity.storage,REQUESTED:.spec.resources.requests.storage

# 进入 Pod 查看实际使用
kubectl exec -it <pod-name> -- df -h
\`\`\`

## 总结

- ✅ 理解 PV 和 PVC 的概念和关系
- ✅ 掌握访问模式和回收策略
- ✅ 理解 PV/PVC 生命周期
- ✅ 掌握静态和动态制备
- ✅ 了解最佳实践和故障排查
- ✅ 能够解决常见 PV/PVC 问题

## 参考资源

- [Persistent Volumes 官方文档](https://kubernetes.io/docs/concepts/storage/persistent-volumes/)
- [Storage Classes](https://kubernetes.io/docs/concepts/storage/storage-classes/)
- [Volume Snapshots](https://kubernetes.io/docs/concepts/storage/volume-snapshots/)

---

*更新日期: 2025-12-03*
