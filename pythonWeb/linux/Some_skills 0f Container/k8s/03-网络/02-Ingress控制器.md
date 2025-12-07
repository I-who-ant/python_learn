# Ingress控制器

> Nginx Ingress、Traefik、实践配置

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Ingress Controller 是 Kubernetes 中实际执行 Ingress 路由规则的组件。它监听 Ingress 资源的变化,并根据配置动态更新负载均衡器规则。Ingress 资源只是定义了路由规则,而 Ingress Controller 才是真正处理流量的实体。

## 核心概念

### 什么是 Ingress Controller

Ingress Controller 是运行在 Kubernetes 集群中的 Pod,主要功能包括:

- **监听资源变化**: 实时监听 Ingress、Service、Endpoint 等资源
- **配置更新**: 动态生成负载均衡器配置(如 Nginx 配置)
- **流量转发**: 接收外部流量并转发到后端 Service
- **TLS 终止**: 处理 HTTPS 加密/解密

### 为什么需要 Ingress Controller

在生产环境中,我们需要 Ingress Controller 来解决以下问题:

1. **Ingress 资源执行**: Ingress 资源本身不具备处理流量的能力
2. **统一流量入口**: 为整个集群提供统一的外部访问入口
3. **灵活路由**: 支持基于域名、路径、Header 等的复杂路由规则
4. **功能扩展**: 提供限流、认证、监控等增强功能

### 常见 Ingress Controller

| 控制器 | 特点 | 适用场景 |
|--------|------|----------|
| **Nginx Ingress** | 成熟稳定,功能丰富,社区活跃 | 生产环境首选 |
| **Traefik** | 配置简单,自带 Dashboard,支持自动发现 | 中小型项目 |
| **HAProxy Ingress** | 高性能,企业级 | 高并发场景 |
| **Kong Ingress** | 基于 API 网关,插件丰富 | 微服务 API 管理 |
| **Contour** | 基于 Envoy,支持 HTTPProxy CRD | 云原生应用 |
| **AWS ALB Ingress** | 原生支持 AWS ALB | AWS 环境 |

## Nginx Ingress Controller

### 安装部署

#### 方法 1: 使用 Helm 安装(推荐)

```bash
# 添加 Helm 仓库
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx
helm repo update

# 安装 Nginx Ingress Controller
helm install ingress-nginx ingress-nginx/ingress-nginx \
  --namespace ingress-nginx \
  --create-namespace \
  --set controller.replicaCount=2 \
  --set controller.nodeSelector."kubernetes\.io/os"=linux

# 查看安装状态
kubectl get pods -n ingress-nginx
kubectl get svc -n ingress-nginx
```

#### 方法 2: 使用 kubectl 安装

```bash
# 下载官方 YAML
kubectl apply -f https://raw.githubusercontent.com/kubernetes/ingress-nginx/controller-v1.8.1/deploy/static/provider/cloud/deploy.yaml

# 或者使用 bare-metal 版本
kubectl apply -f https://raw.githubusercontent.com/kubernetes/ingress-nginx/controller-v1.8.1/deploy/static/provider/baremetal/deploy.yaml
```

#### 方法 3: 自定义部署

```yaml
# nginx-ingress-controller.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-ingress-controller
  namespace: ingress-nginx
spec:
  replicas: 2
  selector:
    matchLabels:
      app: nginx-ingress
  template:
    metadata:
      labels:
        app: nginx-ingress
    spec:
      serviceAccountName: nginx-ingress-serviceaccount
      containers:
      - name: nginx-ingress-controller
        image: registry.k8s.io/ingress-nginx/controller:v1.8.1
        args:
          - /nginx-ingress-controller
          - --configmap=$(POD_NAMESPACE)/nginx-configuration
          - --tcp-services-configmap=$(POD_NAMESPACE)/tcp-services
          - --udp-services-configmap=$(POD_NAMESPACE)/udp-services
          - --publish-service=$(POD_NAMESPACE)/ingress-nginx
          - --annotations-prefix=nginx.ingress.kubernetes.io
        env:
          - name: POD_NAME
            valueFrom:
              fieldRef:
                fieldPath: metadata.name
          - name: POD_NAMESPACE
            valueFrom:
              fieldRef:
                fieldPath: metadata.namespace
        ports:
        - name: http
          containerPort: 80
        - name: https
          containerPort: 443
        livenessProbe:
          httpGet:
            path: /healthz
            port: 10254
          initialDelaySeconds: 10
        readinessProbe:
          httpGet:
            path: /healthz
            port: 10254

---
apiVersion: v1
kind: Service
metadata:
  name: ingress-nginx
  namespace: ingress-nginx
spec:
  type: LoadBalancer  # 或 NodePort
  ports:
  - name: http
    port: 80
    targetPort: 80
  - name: https
    port: 443
    targetPort: 443
  selector:
    app: nginx-ingress
```

### 验证安装

```bash
# 检查 Pod 状态
kubectl get pods -n ingress-nginx

# 检查 Service
kubectl get svc -n ingress-nginx

# 查看日志
kubectl logs -n ingress-nginx -l app.kubernetes.io/name=ingress-nginx

# 检查版本
kubectl exec -n ingress-nginx <pod-name> -- /nginx-ingress-controller --version
```

### 配置项

#### 全局配置 (ConfigMap)

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: nginx-configuration
  namespace: ingress-nginx
data:
  # 日志格式
  log-format-upstream: '$remote_addr - $remote_user [$time_local] "$request" $status $body_bytes_sent "$http_referer" "$http_user_agent"'

  # 客户端请求体大小限制
  proxy-body-size: "100m"

  # 超时设置
  proxy-connect-timeout: "30"
  proxy-send-timeout: "30"
  proxy-read-timeout: "30"

  # 启用 gzip
  use-gzip: "true"
  gzip-level: "5"

  # SSL 配置
  ssl-protocols: "TLSv1.2 TLSv1.3"
  ssl-ciphers: "HIGH:!aNULL:!MD5"

  # 隐藏版本信息
  server-tokens: "false"

  # 启用 HTTP/2
  use-http2: "true"

  # 连接限制
  limit-connections: "100"
```

#### TCP/UDP 服务暴露

```yaml
# tcp-services-configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: tcp-services
  namespace: ingress-nginx
data:
  # 格式: <port>: "<namespace>/<service>:<port>"
  3306: "default/mysql-service:3306"
  6379: "default/redis-service:6379"

---
# udp-services-configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: udp-services
  namespace: ingress-nginx
data:
  53: "kube-system/kube-dns:53"
```

## kubectl 命令

```bash
# 查看 Ingress Controller
kubectl get pods -n ingress-nginx
kubectl get deployment -n ingress-nginx

# 查看 Ingress Controller 日志
kubectl logs -n ingress-nginx -l app.kubernetes.io/name=ingress-nginx --tail=100 -f

# 查看 Nginx 配置
kubectl exec -n ingress-nginx <pod-name> -- cat /etc/nginx/nginx.conf

# 重启 Ingress Controller
kubectl rollout restart deployment ingress-nginx-controller -n ingress-nginx

# 查看配置
kubectl get cm nginx-configuration -n ingress-nginx -o yaml

# 编辑全局配置
kubectl edit cm nginx-configuration -n ingress-nginx

# 查看 Ingress Controller 版本
kubectl get deployment -n ingress-nginx ingress-nginx-controller -o jsonpath='{.spec.template.spec.containers[0].image}'

# 查看所有 Ingress 资源
kubectl get ingress --all-namespaces

# 测试配置语法
kubectl exec -n ingress-nginx <pod-name> -- nginx -t
```

## 实战案例

### 案例 1: 多环境部署

为开发、测试、生产环境部署独立的 Ingress Controller:

```bash
# 生产环境
helm install ingress-prod ingress-nginx/ingress-nginx \
  --namespace ingress-prod \
  --create-namespace \
  --set controller.ingressClassResource.name=nginx-prod \
  --set controller.replicaCount=3

# 测试环境
helm install ingress-test ingress-nginx/ingress-nginx \
  --namespace ingress-test \
  --create-namespace \
  --set controller.ingressClassResource.name=nginx-test \
  --set controller.replicaCount=2
```

使用不同的 IngressClass:

```yaml
# 生产环境 Ingress
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: prod-ingress
spec:
  ingressClassName: nginx-prod  # 使用生产环境控制器
  rules:
  - host: app.example.com
    # ...

---
# 测试环境 Ingress
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: test-ingress
spec:
  ingressClassName: nginx-test  # 使用测试环境控制器
  rules:
  - host: test.example.com
    # ...
```

### 案例 2: 高可用部署

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-ingress-controller
  namespace: ingress-nginx
spec:
  replicas: 3  # 多副本
  strategy:
    rollingUpdate:
      maxSurge: 1
      maxUnavailable: 0
  selector:
    matchLabels:
      app: nginx-ingress
  template:
    metadata:
      labels:
        app: nginx-ingress
    spec:
      # 反亲和性,避免多个 Pod 在同一节点
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
                  - nginx-ingress
              topologyKey: kubernetes.io/hostname

      # 优先调度到高性能节点
      nodeSelector:
        node-role: ingress

      containers:
      - name: nginx-ingress-controller
        image: registry.k8s.io/ingress-nginx/controller:v1.8.1
        resources:
          requests:
            cpu: 500m
            memory: 512Mi
          limits:
            cpu: 2000m
            memory: 2Gi
        # ... 其他配置
```

配置 HPA 自动扩缩容:

```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: nginx-ingress-hpa
  namespace: ingress-nginx
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: nginx-ingress-controller
  minReplicas: 3
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

### 案例 3: 监控和日志

配置 Prometheus 监控:

```yaml
apiVersion: v1
kind: Service
metadata:
  name: ingress-nginx-metrics
  namespace: ingress-nginx
  labels:
    app: nginx-ingress
  annotations:
    prometheus.io/scrape: "true"
    prometheus.io/port: "10254"
spec:
  ports:
  - name: metrics
    port: 10254
    targetPort: 10254
  selector:
    app: nginx-ingress
```

配置访问日志输出到标准输出:

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: nginx-configuration
  namespace: ingress-nginx
data:
  log-format-escape-json: "true"
  log-format-upstream: '{"time": "$time_iso8601", "remote_addr": "$remote_addr", "request": "$request", "status": $status, "bytes_sent": $bytes_sent, "request_time": $request_time, "upstream_response_time": "$upstream_response_time"}'
```

### 案例 4: 默认后端服务

部署自定义 404 页面:

```yaml
# default-backend.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: default-http-backend
  namespace: ingress-nginx
spec:
  replicas: 2
  selector:
    matchLabels:
      app: default-http-backend
  template:
    metadata:
      labels:
        app: default-http-backend
    spec:
      containers:
      - name: default-backend
        image: registry.k8s.io/defaultbackend-amd64:1.5
        ports:
        - containerPort: 8080

---
apiVersion: v1
kind: Service
metadata:
  name: default-http-backend
  namespace: ingress-nginx
spec:
  ports:
  - port: 80
    targetPort: 8080
  selector:
    app: default-http-backend
```

配置 Ingress Controller 使用自定义默认后端:

```bash
helm install ingress-nginx ingress-nginx/ingress-nginx \
  --namespace ingress-nginx \
  --set defaultBackend.enabled=true \
  --set defaultBackend.image.repository=registry.k8s.io/defaultbackend-amd64 \
  --set defaultBackend.image.tag=1.5
```

## Traefik Ingress Controller

### 快速安装

```bash
# 使用 Helm 安装
helm repo add traefik https://traefik.github.io/charts
helm repo update

helm install traefik traefik/traefik \
  --namespace traefik \
  --create-namespace \
  --set dashboard.enabled=true \
  --set dashboard.domain=traefik.example.com

# 查看状态
kubectl get pods -n traefik
kubectl get svc -n traefik
```

### Traefik Dashboard

访问 Traefik Dashboard:

```bash
# 端口转发
kubectl port-forward -n traefik $(kubectl get pods -n traefik -l app.kubernetes.io/name=traefik -o name) 9000:9000

# 访问: http://localhost:9000/dashboard/
```

### Traefik Ingress 示例

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: traefik-ingress
  annotations:
    traefik.ingress.kubernetes.io/router.entrypoints: web,websecure
    traefik.ingress.kubernetes.io/router.tls: "true"
spec:
  rules:
  - host: app.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: app-service
            port:
              number: 80
```

## 最佳实践

1. **生产环境建议**
   - 使用 Helm 部署便于升级和管理
   - 部署多副本确保高可用
   - 使用 NodePort 或 LoadBalancer 暴露服务
   - 为不同环境部署独立的 Ingress Controller

2. **性能优化**
   - 合理设置资源限制(requests/limits)
   - 启用 HTTP/2 和 gzip 压缩
   - 配置 HPA 自动扩缩容
   - 使用专用节点部署 Ingress Controller

3. **安全加固**
   - 隐藏服务器版本信息
   - 限制请求体大小防止攻击
   - 配置 IP 白名单
   - 定期更新镜像版本
   - 使用网络策略限制 Pod 通信

4. **监控告警**
   - 集成 Prometheus 监控指标
   - 配置日志收集(ELK/Loki)
   - 监控 CPU、内存、连接数
   - 设置响应时间和错误率告警

5. **故障排查**
   - 保留足够的日志便于排查
   - 配置健康检查探针
   - 使用 kubectl logs 查看实时日志
   - 定期备份配置文件

## 常见问题

### Q1: Ingress Controller 无法启动?

A: 排查步骤:
1. 检查镜像是否拉取成功
   ```bash
   kubectl describe pod -n ingress-nginx <pod-name>
   ```
2. 检查 RBAC 权限是否正确
3. 查看 Pod 日志
   ```bash
   kubectl logs -n ingress-nginx <pod-name>
   ```
4. 检查端口是否被占用

### Q2: 如何升级 Ingress Controller?

A: 使用 Helm 升级:
```bash
# 查看当前版本
helm list -n ingress-nginx

# 更新仓库
helm repo update

# 升级
helm upgrade ingress-nginx ingress-nginx/ingress-nginx \
  --namespace ingress-nginx \
  --reuse-values
```

### Q3: 如何查看 Nginx 配置?

A:
```bash
# 查看完整配置
kubectl exec -n ingress-nginx <pod-name> -- cat /etc/nginx/nginx.conf

# 测试配置语法
kubectl exec -n ingress-nginx <pod-name> -- nginx -t

# 重新加载配置(无需重启)
kubectl exec -n ingress-nginx <pod-name> -- nginx -s reload
```

### Q4: Nginx Ingress 和 Kubernetes Ingress 的区别?

A:
- **Kubernetes Ingress**: 是 K8s 的 API 资源,定义路由规则
- **Nginx Ingress Controller**: 是具体实现,读取 Ingress 资源并执行路由

两者配合使用: Ingress 定义规则 → Ingress Controller 执行规则

### Q5: 如何处理大文件上传?

A: 修改 ConfigMap 配置:
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: nginx-configuration
  namespace: ingress-nginx
data:
  proxy-body-size: "500m"  # 增加请求体大小限制
  proxy-request-buffering: "off"  # 关闭缓冲
```

或在 Ingress 中使用 annotation:
```yaml
metadata:
  annotations:
    nginx.ingress.kubernetes.io/proxy-body-size: "500m"
```

## 总结

- ✅ 理解 Ingress Controller 的作用和原理
- ✅ 掌握 Nginx Ingress Controller 的安装和配置
- ✅ 了解 Traefik 等其他 Ingress Controller
- ✅ 掌握高可用部署和性能优化
- ✅ 理解监控、日志和故障排查方法
- ✅ 能够解决常见问题

## 参考资源

- [Nginx Ingress Controller 官方文档](https://kubernetes.github.io/ingress-nginx/)
- [Nginx Ingress Controller GitHub](https://github.com/kubernetes/ingress-nginx)
- [Traefik 官方文档](https://doc.traefik.io/traefik/)
- [Ingress Controller 对比](https://docs.google.com/spreadsheets/d/191WWNpjJ2za6-nbG4ZoUMXMpUK8KlCIosvQB0f-oq3k)

---

*更新日期: 2025-12-03*
