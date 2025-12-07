# Secret

> 敏感信息管理、Secret 类型、最佳实践

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**Secret 是 Kubernetes 用于存储和管理敏感信息的对象**,如密码、OAuth 令牌、SSH 密钥和 TLS 证书。Secret 将敏感数据与 Pod 定义分离,提高了安全性和灵活性。

## 核心概念

### 什么是 Secret

Secret 是包含少量敏感数据的对象,避免在 Pod 规约或容器镜像中明文存储机密信息。Secret 与 ConfigMap 类似,但专门用于保存机密数据。

### Secret vs ConfigMap

| 特性 | Secret | ConfigMap |
|------|--------|-----------|
| **用途** | 敏感信息(密码、密钥) | 非敏感配置 |
| **编码** | Base64 编码 | 明文 |
| **etcd 存储** | 可加密存储 | 明文存储 |
| **内存限制** | 挂载到 tmpfs(内存),不写磁盘 | 可能写入磁盘 |
| **API 访问** | 更严格的 RBAC 控制 | 相对宽松 |
| **大小限制** | 1MB | 1MB |

### Secret 类型

| 类型 | 用途 | 数据键 |
|------|------|--------|
| **Opaque** | 通用密钥(默认) | 任意键值对 |
| **kubernetes.io/service-account-token** | ServiceAccount 令牌 | token, ca.crt, namespace |
| **kubernetes.io/dockercfg** | Docker 配置文件(废弃) | .dockercfg |
| **kubernetes.io/dockerconfigjson** | Docker 镜像仓库认证 | .dockerconfigjson |
| **kubernetes.io/basic-auth** | 基本认证凭据 | username, password |
| **kubernetes.io/ssh-auth** | SSH 密钥 | ssh-privatekey |
| **kubernetes.io/tls** | TLS 证书 | tls.crt, tls.key |
| **bootstrap.kubernetes.io/token** | Bootstrap 令牌 | token-id, token-secret |

## 创建 Secret

### 1. 创建 Opaque Secret

#### 1.1 从字面值创建

```bash
# 创建通用 Secret
kubectl create secret generic db-credentials \
  --from-literal=username=admin \
  --from-literal=password='P@ssw0rd!'

# 查看 Secret (内容被隐藏)
kubectl get secret db-credentials
kubectl describe secret db-credentials
```

**生成的 YAML**:
```yaml
apiVersion: v1
kind: Secret
metadata:
  name: db-credentials
type: Opaque
data:
  username: YWRtaW4=       # base64 编码的 "admin"
  password: UEBzc3cwcmQh   # base64 编码的 "P@ssw0rd!"
```

#### 1.2 从文件创建

```bash
# 创建密钥文件
echo -n 'admin' > username.txt
echo -n 'P@ssw0rd!' > password.txt

# 从文件创建 Secret
kubectl create secret generic db-credentials \
  --from-file=username=username.txt \
  --from-file=password=password.txt

# 清理本地文件
rm username.txt password.txt
```

#### 1.3 使用 YAML 定义

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: db-credentials
type: Opaque
data:
  # Base64 编码的值
  username: YWRtaW4=
  password: UEBzc3cwcmQh
# 或使用 stringData (自动编码)
stringData:
  username: admin
  password: P@ssw0rd!
```

```bash
# Base64 编码/解码
echo -n 'admin' | base64      # 编码: YWRtaW4=
echo 'YWRtaW4=' | base64 -d   # 解码: admin
```

### 2. Docker 镜像仓库 Secret

```bash
# 创建 Docker Registry Secret
kubectl create secret docker-registry my-registry-key \
  --docker-server=registry.example.com \
  --docker-username=user \
  --docker-password=password \
  --docker-email=user@example.com
```

**生成的 Secret**:
```yaml
apiVersion: v1
kind: Secret
metadata:
  name: my-registry-key
type: kubernetes.io/dockerconfigjson
data:
  .dockerconfigjson: eyJhdXRocyI6eyJyZWdpc3RyeS5leGFtcGxlLmNvbSI6eyJ1c2VybmFtZSI6InVzZXIiLCJwYXNzd29yZCI6InBhc3N3b3JkIiwiZW1haWwiOiJ1c2VyQGV4YW1wbGUuY29tIiwiYXV0aCI6ImRYTmxjanB3WVhOemQyOXlaQT09In19fQ==
```

### 3. TLS Secret

```bash
# 生成自签名证书
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout tls.key -out tls.crt \
  -subj "/CN=example.com"

# 创建 TLS Secret
kubectl create secret tls my-tls-secret \
  --cert=tls.crt \
  --key=tls.key

# 清理
rm tls.crt tls.key
```

**生成的 Secret**:
```yaml
apiVersion: v1
kind: Secret
metadata:
  name: my-tls-secret
type: kubernetes.io/tls
data:
  tls.crt: LS0tLS1CRUdJTi... (Base64 编码的证书)
  tls.key: LS0tLS1CRUdJTi... (Base64 编码的私钥)
```

### 4. SSH 密钥 Secret

```bash
# 生成 SSH 密钥
ssh-keygen -t rsa -b 4096 -f id_rsa -N ""

# 创建 SSH Secret
kubectl create secret generic ssh-key-secret \
  --from-file=ssh-privatekey=id_rsa \
  --from-file=ssh-publickey=id_rsa.pub

# 或指定类型
kubectl create secret ssh-auth git-ssh-key \
  --from-file=ssh-privatekey=id_rsa
```

### 5. 基本认证 Secret

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: basic-auth-secret
type: kubernetes.io/basic-auth
stringData:
  username: admin
  password: secret123
```

## 使用 Secret

### 1. 作为环境变量

#### 1.1 引用单个键

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secret-env-pod
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh", "-c", "env | grep DB"]
    env:
    - name: DB_USERNAME
      valueFrom:
        secretKeyRef:
          name: db-credentials
          key: username
    - name: DB_PASSWORD
      valueFrom:
        secretKeyRef:
          name: db-credentials
          key: password
          optional: false  # 默认,键不存在则 Pod 无法启动
```

#### 1.2 引用所有键

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secret-envfrom-pod
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["/bin/sh", "-c", "env | sort"]
    envFrom:
    - secretRef:
        name: db-credentials
      prefix: DB_  # 可选前缀
```

### 2. 挂载为 Volume

#### 2.1 挂载整个 Secret

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secret-volume-pod
spec:
  containers:
  - name: app
    image: nginx:1.21
    volumeMounts:
    - name: secret-volume
      mountPath: /etc/secrets
      readOnly: true
  volumes:
  - name: secret-volume
    secret:
      secretName: db-credentials
      defaultMode: 0400  # 只读权限
```

**效果**:
```bash
kubectl exec -it secret-volume-pod -- ls -l /etc/secrets
# -r-------- 1 root root username
# -r-------- 1 root root password

kubectl exec -it secret-volume-pod -- cat /etc/secrets/username
# admin
```

#### 2.2 挂载特定键

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secret-specific-keys
spec:
  containers:
  - name: app
    image: nginx:1.21
    volumeMounts:
    - name: secret-volume
      mountPath: /etc/db-config
      readOnly: true
  volumes:
  - name: secret-volume
    secret:
      secretName: db-credentials
      items:
      - key: username
        path: db-username
        mode: 0600
      - key: password
        path: db-password
        mode: 0600
```

### 3. imagePullSecrets (拉取私有镜像)

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: private-image-pod
spec:
  containers:
  - name: app
    image: registry.example.com/myapp:1.0
  imagePullSecrets:
  - name: my-registry-key
```

**或在 ServiceAccount 中配置**:
```yaml
apiVersion: v1
kind: ServiceAccount
metadata:
  name: myapp-sa
imagePullSecrets:
- name: my-registry-key

---
apiVersion: v1
kind: Pod
metadata:
  name: private-image-pod
spec:
  serviceAccountName: myapp-sa
  containers:
  - name: app
    image: registry.example.com/myapp:1.0
```

### 4. TLS Secret 用于 Ingress

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: example-ingress
spec:
  tls:
  - hosts:
    - example.com
    secretName: my-tls-secret
  rules:
  - host: example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: web-service
            port:
              number: 80
```

## Secret 安全性

### 1. etcd 加密

**启用 etcd 加密存储**:

```yaml
# /etc/kubernetes/enc/encryption-config.yaml
apiVersion: apiserver.config.k8s.io/v1
kind: EncryptionConfiguration
resources:
  - resources:
    - secrets
    providers:
    - aescbc:
        keys:
        - name: key1
          secret: <base64-encoded-32-byte-key>
    - identity: {}
```

```bash
# API Server 启动参数
--encryption-provider-config=/etc/kubernetes/enc/encryption-config.yaml

# 生成加密密钥
head -c 32 /dev/urandom | base64

# 重新加密已有 Secret
kubectl get secrets --all-namespaces -o json | \
  kubectl replace -f -
```

### 2. RBAC 权限控制

```yaml
# 限制 Secret 访问
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: secret-reader
  namespace: default
rules:
- apiGroups: [""]
  resources: ["secrets"]
  resourceNames: ["db-credentials"]  # 只能访问特定 Secret
  verbs: ["get"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: read-secrets
  namespace: default
subjects:
- kind: ServiceAccount
  name: myapp-sa
roleRef:
  kind: Role
  name: secret-reader
  apiGroup: rbac.authorization.k8s.io
```

### 3. 不可变 Secret

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: immutable-secret
type: Opaque
data:
  username: YWRtaW4=
immutable: true  # Kubernetes 1.21+
```

**优点**:
- 防止意外或恶意修改
- 提高性能(kubelet 无需监听变化)
- 更新需要创建新 Secret

## 实战案例

### 案例 1: MySQL 数据库凭据

```yaml
---
# Secret
apiVersion: v1
kind: Secret
metadata:
  name: mysql-secret
  namespace: default
type: Opaque
stringData:
  mysql-root-password: rootpassword
  mysql-user: app_user
  mysql-password: app_password

---
# MySQL Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: mysql
spec:
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
        env:
        - name: MYSQL_ROOT_PASSWORD
          valueFrom:
            secretKeyRef:
              name: mysql-secret
              key: mysql-root-password
        - name: MYSQL_USER
          valueFrom:
            secretKeyRef:
              name: mysql-secret
              key: mysql-user
        - name: MYSQL_PASSWORD
          valueFrom:
            secretKeyRef:
              name: mysql-secret
              key: mysql-password
        - name: MYSQL_DATABASE
          value: myapp
        ports:
        - containerPort: 3306

---
# 应用连接 MySQL
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
        env:
        - name: DB_HOST
          value: mysql
        - name: DB_USER
          valueFrom:
            secretKeyRef:
              name: mysql-secret
              key: mysql-user
        - name: DB_PASSWORD
          valueFrom:
            secretKeyRef:
              name: mysql-secret
              key: mysql-password
```

### 案例 2: TLS 证书管理

```yaml
---
# TLS Secret (Let's Encrypt 证书)
apiVersion: v1
kind: Secret
metadata:
  name: example-tls
  namespace: default
type: kubernetes.io/tls
data:
  tls.crt: LS0tLS1CRUdJTi... (证书内容)
  tls.key: LS0tLS1CRUdJTi... (私钥内容)

---
# Ingress 使用 TLS
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: example-ingress
  annotations:
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
spec:
  ingressClassName: nginx
  tls:
  - hosts:
    - example.com
    - www.example.com
    secretName: example-tls
  rules:
  - host: example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: web
            port:
              number: 80
```

### 案例 3: 私有镜像仓库

```yaml
---
# Docker Registry Secret
apiVersion: v1
kind: Secret
metadata:
  name: harbor-registry
  namespace: default
type: kubernetes.io/dockerconfigjson
data:
  .dockerconfigjson: eyJhdXRocyI6eyJoYXJib3IuZXhhbXBsZS5jb20iOnsidXNlcm5hbWUiOiJhZG1pbiIsInBhc3N3b3JkIjoiSGFyYm9yMTIzNDUiLCJhdXRoIjoiWVdSdGFXNDZTR0Z5WW05eU1USXpORFU9In19fQ==

---
# ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: default
  namespace: default
imagePullSecrets:
- name: harbor-registry

---
# Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: myapp
spec:
  template:
    spec:
      # 自动使用 ServiceAccount 的 imagePullSecrets
      containers:
      - name: app
        image: harbor.example.com/myproject/myapp:1.0
```

## 最佳实践

### 1. 外部密钥管理系统

```yaml
# 使用 External Secrets Operator
apiVersion: external-secrets.io/v1beta1
kind: ExternalSecret
metadata:
  name: vault-secret
spec:
  refreshInterval: 1h
  secretStoreRef:
    name: vault-backend
    kind: SecretStore
  target:
    name: db-credentials
    creationPolicy: Owner
  data:
  - secretKey: username
    remoteRef:
      key: secret/data/database
      property: username
  - secretKey: password
    remoteRef:
      key: secret/data/database
      property: password
```

**集成方案**:
- **HashiCorp Vault**: 企业级密钥管理
- **AWS Secrets Manager**: AWS 原生服务
- **Azure Key Vault**: Azure 云服务
- **Google Secret Manager**: GCP 服务
- **Sealed Secrets**: GitOps 友好的加密方案

### 2. Sealed Secrets (GitOps)

```bash
# 安装 Sealed Secrets Controller
kubectl apply -f https://github.com/bitnami-labs/sealed-secrets/releases/download/v0.18.0/controller.yaml

# 安装 kubeseal CLI
brew install kubeseal

# 创建加密 Secret
kubectl create secret generic mysecret \
  --from-literal=password=secret123 \
  --dry-run=client -o yaml | \
  kubeseal -o yaml > sealed-secret.yaml

# 可以安全地提交到 Git
git add sealed-secret.yaml
git commit -m "Add sealed secret"
```

**SealedSecret 示例**:
```yaml
apiVersion: bitnami.com/v1alpha1
kind: SealedSecret
metadata:
  name: mysecret
spec:
  encryptedData:
    password: AgBy3i4OJSWK+PiTySYZZA9rO43cGDEq...
```

### 3. 最小权限原则

```yaml
# Pod 只挂载需要的 Secret 键
apiVersion: v1
kind: Pod
metadata:
  name: app
spec:
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: db-password
      mountPath: /run/secrets/db
      readOnly: true
  volumes:
  - name: db-password
    secret:
      secretName: db-credentials
      items:
      - key: password  # 只挂载密码,不挂载用户名
        path: password
```

### 4. 定期轮换密钥

```bash
# 1. 创建新版本 Secret
kubectl create secret generic db-credentials-v2 \
  --from-literal=password='NewP@ssw0rd!'

# 2. 更新应用引用
kubectl set env deployment/myapp \
  --from=secret/db-credentials-v2

# 3. 验证后删除旧 Secret
kubectl delete secret db-credentials
```

### 5. 审计日志

```yaml
# API Server 审计策略
apiVersion: audit.k8s.io/v1
kind: Policy
rules:
- level: Metadata
  resources:
  - group: ""
    resources: ["secrets"]
  verbs: ["get", "list", "watch"]
- level: RequestResponse
  resources:
  - group: ""
    resources: ["secrets"]
  verbs: ["create", "update", "patch", "delete"]
```

## 常见问题

### Q1: Secret 是否真的安全?

**A**: Secret 提供基础安全,但需要额外措施:
- ✅ Base64 编码(非加密)
- ✅ etcd 可配置加密存储
- ✅ 传输过程使用 TLS
- ✅ 挂载到 tmpfs(内存),不写磁盘
- ❌ 拥有 namespace 访问权限可查看所有 Secret
- ❌ etcd 备份默认未加密

**加固建议**:
1. 启用 etcd 加密
2. 使用外部密钥管理系统(Vault、AWS Secrets Manager)
3. 严格的 RBAC 权限控制
4. 定期轮换密钥
5. 审计日志监控

### Q2: 如何安全地传递 Secret 到集群?

**A**:
```bash
# 方法 1: 使用 kubectl (从本地文件)
kubectl create secret generic my-secret \
  --from-file=./secret.txt

# 方法 2: 使用管道 (避免文件落盘)
cat secret.txt | kubectl create secret generic my-secret \
  --from-file=/dev/stdin

# 方法 3: 使用 Sealed Secrets
kubeseal < secret.yaml > sealed-secret.yaml

# 方法 4: 使用 Vault
vault kv put secret/myapp password=secret123
# 配置 External Secrets Operator 同步
```

### Q3: Secret 更新后 Pod 会自动重启吗?

**A**: 不会,需要手动触发:

```bash
# 方式 1: 滚动重启
kubectl rollout restart deployment/myapp

# 方式 2: 使用 Reloader (自动监听 Secret 变化)
kubectl apply -f https://raw.githubusercontent.com/stakater/Reloader/master/deployments/kubernetes/reloader.yaml

# 在 Deployment 添加 annotation
annotations:
  reloader.stakater.com/auto: "true"
```

### Q4: 如何在 CI/CD 中管理 Secret?

**A**:
```yaml
# GitLab CI 示例
deploy:
  script:
    # 从 CI 变量创建 Secret
    - kubectl create secret generic app-secret \
        --from-literal=api-key=$API_KEY \
        --dry-run=client -o yaml | kubectl apply -f -

    # 部署应用
    - kubectl apply -f deployment.yaml
  environment:
    name: production
```

**推荐方案**:
- 使用 Sealed Secrets 加密后提交到 Git
- 使用云平台的 Secret Manager
- 使用 Vault 集成
- CI/CD 工具的加密变量功能

## 总结

- ✅ **安全存储**: 敏感信息与代码分离,降低泄露风险
- ✅ **多种类型**: Opaque、TLS、Docker、SSH 等专用类型
- ✅ **灵活使用**: 环境变量、Volume 挂载、imagePullSecrets
- ✅ **加密存储**: 启用 etcd 加密保护静态数据
- ✅ **访问控制**: RBAC 细粒度权限管理
- ✅ **外部集成**: Vault、AWS Secrets Manager 等企业级方案
- ✅ **GitOps 友好**: Sealed Secrets 实现加密存储到 Git

## 参考资源

- [Secret 官方文档](https://kubernetes.io/docs/concepts/configuration/secret/)
- [加密 etcd 数据](https://kubernetes.io/docs/tasks/administer-cluster/encrypt-data/)
- [Sealed Secrets](https://github.com/bitnami-labs/sealed-secrets)
- [External Secrets Operator](https://external-secrets.io/)
- [HashiCorp Vault](https://www.vaultproject.io/docs/platform/k8s)

---

*更新日期: 2025-12-03*
