# ServiceAccount

> 服务账号、Token、权限管理

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**ServiceAccount(服务账号)是 Kubernetes 为 Pod 提供身份认证的机制**。通过 ServiceAccount 和 Token,Pod 可以安全地访问 Kubernetes API,实现自动化管理和服务间认证,同时通过 RBAC 精细控制 Pod 的权限范围。

## 核心概念

### 什么是 ServiceAccount

ServiceAccount 是 Kubernetes 为 Pod 内部进程提供的身份标识:

| 组件 | 说明 | 作用 |
|-----|------|------|
| **ServiceAccount** | 服务账号 | 为 Pod 提供身份 |
| **Token** | JWT 令牌 | 认证凭证 |
| **Secret** | 存储 Token | 自动创建(v1.24+可选) |
| **RBAC** | 权限绑定 | 控制访问权限 |

### ServiceAccount vs User Account

| 特性 | ServiceAccount | User Account |
|-----|---------------|--------------|
| **使用者** | Pod、应用程序 | 人类用户 |
| **作用域** | Namespace 级别 | 集群级别 |
| **管理方式** | kubectl 创建 | 外部系统管理(LDAP、OIDC) |
| **Token** | 自动生成/挂载 | 手动配置 |
| **生命周期** | 与 Namespace 绑定 | 独立管理 |

### 为什么需要 ServiceAccount

| 场景 | 问题 | ServiceAccount 解决方案 |
|-----|------|---------------------|
| **访问 API Server** | Pod 需要操作 K8s 资源 | 提供认证凭证 |
| **权限隔离** | 不同 Pod 需要不同权限 | 每个应用独立 SA |
| **审计追踪** | 无法追踪 API 调用来源 | SA 标识每个 Pod |
| **自动化管理** | 手动配置凭证繁琐 | 自动挂载 Token |

---

## ServiceAccount 基础

### 1. 查看默认 ServiceAccount

每个 Namespace 都有一个 `default` ServiceAccount:

```bash
# 查看 ServiceAccount
kubectl get serviceaccounts -n default
# NAME      SECRETS   AGE
# default   0         10d

# 详细信息
kubectl describe serviceaccount default -n default

# 查看所有命名空间的 ServiceAccount
kubectl get sa --all-namespaces
```

### 2. 创建 ServiceAccount

#### 2.1 YAML 方式

```yaml
apiVersion: v1
kind: ServiceAccount
metadata:
  name: myapp-sa
  namespace: default
# 禁用自动挂载 Token(可选)
automountServiceAccountToken: false
```

#### 2.2 kubectl 命令

```bash
# 创建 ServiceAccount
kubectl create serviceaccount myapp-sa -n default

# 查看
kubectl get sa myapp-sa -n default

# 详细信息
kubectl describe sa myapp-sa -n default
```

### 3. Pod 使用 ServiceAccount

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: myapp
  namespace: default
spec:
  serviceAccountName: myapp-sa  # 指定 ServiceAccount
  containers:
  - name: app
    image: nginx:1.21
```

**默认行为**:
- 如果不指定 `serviceAccountName`,使用 `default` SA
- Token 自动挂载到 `/var/run/secrets/kubernetes.io/serviceaccount/`
- 包含 `ca.crt`、`namespace`、`token` 三个文件

---

## Token 管理

### Kubernetes 1.24+ Token 变化

| 版本 | Token 类型 | 说明 |
|-----|----------|------|
| **< 1.24** | Long-lived Token | 永久有效,自动创建 Secret |
| **>= 1.24** | Time-bound Token | 有过期时间,按需创建 |

### 1. 创建短期 Token

```bash
# 创建 1 小时有效的 Token
kubectl create token myapp-sa -n default --duration=1h

# 创建 24 小时有效的 Token
kubectl create token myapp-sa -n default --duration=24h

# 保存 Token 到变量
TOKEN=$(kubectl create token myapp-sa -n default)
echo $TOKEN
```

### 2. 创建长期 Token (手动 Secret)

```yaml
# 手动创建 Secret 存储 Token
apiVersion: v1
kind: Secret
metadata:
  name: myapp-sa-token
  namespace: default
  annotations:
    kubernetes.io/service-account.name: myapp-sa
type: kubernetes.io/service-account-token
```

**应用后获取 Token**:

```bash
# 获取 Token
kubectl get secret myapp-sa-token -n default -o jsonpath='{.data.token}' | base64 -d

# 获取 CA 证书
kubectl get secret myapp-sa-token -n default -o jsonpath='{.data.ca\.crt}' | base64 -d > ca.crt
```

### 3. Token 自动挂载

**Pod 内部自动挂载的文件**:

```bash
# 进入 Pod
kubectl exec -it myapp -- sh

# 查看挂载的文件
ls -la /var/run/secrets/kubernetes.io/serviceaccount/
# ca.crt      - 集群 CA 证书
# namespace   - Pod 所在命名空间
# token       - JWT Token

# 查看 Token
cat /var/run/secrets/kubernetes.io/serviceaccount/token

# 查看命名空间
cat /var/run/secrets/kubernetes.io/serviceaccount/namespace
# default
```

### 4. 禁用自动挂载

#### 4.1 ServiceAccount 级别

```yaml
apiVersion: v1
kind: ServiceAccount
metadata:
  name: no-token-sa
  namespace: default
automountServiceAccountToken: false  # 禁用
```

#### 4.2 Pod 级别

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: myapp
spec:
  serviceAccountName: myapp-sa
  automountServiceAccountToken: false  # Pod 级别覆盖
  containers:
  - name: app
    image: nginx:1.21
```

---

## ServiceAccount 与 RBAC

### 1. 授予权限

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: pod-reader-sa
  namespace: default

---
# 2. 创建 Role
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: pod-reader
  namespace: default
rules:
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list", "watch"]

---
# 3. 创建 RoleBinding
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: pod-reader-binding
  namespace: default
subjects:
- kind: ServiceAccount
  name: pod-reader-sa
  namespace: default
roleRef:
  kind: Role
  name: pod-reader
  apiGroup: rbac.authorization.k8s.io

---
# 4. Pod 使用 ServiceAccount
apiVersion: v1
kind: Pod
metadata:
  name: pod-reader
  namespace: default
spec:
  serviceAccountName: pod-reader-sa
  containers:
  - name: app
    image: curlimages/curl:latest
    command: ["sh", "-c", "sleep 3600"]
```

### 2. 测试权限

```bash
# 进入 Pod
kubectl exec -it pod-reader -- sh

# 获取 Token
TOKEN=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)

# 访问 API Server
curl -k -H "Authorization: Bearer $TOKEN" \
  https://kubernetes.default.svc/api/v1/namespaces/default/pods

# 成功: 返回 Pod 列表

# 尝试无权限的操作
curl -k -H "Authorization: Bearer $TOKEN" \
  -X DELETE \
  https://kubernetes.default.svc/api/v1/namespaces/default/pods/pod-reader

# 失败: Forbidden
```

---

## 实战案例

### 案例 1: 监控应用权限

**需求**: Prometheus 需要读取 Pod、Node、Service 的指标。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: prometheus-sa
  namespace: monitoring

---
# 2. 创建 ClusterRole(需要跨 Namespace)
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: prometheus-reader
rules:
# 读取 Pods
- apiGroups: [""]
  resources: ["pods", "nodes", "services", "endpoints"]
  verbs: ["get", "list", "watch"]

# 读取 Metrics
- apiGroups: [""]
  resources: ["nodes/metrics", "nodes/stats"]
  verbs: ["get"]

---
# 3. 创建 ClusterRoleBinding
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: prometheus-reader-binding
subjects:
- kind: ServiceAccount
  name: prometheus-sa
  namespace: monitoring
roleRef:
  kind: ClusterRole
  name: prometheus-reader
  apiGroup: rbac.authorization.k8s.io

---
# 4. Prometheus Deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: prometheus
  namespace: monitoring
spec:
  replicas: 1
  selector:
    matchLabels:
      app: prometheus
  template:
    metadata:
      labels:
        app: prometheus
    spec:
      serviceAccountName: prometheus-sa
      containers:
      - name: prometheus
        image: prom/prometheus:latest
        ports:
        - containerPort: 9090
        volumeMounts:
        - name: config
          mountPath: /etc/prometheus
      volumes:
      - name: config
        configMap:
          name: prometheus-config
```

### 案例 2: CI/CD 部署权限

**需求**: GitLab Runner 需要在 Kubernetes 中部署应用。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: gitlab-runner-sa
  namespace: gitlab

---
# 2. 创建 Role
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: deployer
  namespace: production
rules:
# Deployments
- apiGroups: ["apps"]
  resources: ["deployments", "replicasets"]
  verbs: ["get", "list", "watch", "create", "update", "patch", "delete"]

# Pods
- apiGroups: [""]
  resources: ["pods", "pods/log"]
  verbs: ["get", "list", "watch"]

# ConfigMaps/Secrets
- apiGroups: [""]
  resources: ["configmaps", "secrets"]
  verbs: ["get", "list", "create", "update", "patch"]

# Services
- apiGroups: [""]
  resources: ["services"]
  verbs: ["get", "list", "create", "update", "patch"]

---
# 3. 创建 RoleBinding
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: gitlab-runner-deployer
  namespace: production
subjects:
- kind: ServiceAccount
  name: gitlab-runner-sa
  namespace: gitlab
roleRef:
  kind: Role
  name: deployer
  apiGroup: rbac.authorization.k8s.io
```

**在 GitLab CI 中使用**:

```yaml
# .gitlab-ci.yml
deploy:
  stage: deploy
  image: bitnami/kubectl:latest
  script:
    # 获取 Token
    - TOKEN=$(kubectl create token gitlab-runner-sa -n gitlab --duration=1h)
    # 配置 kubectl
    - kubectl config set-credentials gitlab-runner --token=$TOKEN
    - kubectl config set-context gitlab --cluster=production --user=gitlab-runner
    - kubectl config use-context gitlab
    # 部署
    - kubectl apply -f deployment.yaml -n production
    - kubectl rollout status deployment/myapp -n production
```

### 案例 3: 外部应用访问集群

**需求**: 集群外部的应用需要访问 Kubernetes API。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: external-app-sa
  namespace: default

---
# 2. 创建 Secret(长期 Token)
apiVersion: v1
kind: Secret
metadata:
  name: external-app-token
  namespace: default
  annotations:
    kubernetes.io/service-account.name: external-app-sa
type: kubernetes.io/service-account-token

---
# 3. 授予权限
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: external-app-role
rules:
- apiGroups: [""]
  resources: ["pods", "services"]
  verbs: ["get", "list", "watch"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: external-app-binding
subjects:
- kind: ServiceAccount
  name: external-app-sa
  namespace: default
roleRef:
  kind: ClusterRole
  name: external-app-role
  apiGroup: rbac.authorization.k8s.io
```

**获取 Token 并配置 kubeconfig**:

```bash
# 1. 获取 Token
TOKEN=$(kubectl get secret external-app-token -n default -o jsonpath='{.data.token}' | base64 -d)

# 2. 获取 CA 证书
kubectl get secret external-app-token -n default -o jsonpath='{.data.ca\.crt}' | base64 -d > ca.crt

# 3. 获取 API Server 地址
API_SERVER=$(kubectl config view --minify -o jsonpath='{.clusters[0].cluster.server}')

# 4. 创建 kubeconfig
kubectl config set-cluster my-cluster \
  --server=$API_SERVER \
  --certificate-authority=ca.crt \
  --embed-certs=true

kubectl config set-credentials external-app \
  --token=$TOKEN

kubectl config set-context external-app-context \
  --cluster=my-cluster \
  --user=external-app \
  --namespace=default

kubectl config use-context external-app-context

# 5. 测试
kubectl get pods
```

### 案例 4: 动态 Token 轮转

**需求**: 应用定期刷新 Token 以提高安全性。

```go
// Go 示例: 动态获取 Token
package main

import (
    "context"
    "fmt"
    "os"
    "time"

    metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
    "k8s.io/client-go/kubernetes"
    "k8s.io/client-go/rest"
)

func main() {
    // 使用 in-cluster 配置
    config, err := rest.InClusterConfig()
    if err != nil {
        panic(err)
    }

    clientset, err := kubernetes.NewForConfig(config)
    if err != nil {
        panic(err)
    }

    // 定期刷新 Token
    ticker := time.NewTicker(1 * time.Hour)
    defer ticker.Stop()

    for {
        select {
        case <-ticker.C:
            // 获取当前 ServiceAccount
            namespace := os.Getenv("POD_NAMESPACE")
            sa := os.Getenv("SERVICE_ACCOUNT")

            // 创建新 Token
            treq := &authv1.TokenRequest{
                Spec: authv1.TokenRequestSpec{
                    ExpirationSeconds: ptr.Int64(3600), // 1 小时
                },
            }

            token, err := clientset.CoreV1().ServiceAccounts(namespace).
                CreateToken(context.TODO(), sa, treq, metav1.CreateOptions{})

            if err != nil {
                fmt.Printf("Failed to create token: %v\n", err)
                continue
            }

            // 更新配置
            config.BearerToken = token.Status.Token
            fmt.Println("Token refreshed")
        }
    }
}
```

---

## 最佳实践

### 1. 每个应用使用独立 ServiceAccount

```yaml
# ✅ 推荐: 每个应用独立 SA
apiVersion: v1
kind: ServiceAccount
metadata:
  name: app1-sa
  namespace: production
---
apiVersion: v1
kind: ServiceAccount
metadata:
  name: app2-sa
  namespace: production
---
apiVersion: v1
kind: Pod
metadata:
  name: app1
spec:
  serviceAccountName: app1-sa
  containers:
  - name: app
    image: app1:1.0

# ❌ 不推荐: 多个应用共享 default SA
apiVersion: v1
kind: Pod
metadata:
  name: app1
spec:
  # 使用 default SA(不安全)
  containers:
  - name: app
    image: app1:1.0
```

### 2. 禁用不需要访问 API 的 Pod 的 Token 挂载

```yaml
# ✅ 推荐: 不需要 API 访问时禁用 Token
apiVersion: v1
kind: Pod
metadata:
  name: nginx
spec:
  automountServiceAccountToken: false  # 禁用
  containers:
  - name: nginx
    image: nginx:1.21

# ❌ 不推荐: 所有 Pod 都挂载 Token
apiVersion: v1
kind: Pod
metadata:
  name: nginx
spec:
  # 默认挂载 Token(不必要)
  containers:
  - name: nginx
    image: nginx:1.21
```

### 3. 使用短期 Token

```bash
# ✅ 推荐: 使用短期 Token
kubectl create token myapp-sa --duration=1h

# ❌ 不推荐: 使用长期 Token(v1.24+)
# 长期 Token 泄露风险更高
```

### 4. 最小权限原则

```yaml
# ✅ 推荐: 只授予必要权限
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: read-pods
rules:
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list"]

# ❌ 不推荐: 授予过多权限
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: admin-all
rules:
- apiGroups: ["*"]
  resources: ["*"]
  verbs: ["*"]
```

### 5. 定期审计 ServiceAccount

```bash
# 查找没有使用的 ServiceAccount
kubectl get sa --all-namespaces -o json | \
  jq -r '.items[] | select(.metadata.name != "default") |
  "\(.metadata.namespace)/\(.metadata.name)"' | \
  while read sa; do
    namespace=$(echo $sa | cut -d'/' -f1)
    name=$(echo $sa | cut -d'/' -f2)
    count=$(kubectl get pods -n $namespace -o json | \
      jq -r ".items[] | select(.spec.serviceAccountName==\"$name\") | .metadata.name" | wc -l)
    if [ $count -eq 0 ]; then
      echo "Unused: $sa"
    fi
  done

# 查找有过多权限的 ServiceAccount
kubectl get clusterrolebindings -o json | \
  jq -r '.items[] | select(.roleRef.name=="cluster-admin") |
  {name: .metadata.name, subjects: .subjects}'
```

---

## 常见问题

### Q1: 如何查看 ServiceAccount 的权限?

**A**:

```bash
# 方法 1: 使用 auth can-i
kubectl auth can-i --list \
  --as=system:serviceaccount:default:myapp-sa

# 方法 2: 查找 RoleBinding
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.subjects[]?.name=="myapp-sa") |
  {namespace: .metadata.namespace, role: .roleRef.name}'

# 方法 3: 使用插件
kubectl krew install who-can
kubectl who-can get pods --as=system:serviceaccount:default:myapp-sa
```

### Q2: Token 有效期是多久?

**A**:

| Kubernetes 版本 | Token 类型 | 有效期 |
|----------------|----------|-------|
| **< 1.24** | Secret Token | 永久 |
| **>= 1.24** | Projected Token | 默认 1 小时,最长 1 年 |
| **kubectl create token** | 临时 Token | 默认 1 小时,可自定义 |

```bash
# 查看 Token 有效期
TOKEN=$(kubectl create token myapp-sa)
echo $TOKEN | cut -d'.' -f2 | base64 -d 2>/dev/null | jq '.exp'
# 输出: 1699999999(Unix 时间戳)

# 转换为可读时间
date -d @1699999999
```

### Q3: 如何在集群外使用 ServiceAccount?

**A**:

```bash
# 1. 创建 ServiceAccount 和权限(见案例 3)

# 2. 获取 Token 和 CA
TOKEN=$(kubectl get secret myapp-token -o jsonpath='{.data.token}' | base64 -d)
kubectl get secret myapp-token -o jsonpath='{.data.ca\.crt}' | base64 -d > ca.crt

# 3. 配置 kubeconfig
API_SERVER=https://kubernetes.example.com:6443

kubectl config set-cluster my-cluster \
  --server=$API_SERVER \
  --certificate-authority=ca.crt \
  --embed-certs=true

kubectl config set-credentials myapp \
  --token=$TOKEN

kubectl config set-context myapp-context \
  --cluster=my-cluster \
  --user=myapp

kubectl config use-context myapp-context

# 4. 测试
kubectl get pods
```

### Q4: 如何解决 "error: You must be logged in to the server (Unauthorized)"?

**A**:

```bash
# 1. 检查 Token 是否过期
TOKEN=$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)
echo $TOKEN | cut -d'.' -f2 | base64 -d 2>/dev/null | jq '.'

# 2. 检查 ServiceAccount 是否存在
kubectl get sa myapp-sa -n default

# 3. 检查 RoleBinding
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.subjects[]?.name=="myapp-sa")'

# 4. 检查 API Server 连接
curl -k https://kubernetes.default.svc/api/v1

# 5. 重新创建 Token
kubectl create token myapp-sa -n default
```

### Q5: 如何迁移到 Kubernetes 1.24+ 的 Token 机制?

**A**:

**变化**:
- 不再自动创建 Secret 存储 Token
- Token 有过期时间(默认 1 小时)
- 需要手动创建 Secret 或使用 Projected Token

**迁移步骤**:

```bash
# 1. 检查依赖长期 Token 的应用
kubectl get secrets --all-namespaces -o json | \
  jq -r '.items[] | select(.type=="kubernetes.io/service-account-token") |
  "\(.metadata.namespace)/\(.metadata.name)"'

# 2. 方案 A: 创建长期 Token Secret
kubectl apply -f - <<EOF
apiVersion: v1
kind: Secret
metadata:
  name: myapp-token
  namespace: default
  annotations:
    kubernetes.io/service-account.name: myapp-sa
type: kubernetes.io/service-account-token
EOF

# 3. 方案 B: 使用 Projected Token(推荐)
apiVersion: v1
kind: Pod
metadata:
  name: myapp
spec:
  serviceAccountName: myapp-sa
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: token
      mountPath: /var/run/secrets/tokens
  volumes:
  - name: token
    projected:
      sources:
      - serviceAccountToken:
          path: token
          expirationSeconds: 3600  # 1 小时
          audience: api

# 4. 方案 C: 应用内动态刷新 Token
# 使用 client-go 的 TokenRequest API
```

---

## 总结

- ✅ **ServiceAccount**: 为 Pod 提供身份标识和 API 访问凭证
- ✅ **Token**: 认证凭证,Kubernetes 1.24+ 推荐使用短期 Token
- ✅ **自动挂载**: Token 自动挂载到 `/var/run/secrets/kubernetes.io/serviceaccount/`
- ✅ **RBAC 集成**: 通过 RoleBinding 授予 ServiceAccount 权限
- ✅ **最小权限**: 每个应用使用独立 SA,只授予必要权限
- ✅ **禁用挂载**: 不需要 API 访问的 Pod 禁用 Token 挂载
- ✅ **定期审计**: 定期检查和清理未使用的 ServiceAccount

## 参考资源

- [ServiceAccount 官方文档](https://kubernetes.io/docs/concepts/security/service-accounts/)
- [Configure Service Accounts](https://kubernetes.io/docs/tasks/configure-pod-container/configure-service-account/)
- [TokenRequest API](https://kubernetes.io/docs/reference/kubernetes-api/authentication-resources/token-request-v1/)
- [ServiceAccount Token Secrets](https://kubernetes.io/docs/concepts/configuration/secret/#serviceaccount-token-secrets)
- [Kubernetes 1.24 Token 变化](https://kubernetes.io/blog/2022/05/03/kubernetes-1-24-release-announcement/)

---

*更新日期: 2025-12-03*
