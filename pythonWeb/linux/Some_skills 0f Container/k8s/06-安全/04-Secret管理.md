# Secret管理

> Secret 加密、外部密钥管理、Sealed Secrets

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**Secret 管理是 Kubernetes 安全的关键环节**。通过 Secret 加密、外部密钥管理系统(External Secrets、Vault)和 Sealed Secrets,可以安全存储和分发敏感信息(密码、Token、证书等),防止凭证泄露,实现密钥轮转和审计。

## 核心概念

### 什么是 Secret

Secret 是 Kubernetes 用于存储敏感信息的对象:

| 组件 | 说明 | 用途 |
|-----|------|------|
| **Secret** | 敏感数据对象 | 存储密码、Token、证书 |
| **Base64 编码** | 数据编码方式 | 非加密,仅编码 |
| **Encryption at Rest** | 静态加密 | etcd 存储加密 |
| **External Secrets** | 外部密钥管理 | 集成 Vault、AWS Secrets Manager |
| **Sealed Secrets** | 加密 Secret | GitOps 安全存储 |

### Secret 类型

| 类型 | 用途 | 示例 |
|-----|------|------|
| **Opaque** | 通用密钥(默认) | 密码、API Token |
| **kubernetes.io/service-account-token** | ServiceAccount Token | 自动生成 |
| **kubernetes.io/dockerconfigjson** | Docker 镜像仓库凭证 | 私有镜像拉取 |
| **kubernetes.io/tls** | TLS 证书 | HTTPS 证书 |
| **kubernetes.io/basic-auth** | 基础认证 | 用户名密码 |
| **kubernetes.io/ssh-auth** | SSH 密钥 | Git 仓库访问 |

### 为什么需要 Secret 管理

| 安全问题 | 后果 | Secret 管理解决方案 |
|---------|------|------------------|
| **明文存储** | 密码泄露 | Base64 编码 + 静态加密 |
| **Git 仓库泄露** | 凭证公开 | Sealed Secrets |
| **集中管理困难** | 多集群凭证不一致 | External Secrets |
| **无审计记录** | 无法追踪访问 | Vault 审计日志 |
| **密钥轮转困难** | 旧密钥长期有效 | 自动轮转 |

---

## Secret 基础

### 1. 创建 Secret

#### 1.1 命令行创建

```bash
# 通用 Secret
kubectl create secret generic db-credentials \
  --from-literal=username=admin \
  --from-literal=password=P@ssw0rd

# 从文件创建
echo -n 'admin' > username.txt
echo -n 'P@ssw0rd' > password.txt
kubectl create secret generic db-credentials \
  --from-file=username.txt \
  --from-file=password.txt

# Docker 镜像仓库凭证
kubectl create secret docker-registry docker-secret \
  --docker-server=https://index.docker.io/v1/ \
  --docker-username=myuser \
  --docker-password=mypassword \
  --docker-email=myemail@example.com

# TLS 证书
kubectl create secret tls tls-secret \
  --cert=path/to/tls.crt \
  --key=path/to/tls.key
```

#### 1.2 YAML 创建

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: db-credentials
  namespace: default
type: Opaque
data:
  # Base64 编码后的数据
  username: YWRtaW4=        # echo -n 'admin' | base64
  password: UEBzc3cwcmQ=    # echo -n 'P@ssw0rd' | base64

# 或使用 stringData(自动 Base64 编码)
stringData:
  username: admin
  password: P@ssw0rd
```

### 2. 使用 Secret

#### 2.1 环境变量

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: myapp
spec:
  containers:
  - name: app
    image: myapp:1.0
    env:
    # 单个 Key
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

    # 所有 Keys
    envFrom:
    - secretRef:
        name: db-credentials
```

#### 2.2 Volume 挂载

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: myapp
spec:
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: secrets
      mountPath: /etc/secrets
      readOnly: true
  volumes:
  - name: secrets
    secret:
      secretName: db-credentials
      # 可选: 指定特定 Key
      items:
      - key: username
        path: username.txt
      - key: password
        path: password.txt
```

**Pod 内部访问**:

```bash
# 进入 Pod
kubectl exec -it myapp -- sh

# 查看挂载的文件
ls /etc/secrets/
# username.txt  password.txt

# 读取内容
cat /etc/secrets/username.txt
# admin
```

### 3. 查看 Secret

```bash
# 列出 Secret
kubectl get secrets -n default

# 查看详情(不显示内容)
kubectl describe secret db-credentials -n default

# 查看内容(Base64 编码)
kubectl get secret db-credentials -n default -o yaml

# 解码 Base64
kubectl get secret db-credentials -n default -o jsonpath='{.data.password}' | base64 -d
# P@ssw0rd

# 使用 jq 解码所有 Key
kubectl get secret db-credentials -n default -o json | \
  jq -r '.data | to_entries[] | "\(.key): \(.value | @base64d)"'
```

---

## Secret 加密

### 1. etcd 静态加密

**Kubernetes 默认 Secret 存储在 etcd 中是 Base64 编码,非加密**。需要启用 Encryption at Rest。

#### 1.1 创建加密配置

```yaml
# encryption-config.yaml
apiVersion: apiserver.config.k8s.io/v1
kind: EncryptionConfiguration
resources:
  - resources:
      - secrets
    providers:
    # AES-CBC 加密
    - aescbc:
        keys:
        - name: key1
          secret: <BASE64_ENCODED_32_BYTE_KEY>  # 生成: head -c 32 /dev/urandom | base64
    # 回退到未加密(用于迁移)
    - identity: {}
```

**生成加密密钥**:

```bash
# 生成 32 字节随机密钥
head -c 32 /dev/urandom | base64
# 输出: 3q2+796tvu9erb7vg73vv73vv71E77+9Ju+/ve+/ve+/vQ==
```

#### 1.2 配置 API Server

```yaml
# /etc/kubernetes/manifests/kube-apiserver.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kube-apiserver
  namespace: kube-system
spec:
  containers:
  - name: kube-apiserver
    command:
    - kube-apiserver
    - --encryption-provider-config=/etc/kubernetes/encryption-config.yaml
    volumeMounts:
    - name: encryption-config
      mountPath: /etc/kubernetes/encryption-config.yaml
      readOnly: true
  volumes:
  - name: encryption-config
    hostPath:
      path: /etc/kubernetes/encryption-config.yaml
      type: File
```

#### 1.3 重新加密现有 Secret

```bash
# API Server 重启后,新 Secret 会加密,但旧 Secret 仍是明文
# 需要手动重新加密

# 重新加密所有 Secret
kubectl get secrets --all-namespaces -o json | \
  kubectl replace -f -

# 验证加密(直接查看 etcd)
ETCDCTL_API=3 etcdctl get /registry/secrets/default/db-credentials \
  --endpoints=https://127.0.0.1:2379 \
  --cacert=/etc/kubernetes/pki/etcd/ca.crt \
  --cert=/etc/kubernetes/pki/etcd/server.crt \
  --key=/etc/kubernetes/pki/etcd/server.key

# 加密后输出乱码,而非明文
```

### 2. 密钥轮转

```yaml
# encryption-config.yaml - 添加新密钥
apiVersion: apiserver.config.k8s.io/v1
kind: EncryptionConfiguration
resources:
  - resources:
      - secrets
    providers:
    # 新密钥(用于写入)
    - aescbc:
        keys:
        - name: key2
          secret: <NEW_BASE64_KEY>
    # 旧密钥(用于读取)
    - aescbc:
        keys:
        - name: key1
          secret: <OLD_BASE64_KEY>
    - identity: {}
```

**轮转步骤**:

```bash
# 1. 添加新密钥到配置文件
# 2. 重启 API Server
# 3. 重新加密所有 Secret(使用新密钥)
kubectl get secrets --all-namespaces -o json | kubectl replace -f -

# 4. 验证所有 Secret 都使用新密钥后,移除旧密钥
# 5. 再次重启 API Server
```

---

## Sealed Secrets

### 1. 什么是 Sealed Secrets

Sealed Secrets 允许将加密的 Secret 存储在 Git 仓库中,只有集群内的控制器才能解密。

**架构**:
```
开发者 --加密--> SealedSecret(Git) --控制器解密--> Secret(集群)
```

### 2. 安装 Sealed Secrets

```bash
# 安装控制器
kubectl apply -f https://github.com/bitnami-labs/sealed-secrets/releases/download/v0.24.0/controller.yaml

# 安装 kubeseal CLI
wget https://github.com/bitnami-labs/sealed-secrets/releases/download/v0.24.0/kubeseal-linux-amd64 -O kubeseal
chmod +x kubeseal
sudo mv kubeseal /usr/local/bin/

# 验证安装
kubectl get pods -n kube-system | grep sealed-secrets
```

### 3. 创建 Sealed Secret

```bash
# 1. 创建普通 Secret(不应用到集群)
kubectl create secret generic db-credentials \
  --from-literal=username=admin \
  --from-literal=password=P@ssw0rd \
  --dry-run=client -o yaml > secret.yaml

# 2. 使用 kubeseal 加密
kubeseal --format yaml < secret.yaml > sealed-secret.yaml

# 3. sealed-secret.yaml 可以安全提交到 Git
cat sealed-secret.yaml
```

**sealed-secret.yaml 示例**:

```yaml
apiVersion: bitnami.com/v1alpha1
kind: SealedSecret
metadata:
  name: db-credentials
  namespace: default
spec:
  encryptedData:
    username: AgCZ8F7j9... (加密后的数据)
    password: AgBx3K9m2...
  template:
    metadata:
      name: db-credentials
    type: Opaque
```

### 4. 应用 Sealed Secret

```bash
# 应用到集群
kubectl apply -f sealed-secret.yaml

# 控制器自动解密并创建 Secret
kubectl get secret db-credentials -n default

# 查看解密后的内容
kubectl get secret db-credentials -o jsonpath='{.data.password}' | base64 -d
# P@ssw0rd
```

### 5. 备份和恢复密钥

```bash
# 备份 Sealed Secrets 控制器的密钥
kubectl get secret -n kube-system sealed-secrets-key -o yaml > sealed-secrets-key-backup.yaml

# 恢复密钥(新集群)
kubectl apply -f sealed-secrets-key-backup.yaml
kubectl delete pod -n kube-system -l name=sealed-secrets-controller
```

---

## External Secrets

### 1. 什么是 External Secrets

External Secrets Operator 从外部密钥管理系统(Vault、AWS Secrets Manager 等)同步 Secret 到 Kubernetes。

**架构**:
```
Vault/AWS --> External Secrets Operator --> Secret(集群)
```

### 2. 安装 External Secrets Operator

```bash
# Helm 安装
helm repo add external-secrets https://charts.external-secrets.io
helm install external-secrets \
  external-secrets/external-secrets \
  -n external-secrets-system \
  --create-namespace

# 验证
kubectl get pods -n external-secrets-system
```

### 3. 配置 AWS Secrets Manager

#### 3.1 创建 SecretStore

```yaml
apiVersion: external-secrets.io/v1beta1
kind: SecretStore
metadata:
  name: aws-secretstore
  namespace: default
spec:
  provider:
    aws:
      service: SecretsManager
      region: us-east-1
      auth:
        secretRef:
          accessKeyIDSecretRef:
            name: aws-credentials
            key: access-key-id
          secretAccessKeySecretRef:
            name: aws-credentials
            key: secret-access-key

---
# AWS 凭证(仅一次性配置)
apiVersion: v1
kind: Secret
metadata:
  name: aws-credentials
  namespace: default
stringData:
  access-key-id: AKIAIOSFODNN7EXAMPLE
  secret-access-key: wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY
```

#### 3.2 创建 ExternalSecret

```yaml
apiVersion: external-secrets.io/v1beta1
kind: ExternalSecret
metadata:
  name: db-credentials
  namespace: default
spec:
  refreshInterval: 1h  # 同步间隔
  secretStoreRef:
    name: aws-secretstore
    kind: SecretStore
  target:
    name: db-credentials  # 生成的 Secret 名称
    creationPolicy: Owner
  data:
  - secretKey: username  # Secret 的 Key
    remoteRef:
      key: prod/database  # AWS Secrets Manager 的路径
      property: username  # JSON 字段
  - secretKey: password
    remoteRef:
      key: prod/database
      property: password
```

**应用后自动生成 Secret**:

```bash
kubectl apply -f external-secret.yaml

# 查看生成的 Secret
kubectl get secret db-credentials -n default

# External Secret 状态
kubectl get externalsecret db-credentials -n default
# NAME             STORE             REFRESH INTERVAL   STATUS
# db-credentials   aws-secretstore   1h                 SecretSynced
```

### 4. 配置 HashiCorp Vault

#### 4.1 创建 SecretStore

```yaml
apiVersion: external-secrets.io/v1beta1
kind: SecretStore
metadata:
  name: vault-secretstore
  namespace: default
spec:
  provider:
    vault:
      server: "https://vault.example.com"
      path: "secret"  # KV v2 路径
      version: "v2"
      auth:
        # Kubernetes 认证
        kubernetes:
          mountPath: "kubernetes"
          role: "my-app-role"
          serviceAccountRef:
            name: vault-auth
```

#### 4.2 创建 ExternalSecret

```yaml
apiVersion: external-secrets.io/v1beta1
kind: ExternalSecret
metadata:
  name: app-credentials
  namespace: default
spec:
  refreshInterval: 5m
  secretStoreRef:
    name: vault-secretstore
    kind: SecretStore
  target:
    name: app-credentials
  data:
  - secretKey: api-key
    remoteRef:
      key: myapp/config  # Vault 路径: secret/myapp/config
      property: api_key
```

---

## 实战案例

### 案例 1: 数据库凭证管理

```yaml
# 1. 创建 Secret
apiVersion: v1
kind: Secret
metadata:
  name: mysql-credentials
  namespace: production
stringData:
  MYSQL_ROOT_PASSWORD: RootP@ssw0rd
  MYSQL_USER: appuser
  MYSQL_PASSWORD: AppP@ssw0rd
  MYSQL_DATABASE: myapp

---
# 2. MySQL Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: mysql
  namespace: production
spec:
  replicas: 1
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
        envFrom:
        - secretRef:
            name: mysql-credentials
        ports:
        - containerPort: 3306
        volumeMounts:
        - name: data
          mountPath: /var/lib/mysql
      volumes:
      - name: data
        persistentVolumeClaim:
          claimName: mysql-pvc

---
# 3. 应用 Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: myapp
  namespace: production
spec:
  replicas: 3
  selector:
    matchLabels:
      app: myapp
  template:
    metadata:
      labels:
        app: myapp
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
              name: mysql-credentials
              key: MYSQL_USER
        - name: DB_PASSWORD
          valueFrom:
            secretKeyRef:
              name: mysql-credentials
              key: MYSQL_PASSWORD
        - name: DB_NAME
          valueFrom:
            secretKeyRef:
              name: mysql-credentials
              key: MYSQL_DATABASE
```

### 案例 2: TLS 证书管理

```bash
# 1. 生成自签名证书
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout tls.key -out tls.crt \
  -subj "/CN=myapp.example.com/O=MyOrg"

# 2. 创建 Secret
kubectl create secret tls myapp-tls \
  --cert=tls.crt \
  --key=tls.key \
  -n production

# 3. Ingress 使用证书
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: myapp-ingress
  namespace: production
spec:
  tls:
  - hosts:
    - myapp.example.com
    secretName: myapp-tls  # 引用 TLS Secret
  rules:
  - host: myapp.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: myapp
            port:
              number: 80
```

### 案例 3: GitOps 与 Sealed Secrets

```bash
# Git 仓库结构
# k8s-manifests/
# ├── base/
# │   ├── deployment.yaml
# │   └── sealed-secret.yaml  # 加密的 Secret
# └── kustomization.yaml

# 1. 创建 Sealed Secret
kubectl create secret generic app-credentials \
  --from-literal=api-key=secret-api-key-12345 \
  --dry-run=client -o yaml | \
  kubeseal --format yaml > k8s-manifests/base/sealed-secret.yaml

# 2. 提交到 Git
git add k8s-manifests/base/sealed-secret.yaml
git commit -m "Add encrypted secrets"
git push

# 3. ArgoCD/Flux 自动同步
# SealedSecret 应用到集群后自动解密为 Secret
```

---

## 最佳实践

### 1. 永远不要将明文 Secret 提交到 Git

```bash
# ❌ 错误: 明文 Secret
apiVersion: v1
kind: Secret
metadata:
  name: db-password
stringData:
  password: MySecretPassword  # 不要这样做!

# ✅ 正确: 使用 Sealed Secrets 或 External Secrets
# 或者将 Secret 从 Git 仓库中排除
echo "secret.yaml" >> .gitignore
```

### 2. 使用 RBAC 限制 Secret 访问

```yaml
# 只允许特定 ServiceAccount 读取 Secret
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: secret-reader
  namespace: production
rules:
- apiGroups: [""]
  resources: ["secrets"]
  resourceNames: ["db-credentials"]  # 限定特定 Secret
  verbs: ["get"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: app-secret-reader
  namespace: production
subjects:
- kind: ServiceAccount
  name: myapp-sa
  namespace: production
roleRef:
  kind: Role
  name: secret-reader
  apiGroup: rbac.authorization.k8s.io
```

### 3. 启用 etcd 静态加密

```bash
# 生产环境必须启用
# 防止直接访问 etcd 读取明文 Secret
```

### 4. 使用 immutable Secret

```yaml
# Kubernetes 1.21+
apiVersion: v1
kind: Secret
metadata:
  name: immutable-secret
immutable: true  # 创建后不可修改
data:
  key: dmFsdWU=
```

**好处**:
- 防止意外修改
- 提升性能(kubelet 不需要监听变化)
- 若需要更新,删除后重新创建

### 5. 定期轮转密钥

```bash
# 使用 External Secrets 自动同步
# 在外部系统(Vault)中轮转密钥,自动同步到集群

# 或使用 CronJob 定期更新
apiVersion: batch/v1
kind: CronJob
metadata:
  name: rotate-secrets
spec:
  schedule: "0 0 1 * *"  # 每月1号
  jobTemplate:
    spec:
      template:
        spec:
          containers:
          - name: rotate
            image: myrotate:1.0
            command: ["./rotate-secrets.sh"]
```

---

## 常见问题

### Q1: Secret 是如何加密的?

**A**:

| 阶段 | 加密方式 | 说明 |
|-----|---------|------|
| **传输中** | TLS | API Server 通信加密 |
| **静态存储** | 可选加密 | 需要配置 Encryption at Rest |
| **内存中** | tmpfs | Pod 挂载 Secret 使用内存文件系统 |

**默认情况**:
- Secret 在 etcd 中仅 Base64 编码,**不加密**
- 需要手动启用 Encryption at Rest

### Q2: Base64 编码和加密有什么区别?

**A**:

```bash
# Base64 编码(可逆,非加密)
echo -n 'password' | base64
# cGFzc3dvcmQ=

echo 'cGFzc3dvcmQ=' | base64 -d
# password

# 加密(不可逆,需要密钥)
# AES-256 加密后输出乱码,没有密钥无法解密
```

### Q3: 如何审计 Secret 访问?

**A**:

```yaml
# 启用 API Server 审计
# /etc/kubernetes/audit-policy.yaml
apiVersion: audit.k8s.io/v1
kind: Policy
rules:
# 记录 Secret 访问
- level: Metadata
  resources:
  - group: ""
    resources: ["secrets"]
  verbs: ["get", "list", "watch"]

# 记录 Secret 修改
- level: RequestResponse
  resources:
  - group: ""
    resources: ["secrets"]
  verbs: ["create", "update", "patch", "delete"]
```

**查看审计日志**:

```bash
# 查看 API Server 日志
kubectl logs -n kube-system kube-apiserver-master | grep secret

# 或查看审计日志文件
cat /var/log/kubernetes/audit.log | grep secret
```

### Q4: Sealed Secrets 和 External Secrets 如何选择?

**A**:

| 场景 | Sealed Secrets | External Secrets |
|-----|---------------|------------------|
| **GitOps 存储** | ✅ 适合 | ❌ 不适合(存储引用) |
| **多集群同步** | ❌ 需要导出密钥 | ✅ 中心化管理 |
| **密钥轮转** | ❌ 手动 | ✅ 自动同步 |
| **审计** | ❌ 集群内 | ✅ Vault 审计日志 |
| **复杂度** | 低 | 中等 |

**推荐**:
- 小型项目、GitOps: **Sealed Secrets**
- 大型企业、多集群: **External Secrets + Vault**

### Q5: 如何从 Secret 迁移到 External Secrets?

**A**:

```bash
# 1. 将现有 Secret 导入 Vault
kubectl get secret db-credentials -o json | \
  jq -r '.data | to_entries[] | "\(.key)=\(.value | @base64d)"' | \
  while IFS='=' read -r key value; do
    vault kv put secret/myapp/db "$key=$value"
  done

# 2. 创建 ExternalSecret
apiVersion: external-secrets.io/v1beta1
kind: ExternalSecret
metadata:
  name: db-credentials
spec:
  secretStoreRef:
    name: vault-secretstore
  target:
    name: db-credentials
  data:
  - secretKey: username
    remoteRef:
      key: myapp/db
      property: username
  - secretKey: password
    remoteRef:
      key: myapp/db
      property: password

# 3. 应用 ExternalSecret
kubectl apply -f external-secret.yaml

# 4. 验证生成的 Secret
kubectl get secret db-credentials -o yaml

# 5. 删除旧 Secret(谨慎操作)
kubectl delete secret db-credentials-old
```

---

## 总结

- ✅ **Secret**: 存储敏感信息,默认 Base64 编码(非加密)
- ✅ **Encryption at Rest**: 生产环境必须启用 etcd 加密
- ✅ **Sealed Secrets**: GitOps 场景下安全存储加密 Secret
- ✅ **External Secrets**: 集成外部密钥管理系统(Vault、AWS)
- ✅ **RBAC**: 限制 Secret 访问权限
- ✅ **审计**: 记录 Secret 访问和修改
- ✅ **密钥轮转**: 定期更新密钥,降低泄露风险

## 参考资源

- [Secrets 官方文档](https://kubernetes.io/docs/concepts/configuration/secret/)
- [Encrypting Secret Data at Rest](https://kubernetes.io/docs/tasks/administer-cluster/encrypt-data/)
- [Sealed Secrets](https://github.com/bitnami-labs/sealed-secrets)
- [External Secrets Operator](https://external-secrets.io/)
- [HashiCorp Vault](https://www.vaultproject.io/docs/platform/k8s)
- [AWS Secrets Manager](https://docs.aws.amazon.com/secretsmanager/)

---

*更新日期: 2025-12-03*
