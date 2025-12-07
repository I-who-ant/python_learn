# ConfigMap

> 配置管理、环境变量、配置文件挂载

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**ConfigMap 是 Kubernetes 用于存储非机密性配置数据的对象**。它允许将配置与容器镜像分离,使应用程序更加灵活和可移植,支持在不重新构建镜像的情况下更新配置。

## 核心概念

### 什么是 ConfigMap

ConfigMap 是一种 API 对象,用于存储键值对形式的配置数据。Pod 可以通过多种方式使用 ConfigMap:

1. **环境变量**: 将 ConfigMap 的数据注入到容器的环境变量
2. **命令行参数**: 通过环境变量传递给容器的命令
3. **配置文件**: 将 ConfigMap 挂载为 Volume 中的文件
4. **应用代码读取**: 通过 Kubernetes API 直接读取

### 为什么需要 ConfigMap

在生产环境中,ConfigMap 解决以下问题:

| 问题 | 解决方案 |
|------|---------|
| **配置与代码耦合** | 配置外部化,镜像可复用 |
| **多环境配置** | dev/staging/prod 使用不同 ConfigMap |
| **配置热更新** | 更新 ConfigMap 后 Pod 可重新加载 |
| **配置版本管理** | 配置作为 K8s 资源,可通过 Git 管理 |
| **集中管理** | 统一管理应用配置,便于维护 |

### ConfigMap vs Secret

| 特性 | ConfigMap | Secret |
|------|-----------|--------|
| **用途** | 非敏感配置 | 敏感信息(密码、证书) |
| **存储方式** | 明文 | Base64 编码 |
| **大小限制** | 1MB | 1MB |
| **安全性** | 无特殊保护 | etcd 可加密存储 |

## 创建 ConfigMap

### 方式 1: 从字面值创建

```bash
# 创建包含多个键值对的 ConfigMap
kubectl create configmap app-config \
  --from-literal=app.name=myapp \
  --from-literal=app.version=1.0.0 \
  --from-literal=log.level=info

# 查看创建的 ConfigMap
kubectl get configmap app-config -o yaml
```

**生成的 YAML**:
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
data:
  app.name: myapp
  app.version: 1.0.0
  log.level: info
```

### 方式 2: 从文件创建

```bash
# 创建配置文件
cat > app.properties <<EOF
database.host=mysql.example.com
database.port=3306
database.name=mydb
cache.ttl=3600
EOF

# 从文件创建 ConfigMap
kubectl create configmap app-properties \
  --from-file=app.properties

# 指定键名
kubectl create configmap app-properties \
  --from-file=config=app.properties
```

**生成的 YAML**:
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-properties
data:
  app.properties: |
    database.host=mysql.example.com
    database.port=3306
    database.name=mydb
    cache.ttl=3600
```

### 方式 3: 从目录创建

```bash
# 创建配置目录
mkdir config
echo "host=mysql" > config/database.conf
echo "level=info" > config/logging.conf

# 从目录创建 ConfigMap
kubectl create configmap app-configs \
  --from-file=config/

# 查看
kubectl describe configmap app-configs
```

### 方式 4: 使用 YAML 定义

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: nginx-config
  namespace: default
data:
  # 简单键值对
  server.name: "nginx-web"
  worker.processes: "4"

  # 完整配置文件
  nginx.conf: |
    user nginx;
    worker_processes auto;
    error_log /var/log/nginx/error.log;

    events {
        worker_connections 1024;
    }

    http {
        include /etc/nginx/mime.types;
        default_type application/octet-stream;

        server {
            listen 80;
            server_name localhost;

            location / {
                root /usr/share/nginx/html;
                index index.html;
            }
        }
    }
```

## 使用 ConfigMap

### 1. 作为环境变量

#### 1.1 引用单个键

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: env-single-key
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh", "-c", "env | grep APP"]
    env:
    - name: APP_NAME
      valueFrom:
        configMapKeyRef:
          name: app-config
          key: app.name
    - name: APP_VERSION
      valueFrom:
        configMapKeyRef:
          name: app-config
          key: app.version
```

**测试**:
```bash
kubectl apply -f env-single-key.yaml
kubectl logs env-single-key
# 输出:
# APP_NAME=myapp
# APP_VERSION=1.0.0
```

#### 1.2 引用所有键

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: env-all-keys
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh", "-c", "env | sort"]
    envFrom:
    - configMapRef:
        name: app-config
      prefix: CONFIG_  # 可选前缀
```

**效果**:
```bash
CONFIG_APP.NAME=myapp
CONFIG_APP.VERSION=1.0.0
CONFIG_LOG.LEVEL=info
```

### 2. 挂载为 Volume

#### 2.1 挂载整个 ConfigMap

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: volume-configmap
spec:
  containers:
  - name: nginx
    image: nginx:1.21
    volumeMounts:
    - name: config-volume
      mountPath: /etc/config
      readOnly: true
  volumes:
  - name: config-volume
    configMap:
      name: app-config
```

**结果**:
```bash
# 进入容器查看
kubectl exec -it volume-configmap -- ls -l /etc/config
# /etc/config/app.name
# /etc/config/app.version
# /etc/config/log.level

kubectl exec -it volume-configmap -- cat /etc/config/app.name
# myapp
```

#### 2.2 挂载特定键到指定路径

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: volume-specific-keys
spec:
  containers:
  - name: nginx
    image: nginx:1.21
    volumeMounts:
    - name: nginx-config
      mountPath: /etc/nginx/nginx.conf
      subPath: nginx.conf  # 挂载为文件而非目录
      readOnly: true
  volumes:
  - name: nginx-config
    configMap:
      name: nginx-config
      items:
      - key: nginx.conf
        path: nginx.conf
        mode: 0644
```

#### 2.3 设置文件权限

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: volume-with-mode
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh", "-c", "ls -l /etc/config && sleep 3600"]
    volumeMounts:
    - name: config-volume
      mountPath: /etc/config
  volumes:
  - name: config-volume
    configMap:
      name: app-config
      defaultMode: 0400  # 只读权限
      items:
      - key: app.name
        path: app-name.txt
        mode: 0600  # 特定文件权限
```

### 3. 用于命令行参数

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: command-args
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh"]
    args:
    - -c
    - echo "App: $(APP_NAME) Version: $(APP_VERSION)"
    env:
    - name: APP_NAME
      valueFrom:
        configMapKeyRef:
          name: app-config
          key: app.name
    - name: APP_VERSION
      valueFrom:
        configMapKeyRef:
          name: app-config
          key: app.version
```

## 更新 ConfigMap

### 更新配置

```bash
# 方式 1: 直接编辑
kubectl edit configmap app-config

# 方式 2: 使用 patch
kubectl patch configmap app-config \
  --type merge \
  -p '{"data":{"app.version":"2.0.0"}}'

# 方式 3: 从文件替换
echo "new value" > new-config.txt
kubectl create configmap app-config \
  --from-file=new-config.txt \
  --dry-run=client -o yaml | kubectl apply -f -
```

### 配置热更新

**重要**: ConfigMap 更新后的行为:

| 使用方式 | 更新行为 |
|---------|---------|
| **环境变量** | ❌ 不会自动更新,需重启 Pod |
| **Volume 挂载** | ✅ 自动更新(最多延迟 1 分钟 + kubelet 同步周期) |
| **subPath 挂载** | ❌ 不会自动更新,需重启 Pod |

**实现配置热更新**:

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: hot-reload-demo
spec:
  containers:
  - name: app
    image: nginx:1.21
    volumeMounts:
    - name: config
      mountPath: /etc/nginx/conf.d
    lifecycle:
      preStop:
        exec:
          command: ["/bin/sh", "-c", "nginx -s quit"]
  volumes:
  - name: config
    configMap:
      name: nginx-config
```

**监听配置变化**:
```bash
# 使用 inotify 监听文件变化
kubectl exec -it hot-reload-demo -- sh -c '
  apk add inotify-tools
  while inotifywait -e modify /etc/nginx/conf.d/; do
    nginx -s reload
  done
'
```

### 滚动更新策略

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: app-deployment
spec:
  replicas: 3
  template:
    metadata:
      # 在 annotations 中添加 ConfigMap 版本
      annotations:
        configmap.version: "v2"  # 更新此值触发滚动更新
    spec:
      containers:
      - name: app
        image: myapp:1.0
        volumeMounts:
        - name: config
          mountPath: /etc/config
      volumes:
      - name: config
        configMap:
          name: app-config
```

**触发滚动更新**:
```bash
# 更新 annotation 触发滚动
kubectl patch deployment app-deployment \
  -p '{"spec":{"template":{"metadata":{"annotations":{"configmap.version":"v3"}}}}}'
```

## 实战案例

### 案例 1: Nginx 配置管理

```yaml
---
# ConfigMap 定义
apiVersion: v1
kind: ConfigMap
metadata:
  name: nginx-html
data:
  index.html: |
    <!DOCTYPE html>
    <html>
    <head><title>Welcome</title></head>
    <body>
      <h1>Hello from ConfigMap!</h1>
      <p>Version: 1.0</p>
    </body>
    </html>

---
# Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-deployment
spec:
  replicas: 2
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
        ports:
        - containerPort: 80
        volumeMounts:
        - name: html
          mountPath: /usr/share/nginx/html
      volumes:
      - name: html
        configMap:
          name: nginx-html

---
# Service
apiVersion: v1
kind: Service
metadata:
  name: nginx-service
spec:
  selector:
    app: nginx
  ports:
  - port: 80
    targetPort: 80
  type: NodePort
```

**测试**:
```bash
kubectl apply -f nginx-configmap-demo.yaml
kubectl get svc nginx-service
# 访问 NodePort 查看页面

# 更新配置
kubectl edit configmap nginx-html
# 修改 Version: 2.0
# 等待 1-2 分钟后刷新页面查看变化
```

### 案例 2: 多环境配置

```yaml
---
# 开发环境 ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config-dev
  namespace: dev
data:
  database.url: "mysql://dev-db:3306/mydb"
  log.level: "debug"
  feature.flags: "experimental=true"

---
# 生产环境 ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config-prod
  namespace: prod
data:
  database.url: "mysql://prod-db:3306/mydb"
  log.level: "error"
  feature.flags: "experimental=false"

---
# 应用 Deployment (可复用)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: myapp
spec:
  template:
    spec:
      containers:
      - name: app
        image: myapp:1.0
        envFrom:
        - configMapRef:
            name: app-config-${ENV}  # 通过 Kustomize 替换
```

**使用 Kustomize 管理**:
```yaml
# kustomization.yaml
apiVersion: kustomize.config.k8s.io/v1beta1
kind: Kustomization
namespace: dev
resources:
  - deployment.yaml
configMapGenerator:
  - name: app-config-dev
    literals:
      - database.url=mysql://dev-db:3306/mydb
      - log.level=debug
```

### 案例 3: 应用配置注入

```yaml
---
# ConfigMap
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
data:
  application.yaml: |
    server:
      port: 8080
      context-path: /api

    spring:
      datasource:
        url: jdbc:mysql://mysql:3306/mydb
        username: user

    logging:
      level:
        root: INFO
        com.example: DEBUG

---
# Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: spring-app
spec:
  template:
    spec:
      containers:
      - name: app
        image: spring-app:1.0
        env:
        - name: SPRING_CONFIG_LOCATION
          value: /config/application.yaml
        volumeMounts:
        - name: config
          mountPath: /config
      volumes:
      - name: config
        configMap:
          name: app-config
```

## 最佳实践

### 1. 命名规范

```bash
# 推荐命名模式
<app-name>-<component>-<env>-config

# 示例
nginx-web-prod-config
mysql-db-dev-config
app-backend-staging-config
```

### 2. 大小限制

```yaml
# ConfigMap 大小限制为 1MB
# 对于大文件,考虑:
# 1. 使用 InitContainer 下载
# 2. 使用持久化存储
# 3. 拆分为多个 ConfigMap

apiVersion: v1
kind: Pod
metadata:
  name: large-config
spec:
  initContainers:
  - name: download-config
    image: busybox:1.35
    command:
    - sh
    - -c
    - wget -O /config/large-file.txt https://example.com/config.txt
    volumeMounts:
    - name: config
      mountPath: /config
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: config
      mountPath: /etc/config
  volumes:
  - name: config
    emptyDir: {}
```

### 3. 不可变 ConfigMap

```yaml
# 创建不可变 ConfigMap (Kubernetes 1.21+)
apiVersion: v1
kind: ConfigMap
metadata:
  name: immutable-config
data:
  key: value
immutable: true  # 创建后无法修改
```

**优点**:
- 防止意外更新导致故障
- 提高性能(kubelet 不需要监听变化)
- 更新时需创建新 ConfigMap 并更新引用

### 4. 版本管理

```yaml
# 在名称中包含版本号
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config-v2
  labels:
    version: "v2"
    app: myapp
data:
  config.json: |
    {"version": "2.0"}
```

**滚动更新流程**:
```bash
# 1. 创建新版本 ConfigMap
kubectl apply -f app-config-v2.yaml

# 2. 更新 Deployment 引用
kubectl set env deployment/myapp \
  --from=configmap/app-config-v2

# 3. 等待滚动完成后删除旧版本
kubectl delete configmap app-config-v1
```

### 5. 使用 Kustomize

```yaml
# base/kustomization.yaml
apiVersion: kustomize.config.k8s.io/v1beta1
kind: Kustomization
configMapGenerator:
- name: app-config
  literals:
  - log.level=info
  files:
  - config.properties

# overlays/prod/kustomization.yaml
apiVersion: kustomize.config.k8s.io/v1beta1
kind: Kustomization
bases:
- ../../base
configMapGenerator:
- name: app-config
  behavior: merge
  literals:
  - log.level=error
```

```bash
# 应用配置
kubectl apply -k overlays/prod/
```

## 常见问题

### Q1: ConfigMap 更新后 Pod 没有重启?

**A**:
- 环境变量方式不会自动更新,需要手动重启 Pod
- Volume 方式会自动更新,但应用需要支持配置热加载

```bash
# 强制重启 Deployment
kubectl rollout restart deployment/myapp

# 或者修改 annotation 触发滚动
kubectl patch deployment myapp \
  -p '{"spec":{"template":{"metadata":{"annotations":{"restarted-at":"'$(date +%s)'"}}}}}'
```

### Q2: 如何调试 ConfigMap 挂载问题?

**A**:
```bash
# 1. 检查 ConfigMap 是否存在
kubectl get configmap app-config

# 2. 查看 ConfigMap 内容
kubectl describe configmap app-config

# 3. 检查 Pod 事件
kubectl describe pod myapp-xxx

# 4. 进入容器查看挂载
kubectl exec -it myapp-xxx -- ls -la /etc/config
kubectl exec -it myapp-xxx -- cat /etc/config/app.name

# 5. 查看 kubelet 日志
journalctl -u kubelet | grep configmap
```

### Q3: ConfigMap 引用的键不存在会怎样?

**A**:
```yaml
# 可选键 (optional: true)
env:
- name: OPTIONAL_VAR
  valueFrom:
    configMapKeyRef:
      name: app-config
      key: non-existent-key
      optional: true  # Pod 可以正常启动

# 必需键 (默认)
env:
- name: REQUIRED_VAR
  valueFrom:
    configMapKeyRef:
      name: app-config
      key: non-existent-key  # Pod 无法启动
```

### Q4: 如何实现 ConfigMap 的权限控制?

**A**:
```yaml
# 使用 RBAC 限制访问
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: configmap-reader
  namespace: default
rules:
- apiGroups: [""]
  resources: ["configmaps"]
  resourceNames: ["app-config"]  # 只能访问特定 ConfigMap
  verbs: ["get", "list"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: read-configmaps
  namespace: default
subjects:
- kind: ServiceAccount
  name: myapp-sa
roleRef:
  kind: Role
  name: configmap-reader
  apiGroup: rbac.authorization.k8s.io
```

## 总结

- ✅ **配置外部化**: 将配置与镜像分离,提高灵活性
- ✅ **多种使用方式**: 环境变量、Volume、命令参数
- ✅ **配置热更新**: Volume 方式支持自动更新
- ✅ **版本管理**: 通过名称或标签管理配置版本
- ✅ **不可变配置**: 使用 immutable 防止意外修改
- ✅ **多环境支持**: 使用 Kustomize 管理不同环境配置
- ✅ **安全性**: 敏感信息使用 Secret,配置使用 ConfigMap

## 参考资源

- [ConfigMap 官方文档](https://kubernetes.io/docs/concepts/configuration/configmap/)
- [配置最佳实践](https://kubernetes.io/docs/concepts/configuration/overview/)
- [Kustomize 文档](https://kustomize.io/)
- [Configure Pod ConfigMap](https://kubernetes.io/docs/tasks/configure-pod-container/configure-pod-configmap/)

---

*更新日期: 2025-12-03*
