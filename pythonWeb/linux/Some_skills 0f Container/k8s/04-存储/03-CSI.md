# CSI

> 容器存储接口、CSI 驱动、卷快照

## 概述

【本文档是 Kubernetes 知识体系的一部分】

CSI (Container Storage Interface) 是 Kubernetes 的容器存储接口标准,定义了容器编排系统与存储系统之间的标准接口。CSI 使存储供应商能够开发一次插件,就可以在多个容器编排系统中使用。

## 核心概念

### 什么是 CSI

CSI 是一个行业标准接口,主要功能包括:

- **标准化接口**: 统一的存储接口规范
- **插件化架构**: 存储系统作为插件运行
- **供应商中立**: 不依赖特定的容器编排系统
- **功能丰富**: 支持卷快照、克隆、扩容等高级功能

### 为什么需要 CSI

在生产环境中,CSI 解决了以下问题:

1. **解耦合**: 存储代码不再内置在 Kubernetes 中
2. **可扩展**: 第三方可以开发自己的存储插件
3. **标准化**: 统一的接口规范
4. **功能增强**: 支持快照、克隆等高级特性
5. **简化开发**: 存储供应商只需实现一次

### CSI vs In-Tree vs FlexVolume

| 特性 | CSI | In-Tree | FlexVolume |
|------|-----|---------|------------|
| **维护** | 第三方 | Kubernetes | 第三方 |
| **部署** | 独立 Pod | 内置 | 二进制文件 |
| **功能** | 完整 | 基础 | 有限 |
| **状态** | 推荐 | 废弃中 | 废弃 |
| **快照支持** | ✅ | ❌ | ❌ |
| **克隆支持** | ✅ | ❌ | ❌ |

### CSI 架构

\`\`\`
                   Kubernetes
                       |
           +-----------+-----------+
           |                       |
    Node Plugin              Controller Plugin
    (每个节点)                  (1-3副本)
           |                       |
    +------+------+         +------+------+
    |             |         |             |
 Attach      Mount    Provision      Snapshot
\`\`\`

**核心组件**:

1. **CSI Controller**: 管理卷的生命周期(创建、删除、快照等)
2. **CSI Node**: 在节点上挂载/卸载卷
3. **External Provisioner**: 监听 PVC,调用 CSI 创建卷
4. **External Attacher**: 处理卷的 Attach/Detach
5. **External Snapshotter**: 处理快照操作
6. **External Resizer**: 处理卷扩容

## CSI 驱动部署

### 常见 CSI 驱动

| 驱动 | 说明 |
|------|------|
| [AWS EBS CSI](https://github.com/kubernetes-sigs/aws-ebs-csi-driver) | AWS 弹性块存储 |
| [GCE PD CSI](https://github.com/kubernetes-sigs/gcp-compute-persistent-disk-csi-driver) | Google Cloud 持久化磁盘 |
| [Azure Disk CSI](https://github.com/kubernetes-sigs/azuredisk-csi-driver) | Azure 磁盘 |
| [Ceph CSI](https://github.com/ceph/ceph-csi) | Ceph RBD/CephFS |
| [NFS CSI](https://github.com/kubernetes-csi/csi-driver-nfs) | NFS |
| [Local Path](https://github.com/rancher/local-path-provisioner) | 本地路径 |

### 部署 AWS EBS CSI Driver

\`\`\`bash
# 1. 添加 Helm 仓库
helm repo add aws-ebs-csi-driver https://kubernetes-sigs.github.io/aws-ebs-csi-driver
helm repo update

# 2. 安装驱动
helm install aws-ebs-csi-driver \\
  aws-ebs-csi-driver/aws-ebs-csi-driver \\
  --namespace kube-system

# 3. 验证安装
kubectl get pods -n kube-system -l app.kubernetes.io/name=aws-ebs-csi-driver

# 4. 查看 CSIDriver 对象
kubectl get csidriver
\`\`\`

### 部署 NFS CSI Driver

\`\`\`bash
# 使用 Helm 安装
helm repo add csi-driver-nfs https://raw.githubusercontent.com/kubernetes-csi/csi-driver-nfs/master/charts
helm install csi-driver-nfs csi-driver-nfs/csi-driver-nfs \\
  --namespace kube-system \\
  --set kubeletDir=/var/lib/kubelet

# 或使用 YAML
kubectl apply -f https://raw.githubusercontent.com/kubernetes-csi/csi-driver-nfs/master/deploy/install-driver.sh
\`\`\`

### CSI Driver 配置示例

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: CSIDriver
metadata:
  name: ebs.csi.aws.com
spec:
  # 是否支持 Attach
  attachRequired: true
  # Pod 信息传递给 CSI Driver
  podInfoOnMount: false
  # 卷生命周期模式
  volumeLifecycleModes:
  - Persistent
  - Ephemeral
  # 支持的文件系统分组
  fsGroupPolicy: File
\`\`\`

## StorageClass 配置

### AWS EBS CSI

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: ebs-sc
provisioner: ebs.csi.aws.com
parameters:
  type: gp3
  iops: "3000"
  throughput: "125"
  encrypted: "true"
  kmsKeyId: arn:aws:kms:us-east-1:123456789:key/xxx
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: true
\`\`\`

### GCE PD CSI

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: gce-pd-sc
provisioner: pd.csi.storage.gke.io
parameters:
  type: pd-ssd
  replication-type: regional-pd
volumeBindingMode: WaitForFirstConsumer
allowVolumeExpansion: true
\`\`\`

### Ceph RBD CSI

\`\`\`yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: ceph-rbd
provisioner: rbd.csi.ceph.com
parameters:
  clusterID: <cluster-id>
  pool: kubernetes
  imageFeatures: layering
  csi.storage.k8s.io/provisioner-secret-name: csi-rbd-secret
  csi.storage.k8s.io/provisioner-secret-namespace: default
  csi.storage.k8s.io/node-stage-secret-name: csi-rbd-secret
  csi.storage.k8s.io/node-stage-secret-namespace: default
reclaimPolicy: Delete
allowVolumeExpansion: true
mountOptions:
  - discard
\`\`\`

## 卷快照

### 创建 VolumeSnapshotClass

\`\`\`yaml
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshotClass
metadata:
  name: csi-snapclass
driver: ebs.csi.aws.com
deletionPolicy: Delete
parameters:
  # 快照标签
  tagSpecification_1: "Name=snapshot-{{ .VolumeSnapshotName }}"
  tagSpecification_2: "CreatedBy=CSI"
\`\`\`

### 创建快照

\`\`\`yaml
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshot
metadata:
  name: mysql-snapshot
  namespace: default
spec:
  volumeSnapshotClassName: csi-snapclass
  source:
    persistentVolumeClaimName: mysql-pvc
\`\`\`

### 从快照恢复

\`\`\`yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: mysql-pvc-restore
spec:
  storageClassName: ebs-sc
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

### 快照定时备份

使用 CronJob 定时创建快照:

\`\`\`yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: volume-snapshot
spec:
  schedule: "0 2 * * *"  # 每天凌晨2点
  jobTemplate:
    spec:
      template:
        spec:
          serviceAccountName: snapshot-creator
          containers:
          - name: kubectl
            image: bitnami/kubectl
            command:
            - /bin/sh
            - -c
            - |
              kubectl create -f - <<EOF
              apiVersion: snapshot.storage.k8s.io/v1
              kind: VolumeSnapshot
              metadata:
                name: mysql-snapshot-$(date +%Y%m%d-%H%M%S)
              spec:
                volumeSnapshotClassName: csi-snapclass
                source:
                  persistentVolumeClaimName: mysql-pvc
              EOF
          restartPolicy: OnFailure
\`\`\`

## 卷克隆

从现有 PVC 克隆新 PVC:

\`\`\`yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: mysql-pvc-clone
spec:
  storageClassName: ebs-sc
  dataSource:
    name: mysql-pvc      # 源 PVC
    kind: PersistentVolumeClaim
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 20Gi
\`\`\`

## kubectl 命令

\`\`\`bash
# 查看 CSI Driver
kubectl get csidriver
kubectl describe csidriver <driver-name>

# 查看 CSI Pods
kubectl get pods -n kube-system | grep csi

# 查看 VolumeSnapshotClass
kubectl get volumesnapshotclass
kubectl get vsc

# 查看 VolumeSnapshot
kubectl get volumesnapshot
kubectl get vs

# 创建快照
kubectl create -f snapshot.yaml

# 查看快照详情
kubectl describe volumesnapshot <snapshot-name>

# 删除快照
kubectl delete volumesnapshot <snapshot-name>

# 查看 CSI 节点信息
kubectl get csinodes

# 查看 VolumeAttachment
kubectl get volumeattachment
\`\`\`

## 实战案例

### 案例 1: 数据库备份和恢复

\`\`\`yaml
# 1. 创建数据库
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: postgres-pvc
spec:
  storageClassName: ebs-sc
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 50Gi

---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: postgres
spec:
  serviceName: postgres
  replicas: 1
  selector:
    matchLabels:
      app: postgres
  template:
    metadata:
      labels:
        app: postgres
    spec:
      containers:
      - name: postgres
        image: postgres:14
        ports:
        - containerPort: 5432
        volumeMounts:
        - name: data
          mountPath: /var/lib/postgresql/data
        env:
        - name: POSTGRES_PASSWORD
          valueFrom:
            secretKeyRef:
              name: postgres-secret
              key: password
      volumes:
      - name: data
        persistentVolumeClaim:
          claimName: postgres-pvc

---
# 2. 创建快照
apiVersion: snapshot.storage.k8s.io/v1
kind: VolumeSnapshot
metadata:
  name: postgres-backup
spec:
  volumeSnapshotClassName: csi-snapclass
  source:
    persistentVolumeClaimName: postgres-pvc

---
# 3. 从快照恢复
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: postgres-pvc-restore
spec:
  storageClassName: ebs-sc
  dataSource:
    name: postgres-backup
    kind: VolumeSnapshot
    apiGroup: snapshot.storage.k8s.io
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 50Gi
\`\`\`

### 案例 2: 开发环境快速复制

\`\`\`yaml
# 从生产环境 PVC 克隆到开发环境
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: dev-database
  namespace: dev
spec:
  storageClassName: ebs-sc
  dataSource:
    name: prod-database
    kind: PersistentVolumeClaim
    namespace: prod
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 20Gi
\`\`\`

### 案例 3: 临时卷

CSI 支持临时卷(Ephemeral Volume):

\`\`\`yaml
apiVersion: v1
kind: Pod
metadata:
  name: test-pod
spec:
  containers:
  - name: app
    image: busybox
    command: ['sleep', '3600']
    volumeMounts:
    - name: scratch
      mountPath: /scratch
  volumes:
  - name: scratch
    csi:
      driver: ebs.csi.aws.com
      volumeAttributes:
        type: gp3
\`\`\`

## 最佳实践

1. **生产环境建议**
   - 使用 CSI 驱动而非 In-Tree
   - 部署多副本 Controller
   - 配置资源限制
   - 监控 CSI 组件健康

2. **快照管理**
   - 定期创建快照
   - 设置快照保留策略
   - 测试快照恢复流程
   - 异地备份快照

3. **性能优化**
   - 使用 WaitForFirstConsumer
   - 选择合适的存储类型
   - 启用卷缓存
   - 监控 I/O 性能

4. **安全加固**
   - 启用存储加密
   - 使用 Secret 管理凭证
   - RBAC 权限控制
   - 审计日志记录

5. **成本优化**
   - 定期清理旧快照
   - 使用生命周期策略
   - 监控存储使用
   - 选择合适的存储层级

## 常见问题

### Q1: CSI 驱动安装失败?

A: 排查步骤:
\`\`\`bash
# 1. 查看 CSI Pods
kubectl get pods -n kube-system | grep csi

# 2. 查看日志
kubectl logs -n kube-system <csi-controller-pod>
kubectl logs -n kube-system <csi-node-pod>

# 3. 检查 RBAC 权限
kubectl get clusterrole | grep csi
kubectl get clusterrolebinding | grep csi

# 4. 检查云服务权限(如 AWS IAM)
\`\`\`

### Q2: 快照创建失败?

A: 检查:
\`\`\`bash
# 查看 VolumeSnapshot
kubectl describe volumesnapshot <snapshot-name>

# 查看 Snapshotter 日志
kubectl logs -n kube-system <external-snapshotter-pod>

# 检查 VolumeSnapshotClass
kubectl get volumesnapshotclass
\`\`\`

常见原因:
- VolumeSnapshotClass 不存在
- PVC 正在使用中
- 存储不支持快照
- 权限不足

### Q3: 卷扩容不生效?

A:
\`\`\`bash
# 1. 检查 StorageClass 是否支持扩容
kubectl get sc <sc-name> -o yaml | grep allowVolumeExpansion

# 2. 某些存储需要重启 Pod
kubectl delete pod <pod-name>

# 3. 查看 PVC 状态
kubectl describe pvc <pvc-name>
\`\`\`

### Q4: In-Tree 如何迁移到 CSI?

A: 使用 CSIMigration 特性:
\`\`\`yaml
# kube-controller-manager 启动参数
--feature-gates=CSIMigration=true,CSIMigrationAWS=true
\`\`\`

步骤:
1. 部署 CSI 驱动
2. 启用 CSIMigration
3. 验证功能正常
4. 逐步迁移 PV

### Q5: 如何监控 CSI 驱动?

A: 使用 Prometheus:
\`\`\`yaml
# ServiceMonitor
apiVersion: monitoring.coreos.com/v1
kind: ServiceMonitor
metadata:
  name: csi-driver
spec:
  selector:
    matchLabels:
      app: csi-driver
  endpoints:
  - port: metrics
\`\`\`

监控指标:
- 卷操作延迟
- 失败率
- 资源使用
- 快照数量

## 总结

- ✅ 理解 CSI 的架构和原理
- ✅ 掌握 CSI 驱动的部署
- ✅ 理解卷快照和克隆
- ✅ 掌握 CSI StorageClass 配置
- ✅ 了解最佳实践和故障排查
- ✅ 能够实现数据备份和恢复

## 参考资源

- [CSI 官方文档](https://kubernetes-csi.github.io/docs/)
- [CSI Spec](https://github.com/container-storage-interface/spec)
- [Volume Snapshots](https://kubernetes.io/docs/concepts/storage/volume-snapshots/)
- [CSI Drivers List](https://kubernetes-csi.github.io/docs/drivers.html)

---

*更新日期: 2025-12-03*
