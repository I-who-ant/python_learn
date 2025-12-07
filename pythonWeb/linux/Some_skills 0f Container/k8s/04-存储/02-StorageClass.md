# StorageClass

> 动态制备、存储类、默认存储类

## 概述

【本文档是 Kubernetes 知识体系的一部分】

StorageClass 是 Kubernetes 中用于动态制备 PersistentVolume 的资源对象。它定义了存储的类型和参数,当用户创建 PVC 时,Kubernetes 会根据 StorageClass 自动创建对应的 PV,无需管理员手动创建。

## 核心概念

### 什么是 StorageClass

StorageClass 提供了一种描述存储"类"的方法,主要功能包括:

- **动态制备**: 自动创建 PV
- **存储分级**: 定义不同性能的存储类型
- **参数化配置**: 灵活配置存储参数
- **制备器**: 指定使用哪种存储后端

### 为什么需要 StorageClass

在生产环境中,我们需要 StorageClass 来解决以下问题:

1. **简化管理**: 无需手动创建大量 PV
2. **按需分配**: 根据需求动态创建存储
3. **存储分级**: 提供不同性能的存储选项
4. **自动化**: 减少人工操作,提高效率
5. **资源优化**: 按实际需求分配存储

### 工作流程

\`\`\`
1. 管理员创建 StorageClass
          ↓
2. 用户创建 PVC,指定 StorageClass
          ↓
3. Provisioner 自动创建 PV
          ↓
4. PVC 绑定到新创建的 PV
          ↓
5. Pod 使用 PVC
\`\`\`

## 基本配置

### StorageClass 示例

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: fast-ssd
provisioner: kubernetes.io/aws-ebs  # 制备器
parameters:
  type: gp3           # 存储类型
  iopsPerGB: "10"     # IOPS
  fsType: ext4        # 文件系统
volumeBindingMode: WaitForFirstConsumer  # 绑定模式
allowVolumeExpansion: true  # 允许扩容
reclaimPolicy: Delete       # 回收策略
mountOptions:
  - debug
\`\`\`

### 核心参数说明

#### 1. provisioner - 制备器

指定使用哪种存储后端创建 PV。

常见制备器:

| Provisioner | 说明 |
|-------------|------|
| kubernetes.io/aws-ebs | AWS EBS |
| kubernetes.io/gce-pd | Google Cloud Persistent Disk |
| kubernetes.io/azure-disk | Azure Disk |
| kubernetes.io/cinder | OpenStack Cinder |
| kubernetes.io/nfs | NFS (需第三方) |
| kubernetes.io/rbd | Ceph RBD |
| kubernetes.io/cephfs | CephFS |

#### 2. parameters - 参数

不同制备器支持不同参数。

**AWS EBS**:
\`\`\`yaml
parameters:
  type: gp3          # io1, io2, gp2, gp3, sc1, st1
  iopsPerGB: "10"
  fsType: ext4
  encrypted: "true"
\`\`\`

**GCE PD**:
\`\`\`yaml
parameters:
  type: pd-ssd       # pd-standard, pd-ssd, pd-balanced
  replication-type: regional-pd
\`\`\`

**Azure Disk**:
\`\`\`yaml
parameters:
  storageaccounttype: Premium_LRS  # Standard_LRS, Premium_LRS
  kind: Managed
\`\`\`

#### 3. volumeBindingMode - 绑定模式

| 模式 | 说明 |
|------|------|
| Immediate | 立即制备和绑定(默认) |
| WaitForFirstConsumer | 等待首个使用的 Pod 调度后再制备 |

**推荐使用 WaitForFirstConsumer**,避免跨区域问题。

#### 4. reclaimPolicy - 回收策略

| 策略 | 说明 |
|------|------|
| Delete | 删除 PVC 时删除 PV 和存储(默认) |
| Retain | 保留 PV 和存储 |

#### 5. allowVolumeExpansion - 允许扩容

\`\`\`yaml
allowVolumeExpansion: true  # 允许 PVC 扩容
\`\`\`

支持扩容的存储:
- AWS EBS
- GCE PD
- Azure Disk
- Ceph RBD
- 大多数 CSI 驱动

#### 6. mountOptions - 挂载选项

\`\`\`yaml
mountOptions:
  - debug
  - hard
  - nfsvers=4.1
\`\`\`

## kubectl 命令

\`\`\`bash
# 查看 StorageClass
kubectl get storageclass
kubectl get sc

# 查看详细信息
kubectl describe sc <sc-name>

# 查看默认 StorageClass
kubectl get sc -o jsonpath='{.items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=="true")].metadata.name}'

# 设置默认 StorageClass
kubectl patch storageclass <sc-name> -p '{"metadata": {"annotations":{"storageclass.kubernetes.io/is-default-class":"true"}}}'

# 取消默认 StorageClass
kubectl patch storageclass <sc-name> -p '{"metadata": {"annotations":{"storageclass.kubernetes.io/is-default-class":"false"}}}'

# 创建 StorageClass
kubectl apply -f storageclass.yaml

# 删除 StorageClass
kubectl delete sc <sc-name>

# 查看使用 StorageClass 的 PVC
kubectl get pvc --all-namespaces -o custom-columns=NAME:.metadata.name,STORAGECLASS:.spec.storageClassName
\`\`\`

## 实战案例

### 案例 1: AWS EBS 动态制备

\`\`\`yaml
# 1. 创建 StorageClass
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: aws-gp3
provisioner: ebs.csi.aws.com
parameters:
  type: gp3
  iops: "3000"
  throughput: "125"
  encrypted: "true"
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: true
reclaimPolicy: Delete

---
# 2. 创建 PVC
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: mysql-pvc
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: aws-gp3
  resources:
    requests:
      storage: 20Gi

---
# 3. Pod 使用 PVC
apiVersion: v1
kind: Pod
metadata:
  name: mysql
spec:
  containers:
  - name: mysql
    image: mysql:8.0
    volumeMounts:
    - name: data
      mountPath: /var/lib/mysql
  volumes:
  - name: data
    persistentVolumeClaim:
      claimName: mysql-pvc
\`\`\`

### 案例 2: NFS 动态制备

需要先部署 NFS Provisioner。

\`\`\`yaml
# 1. 部署 NFS Provisioner
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nfs-provisioner
spec:
  replicas: 1
  selector:
    matchLabels:
      app: nfs-provisioner
  template:
    metadata:
      labels:
        app: nfs-provisioner
    spec:
      serviceAccountName: nfs-provisioner
      containers:
      - name: nfs-provisioner
        image: registry.k8s.io/sig-storage/nfs-subdir-external-provisioner:v4.0.2
        volumeMounts:
        - name: nfs-root
          mountPath: /persistentvolumes
        env:
        - name: PROVISIONER_NAME
          value: nfs-provisioner
        - name: NFS_SERVER
          value: 192.168.1.100
        - name: NFS_PATH
          value: /exports
      volumes:
      - name: nfs-root
        nfs:
          server: 192.168.1.100
          path: /exports

---
# 2. 创建 StorageClass
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: nfs-storage
provisioner: nfs-provisioner
parameters:
  archiveOnDelete: "false"
reclaimPolicy: Delete
volumeBindingMode: Immediate
\`\`\`

### 案例 3: 多层级存储

定义不同性能的存储类:

\`\`\`yaml
# 高性能 SSD
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: fast
provisioner: kubernetes.io/aws-ebs
parameters:
  type: io2
  iopsPerGB: "50"
  fsType: ext4
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: true

---
# 标准 SSD
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: standard
  annotations:
    storageclass.kubernetes.io/is-default-class: "true"
provisioner: kubernetes.io/aws-ebs
parameters:
  type: gp3
  iopsPerGB: "10"
  fsType: ext4
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: true

---
# 归档存储
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: archive
provisioner: kubernetes.io/aws-ebs
parameters:
  type: sc1
  fsType: ext4
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: false
\`\`\`

使用:
\`\`\`yaml
# 数据库使用高性能存储
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: db-pvc
spec:
  storageClassName: fast
  accessModes: [ReadWriteOnce]
  resources:
    requests:
      storage: 100Gi

---
# 日志使用归档存储
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: log-pvc
spec:
  storageClassName: archive
  accessModes: [ReadWriteOnce]
  resources:
    requests:
      storage: 500Gi
\`\`\`

### 案例 4: 本地存储 StorageClass

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: local-storage
provisioner: kubernetes.io/no-provisioner
volumeBindingMode: WaitForFirstConsumer

---
# 手动创建 Local PV
apiVersion: v1
kind: PersistentVolume
metadata:
  name: local-pv-1
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
          - node1
\`\`\`

### 案例 5: 卷快照

需要支持快照的 CSI 驱动。

\`\`\`yaml
# 1. 创建 VolumeSnapshotClass
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshotClass
metadata:
  name: csi-snapclass
driver: ebs.csi.aws.com
deletionPolicy: Delete

---
# 2. 创建快照
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshot
metadata:
  name: mysql-snapshot
spec:
  volumeSnapshotClassName: csi-snapclass
  source:
    persistentVolumeClaimName: mysql-pvc

---
# 3. 从快照恢复
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: mysql-pvc-restore
spec:
  storageClassName: aws-gp3
  dataSource:
    name: mysql-snapshot
    kind: VolumeSnapshot
    apiGroup: snapshot.storage.k8s.io
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 20Gi
\`\`\`

## 最佳实践

1. **生产环境建议**
   - 定义多个 StorageClass 满足不同需求
   - 设置一个默认 StorageClass
   - 使用 WaitForFirstConsumer 绑定模式
   - 重要数据使用 Retain 策略

2. **性能优化**
   - 根据工作负载选择合适的存储类型
   - 使用 SSD 提升数据库性能
   - 合理设置 IOPS 和吞吐量
   - 启用卷扩容功能

3. **成本优化**
   - 日志和备份使用低成本存储
   - 设置存储配额限制
   - 定期清理无用 PVC
   - 监控存储使用率

4. **安全加固**
   - 启用存储加密
   - 使用 RBAC 控制访问
   - 定期备份重要数据
   - 审计存储访问日志

5. **命名规范**
   - 使用描述性名称: `fast-ssd`, `standard-hdd`
   - 包含性能指标: `high-iops`, `low-latency`
   - 标注用途: `db-storage`, `log-storage`

## 常见问题

### Q1: 如何设置默认 StorageClass?

A:
\`\`\`bash
# 设置默认
kubectl annotate storageclass <sc-name> \\
  storageclass.kubernetes.io/is-default-class=true

# 取消默认
kubectl annotate storageclass <sc-name> \\
  storageclass.kubernetes.io/is-default-class=false

# 只能有一个默认 StorageClass
\`\`\`

### Q2: PVC 一直 Pending,如何排查?

A:
\`\`\`bash
# 1. 查看 PVC 事件
kubectl describe pvc <pvc-name>

# 2. 检查 StorageClass 是否存在
kubectl get sc

# 3. 查看 Provisioner 日志
kubectl logs -n kube-system <provisioner-pod>

# 4. 检查配额限制
kubectl describe resourcequota -n <namespace>
\`\`\`

常见原因:
- StorageClass 不存在
- Provisioner 未部署
- 参数配置错误
- 存储配额不足
- 权限不足

### Q3: 如何修改 StorageClass?

A: StorageClass 的某些字段不可变:
- provisioner
- parameters

如需修改,需要:
\`\`\`bash
# 1. 创建新的 StorageClass
kubectl apply -f new-sc.yaml

# 2. 更新应用使用新 StorageClass
# 3. 删除旧 PVC 和 StorageClass
kubectl delete sc old-sc
\`\`\`

### Q4: 动态制备和静态制备的区别?

A:
| 特性 | 动态制备 | 静态制备 |
|------|---------|---------|
| **PV 创建** | 自动 | 手动 |
| **适用场景** | 大规模集群 | 小规模或特殊需求 |
| **灵活性** | 高 | 低 |
| **管理成本** | 低 | 高 |

### Q5: 如何限制用户使用的 StorageClass?

A: 使用 ResourceQuota:
\`\`\`yaml
apiVersion: v1
kind: ResourceQuota
metadata:
  name: storage-quota
  namespace: dev
spec:
  hard:
    # 限制 fast-ssd 存储使用
    fast-ssd.storageclass.storage.k8s.io/requests.storage: "100Gi"
    fast-ssd.storageclass.storage.k8s.io/persistentvolumeclaims: "10"
\`\`\`

## 总结

- ✅ 理解 StorageClass 的概念和用途
- ✅ 掌握动态制备的工作原理
- ✅ 理解各种参数的含义和配置
- ✅ 掌握多层级存储的设计
- ✅ 了解最佳实践和成本优化
- ✅ 能够排查 StorageClass 相关问题

## 参考资源

- [Storage Classes 官方文档](https://kubernetes.io/docs/concepts/storage/storage-classes/)
- [Dynamic Volume Provisioning](https://kubernetes.io/docs/concepts/storage/dynamic-provisioning/)
- [Volume Snapshots](https://kubernetes.io/docs/concepts/storage/volume-snapshots/)

---

*更新日期: 2025-12-03*
