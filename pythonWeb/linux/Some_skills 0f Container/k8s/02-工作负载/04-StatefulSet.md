# StatefulSet

> 有状态应用、稳定网络标识、有序部署

## 概述

【本文档是 Kubernetes 知识体系的一部分】

StatefulSet 是用于管理有状态应用的工作负载资源。与 Deployment 不同,它为每个 Pod 提供稳定的网络标识、持久存储和有序的部署/扩缩容。

## 核心概念

### 什么是 StatefulSet

StatefulSet 用于部署需要以下特性的应用:
- **稳定的网络标识**: 每个 Pod 有固定的主机名
- **稳定的持久存储**: Pod 重启后仍使用相同的 PV
- **有序部署**: Pod 按序号顺序创建和删除
- **有序扩缩容**: 严格按序号进行
- **有序滚动更新**: 按序号逐个更新

**核心特性:**
- **Pod 标识**: pod-name-0, pod-name-1, pod-name-2...
- **固定主机名**: 通过 Headless Service 访问
- **PVC 绑定**: 每个 Pod 独立的持久卷
- **有序操作**: 创建、删除、扩缩容都有序进行

### 为什么需要

在生产环境中,有状态应用需要 StatefulSet:

1. **数据库**: MySQL、PostgreSQL、MongoDB 集群
2. **消息队列**: Kafka、RabbitMQ 集群
3. **分布式存储**: Elasticsearch、Cassandra
4. **协调服务**: ZooKeeper、etcd 集群

### StatefulSet vs Deployment

| 特性 | StatefulSet | Deployment |
|------|------------|-----------|
| 应用类型 | 有状态应用 | 无状态应用 |
| Pod 名称 | 固定(pod-0, pod-1) | 随机 hash |
| 网络标识 | 稳定不变 | 动态变化 |
| 存储 | 独立持久卷 | 共享或临时 |
| 部署顺序 | 有序(0→1→2) | 并行 |
| 扩缩容 | 有序 | 并行 |
| 更新策略 | 有序更新 | 滚动更新 |

## 基本使用

### 完整示例

```yaml
# Headless Service (必需)
apiVersion: v1
kind: Service
metadata:
  name: nginx-headless
  labels:
    app: nginx
spec:
  clusterIP: None  # Headless Service
  selector:
    app: nginx
  ports:
  - port: 80
    name: web
---
# StatefulSet
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: web
spec:
  # 关联的 Headless Service
  serviceName: "nginx-headless"

  # 副本数
  replicas: 3

  # 选择器
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
          name: web
        # 卷挂载
        volumeMounts:
        - name: www
          mountPath: /usr/share/nginx/html

  # 卷声明模板
  volumeClaimTemplates:
  - metadata:
      name: www
    spec:
      accessModes: [ "ReadWriteOnce" ]
      resources:
        requests:
          storage: 1Gi
```

**Pod 命名规则:**
```
web-0  # 第一个 Pod
web-1  # 第二个 Pod
web-2  # 第三个 Pod
```

**DNS 域名:**
```
<pod-name>.<service-name>.<namespace>.svc.cluster.local

web-0.nginx-headless.default.svc.cluster.local
web-1.nginx-headless.default.svc.cluster.local
web-2.nginx-headless.default.svc.cluster.local
```

### kubectl 命令

```bash
# 创建 StatefulSet
kubectl apply -f statefulset.yaml

# 查看 StatefulSet
kubectl get statefulset
kubectl get sts  # 简写

# 查看 Pod (注意有序命名)
kubectl get pods -l app=nginx

# 查看详情
kubectl describe statefulset web

# 查看 PVC
kubectl get pvc

# 扩容
kubectl scale statefulset web --replicas=5

# 删除 StatefulSet (保留 PVC)
kubectl delete statefulset web

# 删除 StatefulSet 和 PVC
kubectl delete statefulset web
kubectl delete pvc -l app=nginx
```

## 有序部署和扩缩容

### 创建过程

```yaml
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: web
spec:
  serviceName: "nginx"
  replicas: 3
  podManagementPolicy: OrderedReady  # 有序策略 (默认)

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
```

**创建顺序:**
```
1. 创建 web-0 → 等待 Running + Ready
2. 创建 web-1 → 等待 Running + Ready
3. 创建 web-2 → 等待 Running + Ready

完成: web-0, web-1, web-2 全部就绪
```

### 扩容过程

```bash
# 从 3 个副本扩容到 5 个
kubectl scale statefulset web --replicas=5
```

**扩容顺序:**
```
当前: web-0, web-1, web-2
步骤1: 创建 web-3 → 等待就绪
步骤2: 创建 web-4 → 等待就绪
完成: web-0, web-1, web-2, web-3, web-4
```

### 缩容过程

```bash
# 从 5 个副本缩容到 3 个
kubectl scale statefulset web --replicas=3
```

**缩容顺序:**
```
当前: web-0, web-1, web-2, web-3, web-4
步骤1: 删除 web-4 → 等待完全终止
步骤2: 删除 web-3 → 等待完全终止
完成: web-0, web-1, web-2

⚠️ 注意: PVC 不会被自动删除
```

### 并行策略

```yaml
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: web
spec:
  podManagementPolicy: Parallel  # 并行策略
  replicas: 3

  serviceName: "nginx"
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
```

**并行创建:**
```
同时创建: web-0, web-1, web-2
不等待前一个 Pod 就绪
更快但失去了顺序保证
```

## 更新策略

### RollingUpdate (滚动更新)

```yaml
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: web
spec:
  replicas: 3
  serviceName: "nginx"

  # 更新策略
  updateStrategy:
    type: RollingUpdate
    rollingUpdate:
      partition: 0  # 从序号 >= partition 的 Pod 开始更新

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
        image: nginx:1.22  # 新版本
```

**更新顺序(逆序):**
```
当前: web-0 (v1.21), web-1 (v1.21), web-2 (v1.21)

步骤1: 删除并重建 web-2 → 等待就绪
步骤2: 删除并重建 web-1 → 等待就绪
步骤3: 删除并重建 web-0 → 等待就绪

完成: web-0 (v1.22), web-1 (v1.22), web-2 (v1.22)
```

### 分区更新 (Canary)

```yaml
updateStrategy:
  type: RollingUpdate
  rollingUpdate:
    partition: 2  # 只更新序号 >= 2 的 Pod
```

**分区更新流程:**
```
partition: 2
当前: web-0 (v1.21), web-1 (v1.21), web-2 (v1.21)

步骤1: 更新 web-2 → v1.22
观察: web-0 (v1.21), web-1 (v1.21), web-2 (v1.22)

验证 web-2 没问题后:
partition: 1

步骤2: 更新 web-1 → v1.22
观察: web-0 (v1.21), web-1 (v1.22), web-2 (v1.22)

最后:
partition: 0

步骤3: 更新 web-0 → v1.22
完成: web-0 (v1.22), web-1 (v1.22), web-2 (v1.22)
```

### OnDelete (手动更新)

```yaml
updateStrategy:
  type: OnDelete  # 手动删除 Pod 才更新
```

```bash
# 更新 StatefulSet 配置
kubectl apply -f statefulset.yaml

# 手动删除 Pod 触发更新
kubectl delete pod web-2  # 重建时使用新配置
kubectl delete pod web-1
kubectl delete pod web-0
```

## 实战案例

### 案例 1: MySQL 主从复制

```yaml
# Headless Service
apiVersion: v1
kind: Service
metadata:
  name: mysql
  labels:
    app: mysql
spec:
  clusterIP: None
  selector:
    app: mysql
  ports:
  - port: 3306
    name: mysql
---
# StatefulSet
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: mysql
spec:
  serviceName: "mysql"
  replicas: 3
  selector:
    matchLabels:
      app: mysql

  template:
    metadata:
      labels:
        app: mysql
    spec:
      initContainers:
      # Init 容器: 设置 server-id
      - name: init-mysql
        image: mysql:5.7
        command:
        - bash
        - "-c"
        - |
          set -ex
          # 从 Pod 序号生成 server-id
          [[ $HOSTNAME =~ -([0-9]+)$ ]] || exit 1
          ordinal=${BASH_REMATCH[1]}
          echo [mysqld] > /mnt/conf.d/server-id.cnf
          # server-id 不能为 0
          echo server-id=$((100 + $ordinal)) >> /mnt/conf.d/server-id.cnf

          # mysql-0 为主节点
          if [[ $ordinal -eq 0 ]]; then
            cp /mnt/config-map/master.cnf /mnt/conf.d/
          else
            cp /mnt/config-map/slave.cnf /mnt/conf.d/
          fi
        volumeMounts:
        - name: conf
          mountPath: /mnt/conf.d
        - name: config-map
          mountPath: /mnt/config-map

      containers:
      - name: mysql
        image: mysql:5.7
        env:
        - name: MYSQL_ROOT_PASSWORD
          value: "password"
        ports:
        - containerPort: 3306
          name: mysql
        volumeMounts:
        - name: data
          mountPath: /var/lib/mysql
        - name: conf
          mountPath: /etc/mysql/conf.d
        resources:
          requests:
            cpu: 500m
            memory: 1Gi

        livenessProbe:
          exec:
            command: ["mysqladmin", "ping"]
          initialDelaySeconds: 30
          periodSeconds: 10
          timeoutSeconds: 5

        readinessProbe:
          exec:
            command: ["mysql", "-h", "127.0.0.1", "-e", "SELECT 1"]
          initialDelaySeconds: 5
          periodSeconds: 2
          timeoutSeconds: 1

      volumes:
      - name: conf
        emptyDir: {}
      - name: config-map
        configMap:
          name: mysql

  volumeClaimTemplates:
  - metadata:
      name: data
    spec:
      accessModes: ["ReadWriteOnce"]
      resources:
        requests:
          storage: 10Gi
---
# ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: mysql
data:
  master.cnf: |
    [mysqld]
    log-bin=mysql-bin
    binlog-format=ROW
  slave.cnf: |
    [mysqld]
    super-read-only
    relay-log=mysql-relay-bin
```

访问方式:
```bash
# 访问主节点 (写)
mysql -h mysql-0.mysql -uroot -p

# 访问从节点 (读)
mysql -h mysql-1.mysql -uroot -p
mysql -h mysql-2.mysql -uroot -p

# 在应用中
# 写: mysql-0.mysql.default.svc.cluster.local
# 读: mysql-1.mysql.default.svc.cluster.local
```

### 案例 2: ZooKeeper 集群

```yaml
apiVersion: v1
kind: Service
metadata:
  name: zk-headless
  labels:
    app: zk
spec:
  clusterIP: None
  selector:
    app: zk
  ports:
  - port: 2888
    name: server
  - port: 3888
    name: leader-election
---
apiVersion: v1
kind: Service
metadata:
  name: zk-service
  labels:
    app: zk
spec:
  selector:
    app: zk
  ports:
  - port: 2181
    name: client
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: zk
spec:
  serviceName: "zk-headless"
  replicas: 3
  selector:
    matchLabels:
      app: zk

  podManagementPolicy: OrderedReady
  updateStrategy:
    type: RollingUpdate

  template:
    metadata:
      labels:
        app: zk
    spec:
      containers:
      - name: zookeeper
        image: zookeeper:3.7
        ports:
        - containerPort: 2181
          name: client
        - containerPort: 2888
          name: server
        - containerPort: 3888
          name: leader-election

        command:
        - sh
        - -c
        - |
          # 生成 myid
          [[ $HOSTNAME =~ -([0-9]+)$ ]] || exit 1
          ordinal=${BASH_REMATCH[1]}
          echo $((ordinal + 1)) > /data/myid

          # 生成 zoo.cfg
          cat > /conf/zoo.cfg <<EOF
          tickTime=2000
          dataDir=/data
          clientPort=2181
          initLimit=10
          syncLimit=5
          server.1=zk-0.zk-headless:2888:3888
          server.2=zk-1.zk-headless:2888:3888
          server.3=zk-2.zk-headless:2888:3888
          EOF

          # 启动 ZooKeeper
          zkServer.sh start-foreground

        volumeMounts:
        - name: data
          mountPath: /data

        readinessProbe:
          exec:
            command:
            - sh
            - -c
            - 'echo ruok | nc 127.0.0.1 2181 | grep imok'
          initialDelaySeconds: 10
          periodSeconds: 5

        livenessProbe:
          exec:
            command:
            - sh
            - -c
            - 'echo ruok | nc 127.0.0.1 2181 | grep imok'
          initialDelaySeconds: 10
          periodSeconds: 10

  volumeClaimTemplates:
  - metadata:
      name: data
    spec:
      accessModes: ["ReadWriteOnce"]
      resources:
        requests:
          storage: 5Gi
```

### 案例 3: MongoDB 副本集

```yaml
apiVersion: v1
kind: Service
metadata:
  name: mongodb
  labels:
    app: mongodb
spec:
  clusterIP: None
  selector:
    app: mongodb
  ports:
  - port: 27017
    name: mongodb
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: mongodb
spec:
  serviceName: "mongodb"
  replicas: 3
  selector:
    matchLabels:
      app: mongodb

  template:
    metadata:
      labels:
        app: mongodb
    spec:
      containers:
      - name: mongodb
        image: mongo:5.0
        command:
        - mongod
        - "--replSet"
        - "rs0"
        - "--bind_ip_all"

        ports:
        - containerPort: 27017
          name: mongodb

        volumeMounts:
        - name: data
          mountPath: /data/db

        env:
        - name: MONGO_INITDB_ROOT_USERNAME
          value: "admin"
        - name: MONGO_INITDB_ROOT_PASSWORD
          valueFrom:
            secretKeyRef:
              name: mongodb-secret
              key: password

        resources:
          requests:
            cpu: 500m
            memory: 1Gi
          limits:
            cpu: 1000m
            memory: 2Gi

        livenessProbe:
          exec:
            command:
            - mongo
            - --eval
            - "db.adminCommand('ping')"
          initialDelaySeconds: 30
          periodSeconds: 10

        readinessProbe:
          exec:
            command:
            - mongo
            - --eval
            - "db.adminCommand('ping')"
          initialDelaySeconds: 5
          periodSeconds: 5

  volumeClaimTemplates:
  - metadata:
      name: data
    spec:
      accessModes: ["ReadWriteOnce"]
      resources:
        requests:
          storage: 10Gi
---
# 初始化副本集 Job
apiVersion: batch/v1
kind: Job
metadata:
  name: mongodb-init
spec:
  template:
    spec:
      restartPolicy: OnFailure
      containers:
      - name: init
        image: mongo:5.0
        command:
        - sh
        - -c
        - |
          sleep 30
          mongo mongodb-0.mongodb:27017 <<EOF
          rs.initiate({
            _id: "rs0",
            members: [
              { _id: 0, host: "mongodb-0.mongodb:27017" },
              { _id: 1, host: "mongodb-1.mongodb:27017" },
              { _id: 2, host: "mongodb-2.mongodb:27017" }
            ]
          })
          EOF
```

## 最佳实践

### 1. Headless Service 必需

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-service
spec:
  clusterIP: None  # 必须设置为 None
  selector:
    app: myapp
  ports:
  - port: 8080
```

### 2. 稳定的存储

```yaml
volumeClaimTemplates:
- metadata:
    name: data
  spec:
    accessModes: ["ReadWriteOnce"]
    storageClassName: "fast-ssd"  # 使用高性能存储
    resources:
      requests:
        storage: 10Gi
```

### 3. 就绪探针

```yaml
readinessProbe:
  httpGet:
    path: /health
    port: 8080
  initialDelaySeconds: 10
  periodSeconds: 5
  failureThreshold: 3
```

### 4. 有序更新

```yaml
updateStrategy:
  type: RollingUpdate
  rollingUpdate:
    partition: 0  # 使用分区更新进行金丝雀测试
```

### 5. Pod 反亲和性

```yaml
spec:
  template:
    spec:
      affinity:
        podAntiAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
          - labelSelector:
              matchLabels:
                app: myapp
            topologyKey: kubernetes.io/hostname
```

## 常见问题

### Q1: StatefulSet 卡在创建中?

**排查:**
```bash
kubectl get pods
kubectl describe pod web-0
kubectl get pvc
kubectl describe pvc data-web-0
```

**常见原因:**
- PVC 无法绑定
- 镜像拉取失败
- 就绪探针失败

### Q2: 如何删除 StatefulSet?

```bash
# 删除 StatefulSet (保留 PVC)
kubectl delete statefulset web

# 删除 PVC
kubectl get pvc -l app=web
kubectl delete pvc data-web-0 data-web-1 data-web-2

# 级联删除
kubectl delete statefulset web --cascade=foreground
```

### Q3: 如何访问特定 Pod?

```bash
# 通过 DNS
curl http://web-0.nginx-headless:80

# 在集群内
curl http://web-1.nginx-headless.default.svc.cluster.local:80

# 端口转发
kubectl port-forward web-0 8080:80
```

### Q4: StatefulSet vs Deployment 如何选择?

**使用 StatefulSet:**
- 需要稳定的网络标识
- 需要持久存储
- 需要有序部署/扩缩容
- 数据库、消息队列等有状态应用

**使用 Deployment:**
- 无状态应用
- 可随时重建
- Web 服务、API 服务

## 总结

- ✅ 理解 StatefulSet 的特性和使用场景
- ✅ 掌握有序部署和扩缩容机制
- ✅ 了解网络标识和持久存储绑定
- ✅ 熟悉更新策略和分区更新
- ✅ 能够部署数据库等有状态应用
- ✅ 掌握故障排查方法

## 参考资源

- [StatefulSet 官方文档](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/statefulset/)
- [有状态应用](https://kubernetes.io/zh-cn/docs/tutorials/stateful-application/)
- [运行 ZooKeeper](https://kubernetes.io/zh-cn/docs/tutorials/stateful-application/zookeeper/)
- [运行 Cassandra](https://kubernetes.io/zh-cn/docs/tutorials/stateful-application/cassandra/)

---

*更新日期: 2025-12-03*
