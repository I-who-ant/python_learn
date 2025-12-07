# DNS

> CoreDNS、服务发现、DNS 记录

## 概述

【本文档是 Kubernetes 知识体系的一部分】

DNS 是 Kubernetes 集群中实现服务发现的核心组件。从 Kubernetes 1.11 开始,CoreDNS 成为默认的 DNS 服务器,替代了之前的 kube-dns。通过 DNS,Pod 可以使用服务名称而不是 IP 地址来访问其他服务。

## 核心概念

### 什么是 Kubernetes DNS

Kubernetes DNS 是集群内部的域名解析服务,主要功能包括:

- **服务发现**: 为 Service 和 Pod 提供 DNS 记录
- **自动注册**: Service 创建时自动注册 DNS 记录
- **命名空间隔离**: 支持跨命名空间的服务访问
- **负载均衡**: DNS 查询返回 Service 的 ClusterIP

### 为什么需要 DNS

在生产环境中,我们需要 DNS 来解决以下问题:

1. **动态 IP 问题**: Pod IP 会变化,无法硬编码
2. **服务发现**: 应用需要发现和连接其他服务
3. **配置简化**: 使用服务名而非 IP,配置更简洁
4. **跨命名空间通信**: 支持不同命名空间的服务互访
5. **开发测试**: 本地环境和集群环境使用相同配置

### CoreDNS vs kube-dns

| 特性 | CoreDNS | kube-dns |
|------|---------|----------|
| **架构** | 单进程 | 多容器(kubedns、dnsmasq、sidecar) |
| **性能** | 更高效 | 较低 |
| **内存占用** | 更少 | 较多 |
| **可扩展性** | 插件机制 | 有限 |
| **配置** | Corefile | ConfigMap |
| **状态** | 当前默认 | 已废弃 |

## DNS 记录类型

### 1. Service DNS 记录

#### A 记录 (普通 Service)

```
<service-name>.<namespace>.svc.cluster.local
```

示例:
- 同命名空间: `nginx-service`
- 跨命名空间: `nginx-service.default.svc.cluster.local`
- 完全限定域名(FQDN): `nginx-service.default.svc.cluster.local.`

#### SRV 记录 (端口和协议)

```
_<port-name>._<protocol>.<service-name>.<namespace>.svc.cluster.local
```

示例:
```bash
# 查询 SRV 记录
nslookup -type=SRV _http._tcp.nginx-service.default.svc.cluster.local
```

#### Headless Service

对于 Headless Service (`clusterIP: None`),DNS 返回所有 Pod 的 IP:

```bash
# 查询 Headless Service
nslookup mysql-headless

# 返回多个 Pod IP
Server:    10.96.0.10
Address:   10.96.0.10:53

Name:      mysql-headless.default.svc.cluster.local
Address:   10.244.1.5
Name:      mysql-headless.default.svc.cluster.local
Address:   10.244.2.6
```

### 2. Pod DNS 记录

#### 普通 Pod

```
<pod-ip-address>.<namespace>.pod.cluster.local
```

示例: `10-244-1-5.default.pod.cluster.local` (IP 中的 `.` 替换为 `-`)

#### 指定 hostname 和 subdomain 的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: myapp
spec:
  hostname: myapp-pod
  subdomain: myapp-service
  containers:
  - name: app
    image: nginx
```

DNS 记录: `myapp-pod.myapp-service.default.svc.cluster.local`

## CoreDNS 配置

### 查看 CoreDNS 配置

```bash
# 查看 CoreDNS ConfigMap
kubectl get configmap coredns -n kube-system -o yaml

# 查看 CoreDNS Pod
kubectl get pods -n kube-system -l k8s-app=kube-dns

# 查看 CoreDNS 日志
kubectl logs -n kube-system -l k8s-app=kube-dns
```

### Corefile 配置示例

```
# 默认 Corefile
.:53 {
    errors                     # 错误日志
    health {                   # 健康检查
       lameduck 5s
    }
    ready                      # 就绪检查
    kubernetes cluster.local in-addr.arpa ip6.arpa {  # Kubernetes 插件
       pods insecure           # Pod DNS 记录模式
       fallthrough in-addr.arpa ip6.arpa
       ttl 30
    }
    prometheus :9153           # Prometheus 指标
    forward . /etc/resolv.conf {  # 转发外部查询
       max_concurrent 1000
    }
    cache 30                   # DNS 缓存(秒)
    loop                       # 检测转发循环
    reload                     # 自动重载配置
    loadbalance               # 负载均衡
}
```

### 修改 CoreDNS 配置

```bash
# 编辑 ConfigMap
kubectl edit configmap coredns -n kube-system

# 重启 CoreDNS 使配置生效
kubectl rollout restart deployment coredns -n kube-system
```

## 基本使用

### DNS 查询命令

```bash
# 在 Pod 中查询 DNS
kubectl run test-dns --image=busybox --rm -it -- sh

# 在 Pod 内执行:

# nslookup 查询
nslookup kubernetes.default.svc.cluster.local

# dig 查询(需要安装 bind-tools)
kubectl run test-dig --image=nicolaka/netshoot --rm -it -- sh
dig kubernetes.default.svc.cluster.local

# host 命令
host kubernetes.default.svc.cluster.local

# 查看 Pod 的 DNS 配置
cat /etc/resolv.conf
```

### Pod 的 DNS 策略

通过 `dnsPolicy` 字段配置:

#### 1. Default

使用节点的 DNS 配置:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-dns
spec:
  dnsPolicy: Default
  containers:
  - name: app
    image: nginx
```

#### 2. ClusterFirst (默认)

优先使用集群 DNS:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: cluster-first
spec:
  dnsPolicy: ClusterFirst  # 默认值
  containers:
  - name: app
    image: nginx
```

#### 3. ClusterFirstWithHostNet

用于 hostNetwork 的 Pod:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: host-network
spec:
  hostNetwork: true
  dnsPolicy: ClusterFirstWithHostNet
  containers:
  - name: app
    image: nginx
```

#### 4. None (自定义)

完全自定义 DNS 配置:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: custom-dns
spec:
  dnsPolicy: None
  dnsConfig:
    nameservers:
      - 8.8.8.8
      - 8.8.4.4
    searches:
      - default.svc.cluster.local
      - svc.cluster.local
      - cluster.local
    options:
      - name: ndots
        value: "5"
  containers:
  - name: app
    image: nginx
```

## kubectl 命令

```bash
# 查看 CoreDNS Deployment
kubectl get deployment coredns -n kube-system

# 查看 CoreDNS Pod
kubectl get pods -n kube-system -l k8s-app=kube-dns

# 查看 CoreDNS Service
kubectl get svc kube-dns -n kube-system

# 查看 CoreDNS 配置
kubectl get configmap coredns -n kube-system -o yaml

# 查看 CoreDNS 日志
kubectl logs -n kube-system -l k8s-app=kube-dns --tail=100 -f

# 编辑 CoreDNS 配置
kubectl edit configmap coredns -n kube-system

# 重启 CoreDNS
kubectl rollout restart deployment coredns -n kube-system

# 扩容 CoreDNS
kubectl scale deployment coredns --replicas=3 -n kube-system

# 查看 DNS Service Endpoints
kubectl get endpoints kube-dns -n kube-system

# 测试 DNS 解析
kubectl run test-dns --image=busybox --rm -it -- nslookup kubernetes.default
```

## 实战案例

### 案例 1: 配置自定义 DNS

为特定域名配置上游 DNS:

```yaml
# 编辑 coredns ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: coredns
  namespace: kube-system
data:
  Corefile: |
    .:53 {
        errors
        health
        ready
        kubernetes cluster.local in-addr.arpa ip6.arpa {
           pods insecure
           fallthrough in-addr.arpa ip6.arpa
           ttl 30
        }
        prometheus :9153
        forward . /etc/resolv.conf
        cache 30
        loop
        reload
        loadbalance
    }

    # 自定义域名解析
    example.com:53 {
        errors
        cache 30
        forward . 192.168.1.1  # 使用内部 DNS 服务器
    }

    # 特定服务的 DNS 重写
    company.internal:53 {
        rewrite name company-service.default.svc.cluster.local company.internal
        forward . 10.0.0.1
    }
```

### 案例 2: 配置 DNS 缓存和性能优化

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: coredns
  namespace: kube-system
data:
  Corefile: |
    .:53 {
        errors
        health {
            lameduck 5s
        }
        ready
        kubernetes cluster.local in-addr.arpa ip6.arpa {
            pods insecure
            fallthrough in-addr.arpa ip6.arpa
            ttl 30
        }
        prometheus :9153
        forward . /etc/resolv.conf {
            max_concurrent 1000      # 增加并发数
            except example.com       # 排除特定域名
        }
        cache 300 {                  # 增加缓存时间到 5 分钟
            success 9984 30          # 成功记录缓存
            denial 9984 5            # 否定记录缓存
        }
        loop
        reload 10s                   # 配置重载间隔
        loadbalance round_robin      # 负载均衡算法
    }
```

### 案例 3: StatefulSet 的 DNS 配置

StatefulSet Pod 有稳定的 DNS 名称:

```yaml
apiVersion: v1
kind: Service
metadata:
  name: mysql
spec:
  clusterIP: None  # Headless Service
  selector:
    app: mysql
  ports:
  - port: 3306

---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: mysql
spec:
  serviceName: mysql  # 关联 Headless Service
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
```

DNS 记录:
- `mysql-0.mysql.default.svc.cluster.local`
- `mysql-1.mysql.default.svc.cluster.local`
- `mysql-2.mysql.default.svc.cluster.local`

### 案例 4: 配置外部 DNS

为集群外部提供 DNS 记录:

```yaml
# ExternalName Service
apiVersion: v1
kind: Service
metadata:
  name: external-db
spec:
  type: ExternalName
  externalName: database.example.com  # 外部域名
```

访问:
```bash
# Pod 内访问
mysql -h external-db.default.svc.cluster.local -u user -p
# 实际连接到 database.example.com
```

### 案例 5: DNS 故障排查

```bash
# 1. 检查 CoreDNS 状态
kubectl get pods -n kube-system -l k8s-app=kube-dns
kubectl logs -n kube-system -l k8s-app=kube-dns --tail=50

# 2. 创建测试 Pod
kubectl run test-dns --image=nicolaka/netshoot --rm -it -- bash

# 在 Pod 内执行:

# 查看 DNS 配置
cat /etc/resolv.conf

# 测试内部服务解析
nslookup kubernetes.default.svc.cluster.local

# 测试外部域名解析
nslookup google.com

# 详细 DNS 查询
dig +trace kubernetes.default.svc.cluster.local

# 查询 SRV 记录
dig SRV _https._tcp.kubernetes.default.svc.cluster.local

# 3. 检查 DNS 延迟
time nslookup kubernetes.default.svc.cluster.local

# 4. 查看 CoreDNS 指标
kubectl port-forward -n kube-system svc/kube-dns 9153:9153
curl http://localhost:9153/metrics
```

## 高级配置

### 1. 启用 DNS 调试日志

```yaml
# 编辑 Corefile,添加 log 插件
.:53 {
    log  # 启用请求日志
    errors
    # ... 其他配置
}
```

### 2. 配置 DNS 重写

```yaml
.:53 {
    rewrite stop {
        name regex (.*)\.old\.domain\.com {1}.new.domain.com
        answer name (.*)\.new\.domain\.com {1}.old.domain.com
    }
    # ... 其他配置
}
```

### 3. 配置条件转发

```yaml
# 不同域名使用不同的 DNS 服务器
.:53 {
    kubernetes cluster.local in-addr.arpa ip6.arpa {
        pods insecure
        fallthrough in-addr.arpa ip6.arpa
    }
    forward example.com 192.168.1.10   # example.com 使用特定 DNS
    forward . /etc/resolv.conf         # 其他域名使用默认 DNS
    cache 30
}
```

### 4. 配置 DNSSEC

```yaml
.:53 {
    dnssec {
        key file /etc/coredns/Kcluster.local.+013+12345.key
    }
    # ... 其他配置
}
```

## 性能优化

### 1. 增加 CoreDNS 副本数

```bash
# 根据集群规模调整副本数
kubectl scale deployment coredns --replicas=3 -n kube-system
```

### 2. 配置 HPA 自动扩缩容

```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: coredns-hpa
  namespace: kube-system
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: coredns
  minReplicas: 2
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
```

### 3. 优化 Pod 的 ndots 配置

减少 DNS 查询次数:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: optimized-dns
spec:
  dnsConfig:
    options:
    - name: ndots
      value: "2"  # 默认是 5,减少可以减少查询次数
    - name: timeout
      value: "2"  # DNS 查询超时时间
    - name: attempts
      value: "2"  # DNS 查询重试次数
  containers:
  - name: app
    image: nginx
```

### 4. 使用 NodeLocal DNSCache

部署本地 DNS 缓存提升性能:

```bash
# 安装 NodeLocal DNSCache
kubectl apply -f https://k8s.io/examples/admin/dns/nodelocaldns.yaml
```

## 最佳实践

1. **生产环境建议**
   - 部署多个 CoreDNS 副本确保高可用
   - 配置 HPA 自动扩缩容
   - 使用 NodeLocal DNSCache 提升性能
   - 定期检查 DNS 查询性能

2. **性能优化**
   - 合理设置 DNS 缓存时间(cache)
   - 优化 ndots 配置减少查询
   - 增加 forward 并发数
   - 使用本地 DNS 缓存

3. **安全加固**
   - 限制 DNS 查询来源
   - 配置 DNS 查询日志审计
   - 定期更新 CoreDNS 版本
   - 使用 NetworkPolicy 保护 DNS

4. **监控告警**
   - 监控 CoreDNS 的 CPU 和内存使用
   - 监控 DNS 查询延迟
   - 配置 DNS 查询失败告警
   - 使用 Prometheus 收集指标

5. **故障处理**
   - 保留详细的 DNS 日志
   - 定期测试 DNS 解析
   - 准备 DNS 故障恢复方案
   - 配置多个上游 DNS 服务器

## 常见问题

### Q1: DNS 解析很慢怎么办?

A: 优化方法:
1. 增加 CoreDNS 副本数
2. 部署 NodeLocal DNSCache
3. 优化 ndots 配置
4. 增加 DNS 缓存时间
5. 检查上游 DNS 服务器性能

### Q2: 为什么域名解析失败?

A: 排查步骤:
```bash
# 1. 检查 CoreDNS 是否正常
kubectl get pods -n kube-system -l k8s-app=kube-dns

# 2. 查看 CoreDNS 日志
kubectl logs -n kube-system -l k8s-app=kube-dns

# 3. 检查 Pod 的 DNS 配置
kubectl exec -it <pod> -- cat /etc/resolv.conf

# 4. 测试 DNS 解析
kubectl exec -it <pod> -- nslookup kubernetes.default
```

### Q3: 如何配置使用外部 DNS?

A: 修改 Pod 的 dnsPolicy:
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: external-dns
spec:
  dnsPolicy: None
  dnsConfig:
    nameservers:
    - 8.8.8.8  # Google DNS
    - 8.8.4.4
    searches:
    - default.svc.cluster.local
    - svc.cluster.local
  containers:
  - name: app
    image: nginx
```

### Q4: 什么是 ndots?

A: `ndots` 决定了何时使用搜索域。
- 默认值: 5
- 如果域名中的 `.` 数量 < ndots,会依次尝试搜索域
- 例如: 查询 `nginx-service` 时会尝试:
  1. `nginx-service.default.svc.cluster.local`
  2. `nginx-service.svc.cluster.local`
  3. `nginx-service.cluster.local`
  4. `nginx-service.` (失败)

减少 ndots 可以减少 DNS 查询次数。

### Q5: CoreDNS 和 kube-dns 可以共存吗?

A: 不建议共存。选择其中一个作为集群 DNS:
```bash
# 查看当前 DNS
kubectl get svc -n kube-system kube-dns

# 从 kube-dns 迁移到 CoreDNS
kubectl delete deployment kube-dns -n kube-system
kubectl apply -f https://storage.googleapis.com/kubernetes-the-hard-way/coredns.yaml
```

## 总结

- ✅ 理解 Kubernetes DNS 的工作原理
- ✅ 掌握 CoreDNS 的配置和管理
- ✅ 理解 DNS 记录类型和命名规则
- ✅ 掌握 Pod 的 DNS 策略配置
- ✅ 了解性能优化和故障排查
- ✅ 能够解决常见 DNS 问题

## 参考资源

- [Kubernetes DNS 官方文档](https://kubernetes.io/docs/concepts/services-networking/dns-pod-service/)
- [CoreDNS 官方文档](https://coredns.io/manual/toc/)
- [CoreDNS 插件列表](https://coredns.io/plugins/)
- [NodeLocal DNSCache](https://kubernetes.io/docs/tasks/administer-cluster/nodelocaldns/)

---

*更新日期: 2025-12-03*
