# RBAC

> Role、ClusterRole、RoleBinding、授权

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**RBAC(基于角色的访问控制)是 Kubernetes 集群安全的核心机制**。通过 Role、ClusterRole、RoleBinding 和 ClusterRoleBinding,可以精细控制用户、服务账号对集群资源的访问权限,实现最小权限原则,防止权限滥用和越权操作。

## 核心概念

### 什么是 RBAC

RBAC(Role-Based Access Control)是 Kubernetes 的授权机制,涉及四个核心对象:

| 对象类型 | 作用域 | 说明 |
|---------|-------|------|
| **Role** | Namespace 级别 | 定义命名空间内的权限规则 |
| **ClusterRole** | 集群级别 | 定义集群范围的权限规则 |
| **RoleBinding** | Namespace 级别 | 绑定 Role 到用户/组/SA |
| **ClusterRoleBinding** | 集群级别 | 绑定 ClusterRole 到用户/组/SA |

### RBAC 授权流程

```
用户/ServiceAccount 发起请求
         ↓
API Server 接收请求
         ↓
认证(Authentication) - 确认身份
         ↓
授权(Authorization) - RBAC 检查
         ↓
准入控制(Admission Control)
         ↓
执行操作
```

### 为什么需要 RBAC

| 安全风险 | 后果 | RBAC 防护 |
|---------|------|----------|
| **过度授权** | 用户可以删除关键资源 | 最小权限原则 |
| **ServiceAccount 滥用** | Pod 可以操作其他 Namespace | Namespace 隔离 |
| **集群管理员过多** | 误操作导致集群瘫痪 | 细粒度权限控制 |
| **审计困难** | 无法追踪谁做了什么 | RoleBinding 绑定记录 |

---

## Role 与 RoleBinding

### 1. Role - 定义权限

#### 1.1 基础 Role

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: pod-reader
  namespace: default
rules:
# 规则 1: 读取 Pods
- apiGroups: [""]  # "" 表示 core API group
  resources: ["pods"]
  verbs: ["get", "list", "watch"]

# 规则 2: 读取 Pod 日志
- apiGroups: [""]
  resources: ["pods/log"]
  verbs: ["get"]
```

**verbs 动词说明**:

| Verb | 说明 | HTTP 方法 |
|------|------|----------|
| **get** | 获取单个资源 | GET |
| **list** | 列出资源集合 | GET |
| **watch** | 监听资源变化 | GET(WebSocket) |
| **create** | 创建资源 | POST |
| **update** | 更新资源 | PUT |
| **patch** | 部分更新资源 | PATCH |
| **delete** | 删除单个资源 | DELETE |
| **deletecollection** | 批量删除资源 | DELETE |

#### 1.2 多种资源的 Role

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: app-manager
  namespace: production
rules:
# Pods 全操作
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list", "watch", "create", "update", "patch", "delete"]

# ConfigMaps 和 Secrets 只读
- apiGroups: [""]
  resources: ["configmaps", "secrets"]
  verbs: ["get", "list"]

# Deployments 全操作
- apiGroups: ["apps"]
  resources: ["deployments", "replicasets"]
  verbs: ["get", "list", "watch", "create", "update", "patch", "delete"]

# Services 全操作
- apiGroups: [""]
  resources: ["services"]
  verbs: ["get", "list", "watch", "create", "update", "patch", "delete"]
```

#### 1.3 指定资源名称

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: configmap-updater
  namespace: default
rules:
- apiGroups: [""]
  resources: ["configmaps"]
  # 只能操作名为 app-config 的 ConfigMap
  resourceNames: ["app-config"]
  verbs: ["get", "update", "patch"]
```

### 2. RoleBinding - 绑定权限

#### 2.1 绑定到 User

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: read-pods
  namespace: default
subjects:
# 用户
- kind: User
  name: jane
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: Role
  name: pod-reader
  apiGroup: rbac.authorization.k8s.io
```

#### 2.2 绑定到 ServiceAccount

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: app-manager-binding
  namespace: production
subjects:
# ServiceAccount
- kind: ServiceAccount
  name: app-sa
  namespace: production
roleRef:
  kind: Role
  name: app-manager
  apiGroup: rbac.authorization.k8s.io
```

#### 2.3 绑定到 Group

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: dev-team-binding
  namespace: development
subjects:
# 组
- kind: Group
  name: dev-team
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: Role
  name: app-manager
  apiGroup: rbac.authorization.k8s.io
```

### 3. kubectl 命令

```bash
# 创建 Role
kubectl create role pod-reader \
  --verb=get,list,watch \
  --resource=pods \
  -n default

# 创建 RoleBinding
kubectl create rolebinding read-pods \
  --role=pod-reader \
  --user=jane \
  -n default

# 查看 Role
kubectl get roles -n default
kubectl describe role pod-reader -n default

# 查看 RoleBinding
kubectl get rolebindings -n default
kubectl describe rolebinding read-pods -n default

# 删除
kubectl delete role pod-reader -n default
kubectl delete rolebinding read-pods -n default
```

---

## ClusterRole 与 ClusterRoleBinding

### 1. ClusterRole - 集群级别权限

#### 1.1 集群资源访问

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: node-reader
rules:
# 读取节点信息
- apiGroups: [""]
  resources: ["nodes"]
  verbs: ["get", "list", "watch"]

# 读取 PersistentVolumes
- apiGroups: [""]
  resources: ["persistentvolumes"]
  verbs: ["get", "list", "watch"]

# 读取命名空间
- apiGroups: [""]
  resources: ["namespaces"]
  verbs: ["get", "list"]
```

#### 1.2 跨 Namespace 资源访问

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: pod-reader-all-namespaces
rules:
# 可以读取所有命名空间的 Pods
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list", "watch"]
```

#### 1.3 聚合 ClusterRole

```yaml
# 基础 ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: monitoring-base
  labels:
    rbac.example.com/aggregate-to-monitoring: "true"
rules:
- apiGroups: [""]
  resources: ["pods", "services"]
  verbs: ["get", "list", "watch"]

---
# 扩展 ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: monitoring-extended
  labels:
    rbac.example.com/aggregate-to-monitoring: "true"
rules:
- apiGroups: ["apps"]
  resources: ["deployments", "statefulsets"]
  verbs: ["get", "list", "watch"]

---
# 聚合 ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: monitoring
aggregationRule:
  clusterRoleSelectors:
  - matchLabels:
      rbac.example.com/aggregate-to-monitoring: "true"
rules: []  # 自动填充
```

### 2. ClusterRoleBinding - 集群级别绑定

```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: node-reader-binding
subjects:
- kind: User
  name: ops-user
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: ClusterRole
  name: node-reader
  apiGroup: rbac.authorization.k8s.io
```

### 3. ClusterRole + RoleBinding

```yaml
# ClusterRole 定义权限(可复用)
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: secret-reader
rules:
- apiGroups: [""]
  resources: ["secrets"]
  verbs: ["get", "list"]

---
# RoleBinding 限定在某个 Namespace
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: secret-reader-binding
  namespace: production  # 只在 production 命名空间生效
subjects:
- kind: User
  name: bob
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: ClusterRole  # 引用 ClusterRole
  name: secret-reader
  apiGroup: rbac.authorization.k8s.io
```

### 4. kubectl 命令

```bash
# 创建 ClusterRole
kubectl create clusterrole node-reader \
  --verb=get,list,watch \
  --resource=nodes

# 创建 ClusterRoleBinding
kubectl create clusterrolebinding node-reader-binding \
  --clusterrole=node-reader \
  --user=ops-user

# 查看 ClusterRole
kubectl get clusterroles
kubectl describe clusterrole node-reader

# 查看 ClusterRoleBinding
kubectl get clusterrolebindings
kubectl describe clusterrolebinding node-reader-binding

# 删除
kubectl delete clusterrole node-reader
kubectl delete clusterrolebinding node-reader-binding
```

---

## 内置 ClusterRole

### 常用内置角色

| ClusterRole | 权限范围 | 适用场景 |
|------------|---------|---------|
| **cluster-admin** | 所有资源所有操作 | 集群管理员 |
| **admin** | Namespace 内所有资源 | Namespace 管理员 |
| **edit** | 读写大部分资源(不含 RBAC) | 开发者 |
| **view** | 只读大部分资源 | 只读用户 |

### 查看内置角色

```bash
# 查看所有内置 ClusterRole
kubectl get clusterroles | grep system:

# 查看 view 角色详情
kubectl describe clusterrole view

# 查看 edit 角色详情
kubectl describe clusterrole edit

# 查看 admin 角色详情
kubectl describe clusterrole admin

# 查看 cluster-admin 角色详情
kubectl describe clusterrole cluster-admin
```

### 使用内置角色

```yaml
# 授予用户 Namespace 管理员权限
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: admin-binding
  namespace: production
subjects:
- kind: User
  name: alice
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: ClusterRole
  name: admin  # 使用内置 admin 角色
  apiGroup: rbac.authorization.k8s.io

---
# 授予用户只读权限
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: view-binding
  namespace: production
subjects:
- kind: User
  name: bob
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: ClusterRole
  name: view  # 使用内置 view 角色
  apiGroup: rbac.authorization.k8s.io
```

---

## 实战案例

### 案例 1: 开发者权限管理

**需求**: 给开发团队成员授予 development 命名空间的完整权限,但不能修改 RBAC 资源。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: developer
  namespace: development

---
# 2. 使用内置 edit 角色
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: developer-edit
  namespace: development
subjects:
- kind: ServiceAccount
  name: developer
  namespace: development
roleRef:
  kind: ClusterRole
  name: edit  # edit 角色包含大部分资源的读写权限,但不含 RBAC
  apiGroup: rbac.authorization.k8s.io

---
# 3. 额外授予查看 ResourceQuota 的权限
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: view-resourcequota
  namespace: development
rules:
- apiGroups: [""]
  resources: ["resourcequotas"]
  verbs: ["get", "list"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: developer-view-quota
  namespace: development
subjects:
- kind: ServiceAccount
  name: developer
  namespace: development
roleRef:
  kind: Role
  name: view-resourcequota
  apiGroup: rbac.authorization.k8s.io
```

**测试权限**:

```bash
# 创建 ServiceAccount Token
kubectl create token developer -n development

# 使用 Token 测试(假设 Token 保存在 $TOKEN 变量)
kubectl --token=$TOKEN get pods -n development
# 成功

kubectl --token=$TOKEN delete pod mypod -n development
# 成功

kubectl --token=$TOKEN get pods -n production
# 错误: 无权限访问 production 命名空间

kubectl --token=$TOKEN create role test -n development --verb=get --resource=pods
# 错误: 无权限创建 Role
```

### 案例 2: 只读用户

**需求**: 给运维团队授予所有命名空间的只读权限。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: viewer
  namespace: default

---
# 2. 使用内置 view 角色,绑定到集群级别
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: viewer-all-namespaces
subjects:
- kind: ServiceAccount
  name: viewer
  namespace: default
roleRef:
  kind: ClusterRole
  name: view  # 只读权限
  apiGroup: rbac.authorization.k8s.io
```

**测试**:

```bash
# 获取 Token
TOKEN=$(kubectl create token viewer -n default)

# 可以查看所有命名空间的资源
kubectl --token=$TOKEN get pods --all-namespaces
kubectl --token=$TOKEN get deployments -n production
kubectl --token=$TOKEN get services -n development

# 无法修改资源
kubectl --token=$TOKEN delete pod mypod -n production
# 错误: 无权限
```

### 案例 3: CI/CD 部署权限

**需求**: 给 CI/CD 系统授予部署应用的权限。

```yaml
# 1. 创建 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: ci-deployer
  namespace: production

---
# 2. 定义 Role
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: deployer
  namespace: production
rules:
# Deployments 管理
- apiGroups: ["apps"]
  resources: ["deployments"]
  verbs: ["get", "list", "watch", "create", "update", "patch"]

# ReplicaSets 只读(Deployment 会创建)
- apiGroups: ["apps"]
  resources: ["replicasets"]
  verbs: ["get", "list", "watch"]

# Pods 只读
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["get", "list", "watch"]

# ConfigMaps 和 Secrets 读写
- apiGroups: [""]
  resources: ["configmaps", "secrets"]
  verbs: ["get", "list", "watch", "create", "update", "patch"]

# Services 管理
- apiGroups: [""]
  resources: ["services"]
  verbs: ["get", "list", "watch", "create", "update", "patch"]

---
# 3. 绑定
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: ci-deployer-binding
  namespace: production
subjects:
- kind: ServiceAccount
  name: ci-deployer
  namespace: production
roleRef:
  kind: Role
  name: deployer
  apiGroup: rbac.authorization.k8s.io
```

**在 CI/CD 中使用**:

```bash
# GitLab CI 示例
deploy:
  stage: deploy
  script:
    - kubectl create token ci-deployer -n production > /tmp/token
    - kubectl --token=$(cat /tmp/token) apply -f deployment.yaml -n production
    - kubectl --token=$(cat /tmp/token) rollout status deployment/myapp -n production
```

### 案例 4: 审计与检查

```bash
# 检查用户权限
kubectl auth can-i list pods --as=jane -n default
# yes

kubectl auth can-i delete deployments --as=jane -n production
# no

# 检查 ServiceAccount 权限
kubectl auth can-i get secrets \
  --as=system:serviceaccount:default:viewer -n production
# yes

# 列出用户可以执行的所有操作
kubectl auth can-i --list --as=jane -n default

# 查看谁有权限删除 Pods
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.roleRef.name=="admin" or .roleRef.name=="edit") | .metadata.name'
```

---

## 最佳实践

### 1. 最小权限原则

```yaml
# ✅ 推荐: 只授予必要的权限
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: app-logs-reader
  namespace: production
rules:
- apiGroups: [""]
  resources: ["pods/log"]
  verbs: ["get", "list"]

# ❌ 不推荐: 授予过多权限
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: app-logs-reader
  namespace: production
rules:
- apiGroups: [""]
  resources: ["*"]  # 所有资源
  verbs: ["*"]      # 所有操作
```

### 2. 使用 ServiceAccount 而非用户凭证

```yaml
# ✅ 推荐: 使用 ServiceAccount
apiVersion: v1
kind: ServiceAccount
metadata:
  name: myapp
  namespace: production
---
apiVersion: v1
kind: Pod
metadata:
  name: myapp
  namespace: production
spec:
  serviceAccountName: myapp  # 使用专用 SA
  containers:
  - name: app
    image: myapp:1.0

# ❌ 不推荐: 使用默认 ServiceAccount 或用户凭证
spec:
  containers:
  - name: app
    image: myapp:1.0
    env:
    - name: KUBECONFIG
      value: /etc/kubernetes/admin.conf  # 使用 admin 凭证
```

### 3. 定期审计权限

```bash
# 列出所有 ClusterRoleBindings
kubectl get clusterrolebindings -o json | \
  jq -r '.items[] | select(.roleRef.name=="cluster-admin") |
  {name: .metadata.name, subjects: .subjects}'

# 查找过度授权的 ServiceAccount
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.subjects[].kind=="ServiceAccount") |
  {namespace: .metadata.namespace, name: .metadata.name,
   role: .roleRef.name, sa: .subjects[].name}'

# 使用 kubectl-who-can 插件
kubectl krew install who-can
kubectl who-can delete pods -n production
kubectl who-can create secrets --all-namespaces
```

### 4. 使用 Namespace 隔离

```yaml
# ✅ 推荐: 每个团队/项目使用独立 Namespace
apiVersion: v1
kind: Namespace
metadata:
  name: team-a-dev
---
apiVersion: v1
kind: Namespace
metadata:
  name: team-a-prod
---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: team-a-dev-admin
  namespace: team-a-dev
subjects:
- kind: Group
  name: team-a
  apiGroup: rbac.authorization.k8s.io
roleRef:
  kind: ClusterRole
  name: admin
  apiGroup: rbac.authorization.k8s.io
```

### 5. 避免使用通配符

```yaml
# ✅ 推荐: 明确指定资源
rules:
- apiGroups: [""]
  resources: ["pods", "services", "configmaps"]
  verbs: ["get", "list", "watch"]

# ❌ 不推荐: 使用通配符
rules:
- apiGroups: ["*"]
  resources: ["*"]
  verbs: ["*"]
```

---

## 常见问题

### Q1: 如何查看当前用户的权限?

**A**:

```bash
# 方法 1: 使用 auth can-i
kubectl auth can-i --list

# 方法 2: 列出所有可操作的资源
kubectl auth can-i --list --namespace=default

# 方法 3: 检查特定操作
kubectl auth can-i create deployments -n production
kubectl auth can-i delete pods --all-namespaces

# 方法 4: 使用插件
kubectl krew install access-matrix
kubectl access-matrix -n production
```

### Q2: ServiceAccount 自动挂载的 Token 有什么权限?

**A**:

默认情况下,每个 Namespace 都有一个 `default` ServiceAccount,挂载到 Pod 的 Token 只有该 SA 的权限。

```bash
# 查看 default ServiceAccount 的权限
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.subjects[]?.name=="default") |
  {namespace: .metadata.namespace, role: .roleRef.name}'

# 通常,默认 SA 没有任何权限绑定
# 输出为空或只有基本权限

# 禁用自动挂载 Token
apiVersion: v1
kind: ServiceAccount
metadata:
  name: myapp
automountServiceAccountToken: false  # 禁用
```

### Q3: 如何撤销用户的所有权限?

**A**:

```bash
# 1. 查找该用户的所有 RoleBinding 和 ClusterRoleBinding
kubectl get rolebindings,clusterrolebindings \
  --all-namespaces -o json | \
  jq -r '.items[] | select(.subjects[]?.name=="jane") |
  "\(.kind) \(.metadata.name) -n \(.metadata.namespace // "cluster")"'

# 2. 删除相关绑定
kubectl delete rolebinding user-jane-binding -n production
kubectl delete clusterrolebinding user-jane-cluster-binding

# 3. 验证
kubectl auth can-i list pods --as=jane -n production
# no - forbidden
```

### Q4: Role 和 ClusterRole 有什么区别?

**A**:

| 特性 | Role | ClusterRole |
|------|------|-------------|
| **作用域** | 单个 Namespace | 集群级别或所有 Namespace |
| **可授权资源** | Namespace 资源(Pod、Service 等) | 集群资源(Node、PV 等) + Namespace 资源 |
| **绑定方式** | RoleBinding(限定 Namespace) | ClusterRoleBinding(所有 Namespace) 或 RoleBinding(限定 Namespace) |
| **使用场景** | Namespace 管理员、开发者 | 集群管理员、跨 Namespace 权限 |

**示例**:

```yaml
# Role: 只能管理 production 命名空间的 Pods
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: pod-manager
  namespace: production
rules:
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["*"]

---
# ClusterRole: 可以管理所有命名空间的 Pods
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: pod-manager-all
rules:
- apiGroups: [""]
  resources: ["pods"]
  verbs: ["*"]
```

### Q5: 如何调试 RBAC 权限问题?

**A**:

```bash
# 1. 启用 API Server 审计日志
# 编辑 kube-apiserver 配置
--audit-log-path=/var/log/kubernetes/audit.log
--audit-policy-file=/etc/kubernetes/audit-policy.yaml

# 2. 检查 API Server 日志
kubectl logs -n kube-system kube-apiserver-master -f | grep RBAC

# 3. 使用 kubectl auth can-i
kubectl auth can-i get pods --as=jane -n production -v=8
# -v=8 显示详细的调试信息

# 4. 使用 kubectl describe 查看详情
kubectl describe rolebinding user-binding -n production
kubectl describe clusterrole admin

# 5. 使用 kubectl-rbac-tool 插件
kubectl krew install rbac-tool
kubectl rbac-tool lookup jane
kubectl rbac-tool viz  # 可视化 RBAC 关系
```

---

## 总结

- ✅ **Role/ClusterRole**: 定义权限规则,指定可以操作哪些资源
- ✅ **RoleBinding/ClusterRoleBinding**: 绑定权限到用户/组/ServiceAccount
- ✅ **最小权限原则**: 只授予完成任务所需的最小权限
- ✅ **使用 ServiceAccount**: 应用使用 ServiceAccount 而非用户凭证
- ✅ **Namespace 隔离**: 使用 Namespace 隔离不同团队/项目
- ✅ **定期审计**: 定期审查和清理不必要的权限绑定
- ✅ **内置角色**: 优先使用 view、edit、admin 等内置角色

## 参考资源

- [RBAC 官方文档](https://kubernetes.io/docs/reference/access-authn-authz/rbac/)
- [Authorization 概述](https://kubernetes.io/docs/reference/access-authn-authz/authorization/)
- [使用 RBAC 授权](https://kubernetes.io/zh-cn/docs/reference/access-authn-authz/rbac/)
- [kubectl auth can-i](https://kubernetes.io/docs/reference/generated/kubectl/kubectl-commands#-em-can-i-em-)
- [RBAC 最佳实践](https://kubernetes.io/docs/concepts/security/rbac-good-practices/)

---

*更新日期: 2025-12-03*
