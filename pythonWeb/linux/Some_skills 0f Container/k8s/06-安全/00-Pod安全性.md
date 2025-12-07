# Pod安全性

> Pod Security Standards、Security Context

## 概述

【本文档是 Kubernetes 知识体系的一部分】

**Pod 安全性是 Kubernetes 集群安全的基础**。通过 Security Context 和 Pod Security Standards,可以控制 Pod 的权限、能力和访问控制,防止容器逃逸、权限提升等安全风险,确保应用在受限的安全环境中运行。

## 核心概念

### 什么是 Pod 安全性

Pod 安全性涉及多个层面:

| 安全层面 | 说明 | 实现方式 |
|---------|------|---------|
| **Security Context** | 容器运行时安全设置 | runAsUser、capabilities 等 |
| **Pod Security Standards** | Pod 安全标准 | Privileged、Baseline、Restricted |
| **Pod Security Admission** | 准入控制器 | 强制执行安全标准 |
| **AppArmor/SELinux** | 强制访问控制 | 限制进程权限 |
| **Seccomp** | 系统调用过滤 | 限制容器可调用的系统调用 |

### 为什么需要 Pod 安全性

| 安全风险 | 后果 | 防御措施 |
|---------|------|---------|
| **特权容器** | 容器逃逸,控制宿主机 | 禁用 privileged |
| **Root 用户运行** | 权限过大,易受攻击 | runAsNonRoot |
| **挂载宿主机路径** | 访问敏感文件 | 限制 hostPath |
| **危险 Capabilities** | 提权、网络嗅探 | 删除不必要的 capabilities |
| **共享主机命名空间** | 攻击宿主机进程 | 禁用 hostNetwork、hostPID |

---

## Security Context

### 1. 容器级别 Security Context

#### 1.1 运行用户设置

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: security-context-demo
spec:
  containers:
  - name: app
    image: busybox:1.35
    command: ["sh", "-c", "sleep 3600"]
    securityContext:
      # 以非 root 用户运行
      runAsUser: 1000
      runAsGroup: 3000
      # 强制非 root 用户
      runAsNonRoot: true
```

**验证**:
```bash
kubectl exec security-context-demo -- id
# uid=1000 gid=3000
```

#### 1.2 文件系统权限

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: fs-permission-demo
spec:
  containers:
  - name: app
    image: nginx:1.21
    securityContext:
      # 只读根文件系统
      readOnlyRootFilesystem: true
      # 允许权限提升(默认 true)
      allowPrivilegeEscalation: false
    volumeMounts:
    - name: cache
      mountPath: /var/cache/nginx
    - name: run
      mountPath: /var/run
  volumes:
  - name: cache
    emptyDir: {}
  - name: run
    emptyDir: {}
```

**readOnlyRootFilesystem 的好处**:
- 防止恶意程序写入文件
- 防止配置被篡改
- 符合不可变基础设施原则

#### 1.3 Linux Capabilities

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: capabilities-demo
spec:
  containers:
  - name: app
    image: nginx:1.21
    securityContext:
      capabilities:
        # 删除所有 capabilities
        drop:
        - ALL
        # 只添加必需的 capabilities
        add:
        - NET_BIND_SERVICE  # 绑定 1024 以下端口
        - CHOWN             # 修改文件所有者
```

**常见 Capabilities**:

| Capability | 说明 | 风险等级 |
|-----------|------|---------|
| **ALL** | 所有权限 | 高 |
| **SYS_ADMIN** | 系统管理 | 高 |
| **NET_ADMIN** | 网络管理 | 中 |
| **NET_BIND_SERVICE** | 绑定特权端口 | 低 |
| **CHOWN** | 修改文件所有者 | 低 |
| **DAC_OVERRIDE** | 绕过文件权限检查 | 中 |
| **SETUID/SETGID** | 修改用户/组 ID | 中 |

#### 1.4 特权模式(危险)

```yaml
# ❌ 不推荐:特权模式
apiVersion: v1
kind: Pod
metadata:
  name: privileged-pod
spec:
  containers:
  - name: app
    image: nginx:1.21
    securityContext:
      privileged: true  # 拥有宿主机所有权限
```

**特权模式的风险**:
- 可以访问所有设备
- 可以加载内核模块
- 可以修改内核参数
- 容易逃逸到宿主机

### 2. Pod 级别 Security Context

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: pod-security-context
spec:
  # Pod 级别设置
  securityContext:
    runAsUser: 1000
    runAsGroup: 3000
    fsGroup: 2000        # Volume 的文件系统组
    fsGroupChangePolicy: "OnRootMismatch"
    supplementalGroups:  # 附加组
    - 4000
    - 5000
    # SELinux 选项
    seLinuxOptions:
      level: "s0:c123,c456"
    # Seccomp 配置
    seccompProfile:
      type: RuntimeDefault
    # Sysctl 配置
    sysctls:
    - name: net.ipv4.ip_forward
      value: "1"

  containers:
  - name: app
    image: nginx:1.21
    # 容器级别可以覆盖 Pod 级别设置
    securityContext:
      runAsUser: 2000  # 覆盖 Pod 的 1000
```

#### fsGroup 说明

```yaml
# fsGroup 示例
apiVersion: v1
kind: Pod
metadata:
  name: fsgroup-demo
spec:
  securityContext:
    fsGroup: 2000
  containers:
  - name: app
    image: busybox:1.35
    command: ["sh", "-c", "ls -l /data && sleep 3600"]
    volumeMounts:
    - name: data
      mountPath: /data
  volumes:
  - name: data
    emptyDir: {}
```

**验证**:
```bash
kubectl logs fsgroup-demo
# drwxrwsrwx 2 root 2000 4096 Dec  3 10:00 /data
#                  ^^^^
#                  fsGroup
```

---

## Pod Security Standards

### 三种安全标准

Kubernetes 定义了三种 Pod 安全标准:

| 标准 | 限制级别 | 适用场景 |
|-----|---------|---------|
| **Privileged** | 无限制 | 系统组件、特殊场景 |
| **Baseline** | 最小限制 | 一般应用 |
| **Restricted** | 高度限制 | 安全敏感应用 |

### 1. Privileged(特权)

**允许**:
- 所有配置
- 特权容器
- hostNetwork、hostPID、hostIPC

**适用场景**:
- CNI 网络插件
- 存储插件
- 日志收集代理

### 2. Baseline(基线)

**禁止**:
- ✅ 特权容器
- ✅ hostNetwork、hostPID、hostIPC
- ✅ hostPath(部分路径)
- ✅ 添加危险 capabilities
- ✅ 修改 /proc 挂载

**允许**:
- ✅ 以 root 用户运行
- ✅ 使用默认 capabilities
- ✅ 使用 hostPorts

**适用场景**:
- 大部分应用
- 生产环境最低标准

### 3. Restricted(限制)

**禁止**:
- ✅ Baseline 的所有禁止项
- ✅ 以 root 用户运行
- ✅ 默认 capabilities
- ✅ hostPorts
- ✅ seccomp: Unconfined

**要求**:
- ✅ runAsNonRoot: true
- ✅ 删除所有 capabilities
- ✅ readOnlyRootFilesystem: true(推荐)
- ✅ seccompProfile: RuntimeDefault 或 Localhost
- ✅ allowPrivilegeEscalation: false

**适用场景**:
- 高安全要求应用
- 金融、医疗等行业

---

## Pod Security Admission

### 1. 配置 Pod Security Admission

```yaml
# Namespace 级别配置
apiVersion: v1
kind: Namespace
metadata:
  name: production
  labels:
    # 强制执行 restricted 标准
    pod-security.kubernetes.io/enforce: restricted
    pod-security.kubernetes.io/enforce-version: latest

    # 审计模式(记录违规但不阻止)
    pod-security.kubernetes.io/audit: restricted
    pod-security.kubernetes.io/audit-version: latest

    # 警告模式(返回警告但不阻止)
    pod-security.kubernetes.io/warn: restricted
    pod-security.kubernetes.io/warn-version: latest
```

### 2. 三种模式

| 模式 | 行为 | 用途 |
|-----|------|------|
| **enforce** | 违规则拒绝创建 | 强制执行 |
| **audit** | 记录到审计日志 | 监控合规性 |
| **warn** | 返回警告信息 | 提醒用户 |

### 3. 示例:逐步迁移

```yaml
---
# 阶段 1: 警告和审计
apiVersion: v1
kind: Namespace
metadata:
  name: dev
  labels:
    pod-security.kubernetes.io/warn: baseline
    pod-security.kubernetes.io/audit: baseline
# 不阻止,只提醒

---
# 阶段 2: 强制 baseline
apiVersion: v1
kind: Namespace
metadata:
  name: staging
  labels:
    pod-security.kubernetes.io/enforce: baseline
    pod-security.kubernetes.io/warn: restricted
    pod-security.kubernetes.io/audit: restricted
# 强制 baseline,提醒 restricted

---
# 阶段 3: 强制 restricted
apiVersion: v1
kind: Namespace
metadata:
  name: production
  labels:
    pod-security.kubernetes.io/enforce: restricted
    pod-security.kubernetes.io/audit: restricted
    pod-security.kubernetes.io/warn: restricted
# 全面强制 restricted
```

### 4. 测试 Pod 合规性

```bash
# 方法 1: --dry-run
kubectl apply -f pod.yaml --dry-run=server

# 方法 2: kubectl label
kubectl label namespace default \
  pod-security.kubernetes.io/warn=restricted --overwrite

# 方法 3: 使用 kubectl plugin
kubectl krew install pss
kubectl pss check pod.yaml
```

---

## Restricted 标准完整示例

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: restricted-pod
spec:
  # Pod 级别安全设置
  securityContext:
    runAsNonRoot: true
    runAsUser: 1000
    fsGroup: 2000
    seccompProfile:
      type: RuntimeDefault

  containers:
  - name: app
    image: nginx:1.21
    ports:
    - containerPort: 8080  # 非特权端口

    # 容器级别安全设置
    securityContext:
      allowPrivilegeEscalation: false
      runAsNonRoot: true
      runAsUser: 1000
      readOnlyRootFilesystem: true
      capabilities:
        drop:
        - ALL
      seccompProfile:
        type: RuntimeDefault

    # 只读根文件系统需要的临时目录
    volumeMounts:
    - name: cache
      mountPath: /var/cache/nginx
    - name: run
      mountPath: /var/run

  volumes:
  - name: cache
    emptyDir: {}
  - name: run
    emptyDir: {}
```

---

## Seccomp

### 什么是 Seccomp

Seccomp(Secure Computing Mode)是 Linux 内核提供的系统调用过滤机制。

### 1. 使用默认 Profile

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: seccomp-default
spec:
  securityContext:
    seccompProfile:
      type: RuntimeDefault  # 使用容器运行时的默认 profile
  containers:
  - name: app
    image: nginx:1.21
```

### 2. 自定义 Seccomp Profile

```json
{
  "defaultAction": "SCMP_ACT_ERRNO",
  "architectures": [
    "SCMP_ARCH_X86_64"
  ],
  "syscalls": [
    {
      "names": [
        "accept4",
        "epoll_wait",
        "pselect6",
        "futex",
        "madvise",
        "epoll_ctl",
        "getsockname",
        "setsockopt",
        "vfork",
        "mmap",
        "read",
        "write",
        "close",
        "arch_prctl",
        "sched_getaffinity",
        "munmap",
        "brk",
        "rt_sigaction",
        "rt_sigprocmask",
        "sigaltstack",
        "gettid",
        "clone",
        "bind",
        "socket",
        "openat",
        "readlinkat",
        "exit_group",
        "epoll_create1",
        "listen",
        "rt_sigreturn",
        "sched_yield",
        "clock_gettime",
        "connect",
        "dup2",
        "epoll_pwait",
        "execve",
        "exit",
        "fcntl",
        "getpid",
        "getuid",
        "ioctl",
        "mprotect",
        "nanosleep",
        "open",
        "poll",
        "recvfrom",
        "sendto",
        "set_tid_address",
        "setitimer",
        "writev"
      ],
      "action": "SCMP_ACT_ALLOW"
    }
  ]
}
```

**使用自定义 Profile**:
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: seccomp-custom
spec:
  securityContext:
    seccompProfile:
      type: Localhost
      localhostProfile: profiles/audit.json
  containers:
  - name: app
    image: nginx:1.21
```

---

## AppArmor

### 1. 启用 AppArmor

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: apparmor-pod
  annotations:
    # 容器名: profile 类型
    container.apparmor.security.beta.kubernetes.io/app: runtime/default
spec:
  containers:
  - name: app
    image: nginx:1.21
```

### 2. 自定义 AppArmor Profile

```bash
# 创建 AppArmor profile
cat > /etc/apparmor.d/k8s-nginx <<EOF
#include <tunables/global>

profile k8s-nginx flags=(attach_disconnected,mediate_deleted) {
  #include <abstractions/base>

  network inet tcp,
  network inet udp,

  /usr/sbin/nginx ix,
  /var/log/nginx/* w,
  /var/cache/nginx/** rw,
  /usr/share/nginx/** r,
  /etc/nginx/** r,
}
EOF

# 加载 profile
apparmor_parser -r /etc/apparmor.d/k8s-nginx

# 验证
aa-status | grep k8s-nginx
```

**使用自定义 Profile**:
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: apparmor-custom
  annotations:
    container.apparmor.security.beta.kubernetes.io/app: localhost/k8s-nginx
spec:
  containers:
  - name: app
    image: nginx:1.21
```

---

## 实战案例

### 案例 1: 安全的 Nginx 部署

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: secure-nginx
  namespace: production
spec:
  replicas: 3
  selector:
    matchLabels:
      app: nginx
  template:
    metadata:
      labels:
        app: nginx
    spec:
      securityContext:
        runAsNonRoot: true
        runAsUser: 101  # nginx 用户
        fsGroup: 101
        seccompProfile:
          type: RuntimeDefault

      containers:
      - name: nginx
        image: nginx:1.21
        ports:
        - containerPort: 8080

        securityContext:
          allowPrivilegeEscalation: false
          readOnlyRootFilesystem: true
          runAsNonRoot: true
          runAsUser: 101
          capabilities:
            drop:
            - ALL

        volumeMounts:
        - name: cache
          mountPath: /var/cache/nginx
        - name: run
          mountPath: /var/run
        - name: config
          mountPath: /etc/nginx/nginx.conf
          subPath: nginx.conf
          readOnly: true

      volumes:
      - name: cache
        emptyDir: {}
      - name: run
        emptyDir: {}
      - name: config
        configMap:
          name: nginx-config
```

**Nginx 配置**:
```nginx
# nginx.conf
events {
    worker_connections 1024;
}

http {
    server {
        listen 8080;  # 非特权端口

        location / {
            root /usr/share/nginx/html;
            index index.html;
        }
    }
}
```

### 案例 2: 迁移到 Restricted 标准

```bash
# 1. 检查当前 Pod 是否符合 restricted
kubectl get pods -n default -o json | \
  jq '.items[] | select(.spec.securityContext.runAsNonRoot != true)'

# 2. 修改 Deployment
kubectl edit deployment myapp

# 添加安全配置
spec:
  template:
    spec:
      securityContext:
        runAsNonRoot: true
        runAsUser: 1000
        seccompProfile:
          type: RuntimeDefault
      containers:
      - name: app
        securityContext:
          allowPrivilegeEscalation: false
          capabilities:
            drop:
            - ALL
          readOnlyRootFilesystem: true

# 3. 启用 Pod Security Admission
kubectl label namespace default \
  pod-security.kubernetes.io/enforce=restricted \
  pod-security.kubernetes.io/warn=restricted

# 4. 验证
kubectl get pods -n default
```

### 案例 3: 安全审计

```bash
# 使用 kubectl plugin 审计
kubectl krew install popeye
kubectl popeye

# 使用 kubeaudit
kubeaudit all -n production

# 输出示例
# Audit Results:
# - Pod 'insecure-app' is running as root
# - Pod 'insecure-app' has privileged: true
# - Pod 'insecure-app' is missing seccompProfile
```

---

## 最佳实践

### 1. 始终使用非 root 用户

```yaml
# ✅ 推荐
spec:
  securityContext:
    runAsNonRoot: true
    runAsUser: 1000
  containers:
  - name: app
    image: myapp:1.0

# ❌ 不推荐
spec:
  containers:
  - name: app
    image: myapp:1.0
    securityContext:
      runAsUser: 0  # root 用户
```

### 2. 删除所有 Capabilities

```yaml
# ✅ 推荐
securityContext:
  capabilities:
    drop:
    - ALL
    add:
    - NET_BIND_SERVICE  # 只添加必需的

# ❌ 不推荐
securityContext:
  capabilities:
    add:
    - SYS_ADMIN  # 危险的 capability
```

### 3. 使用只读根文件系统

```yaml
# ✅ 推荐
securityContext:
  readOnlyRootFilesystem: true
volumeMounts:
- name: tmp
  mountPath: /tmp
volumes:
- name: tmp
  emptyDir: {}

# ❌ 不推荐
securityContext:
  readOnlyRootFilesystem: false  # 可写根文件系统
```

### 4. 禁用特权提升

```yaml
# ✅ 推荐
securityContext:
  allowPrivilegeEscalation: false

# ❌ 不推荐
securityContext:
  allowPrivilegeEscalation: true
```

### 5. 启用 Seccomp

```yaml
# ✅ 推荐
securityContext:
  seccompProfile:
    type: RuntimeDefault

# ❌ 不推荐
securityContext:
  seccompProfile:
    type: Unconfined  # 不限制系统调用
```

### 6. 镜像扫描

```bash
# 使用 Trivy 扫描镜像
trivy image nginx:1.21

# 使用 Grype 扫描
grype nginx:1.21

# CI/CD 集成
# .gitlab-ci.yml
security-scan:
  stage: test
  image: aquasec/trivy:latest
  script:
    - trivy image --exit-code 1 --severity HIGH,CRITICAL myapp:$CI_COMMIT_SHA
```

---

## 常见问题

### Q1: 如何让应用以非 root 用户运行?

**A**:

**方法 1: 修改 Dockerfile**
```dockerfile
FROM nginx:1.21

# 创建非 root 用户
RUN groupadd -r appuser && useradd -r -g appuser appuser

# 修改文件权限
RUN chown -R appuser:appuser /var/cache/nginx /var/run /var/log/nginx

# 切换用户
USER appuser

# 使用非特权端口
EXPOSE 8080
```

**方法 2: Pod 配置覆盖**
```yaml
spec:
  securityContext:
    runAsUser: 1000
    runAsGroup: 3000
```

### Q2: readOnlyRootFilesystem 导致应用无法启动?

**A**: 挂载必要的可写目录

```yaml
spec:
  containers:
  - name: app
    image: myapp:1.0
    securityContext:
      readOnlyRootFilesystem: true
    volumeMounts:
    - name: tmp
      mountPath: /tmp
    - name: var-log
      mountPath: /var/log
    - name: var-cache
      mountPath: /var/cache
  volumes:
  - name: tmp
    emptyDir: {}
  - name: var-log
    emptyDir: {}
  - name: var-cache
    emptyDir: {}
```

### Q3: 如何查看 Pod 使用的 Capabilities?

**A**:
```bash
# 进入容器
kubectl exec -it mypod -- sh

# 查看进程的 capabilities
cat /proc/1/status | grep Cap
# CapInh:	0000000000000000
# CapPrm:	00000000a80425fb
# CapEff:	00000000a80425fb
# CapBnd:	00000000a80425fb
# CapAmb:	0000000000000000

# 解码 capabilities
capsh --decode=00000000a80425fb
```

### Q4: 特权容器有什么替代方案?

**A**:

| 需求 | 特权容器 | 替代方案 |
|-----|---------|---------|
| **访问设备** | privileged: true | 挂载特定设备文件 |
| **修改 sysctl** | privileged: true | Init Container + sysctl |
| **网络配置** | NET_ADMIN | CNI 插件 |
| **加载内核模块** | privileged: true | 宿主机预加载 |

```yaml
# 替代方案示例:挂载设备
spec:
  containers:
  - name: app
    image: myapp:1.0
    volumeMounts:
    - name: dev-fuse
      mountPath: /dev/fuse
  volumes:
  - name: dev-fuse
    hostPath:
      path: /dev/fuse
      type: CharDevice
```

### Q5: 如何在 Namespace 级别设置默认安全策略?

**A**:
```yaml
# 使用 LimitRange 设置默认 Security Context
apiVersion: v1
kind: LimitRange
metadata:
  name: default-security
  namespace: production
spec:
  limits:
  - type: Container
    defaultRequest:
      cpu: "100m"
      memory: "128Mi"
    default:
      cpu: "200m"
      memory: "256Mi"

# 然后使用 Admission Webhook 强制安全配置
# 或使用 OPA/Kyverno 策略引擎
```

---

## 总结

- ✅ **Security Context**: 控制容器运行时权限和能力
- ✅ **Pod Security Standards**: Privileged、Baseline、Restricted 三级标准
- ✅ **Pod Security Admission**: enforce、audit、warn 三种模式
- ✅ **Seccomp**: 限制系统调用,减少攻击面
- ✅ **AppArmor/SELinux**: 强制访问控制
- ✅ **最佳实践**: 非 root、删除 capabilities、只读文件系统
- ✅ **合规性**: 生产环境至少使用 Baseline 标准

## 参考资源

- [Pod Security Standards](https://kubernetes.io/docs/concepts/security/pod-security-standards/)
- [Pod Security Admission](https://kubernetes.io/docs/concepts/security/pod-security-admission/)
- [Security Context](https://kubernetes.io/docs/tasks/configure-pod-container/security-context/)
- [Linux Capabilities](https://man7.org/linux/man-pages/man7/capabilities.7.html)
- [Seccomp](https://kubernetes.io/docs/tutorials/security/seccomp/)
- [AppArmor](https://kubernetes.io/docs/tutorials/security/apparmor/)

---

*更新日期: 2025-12-03*
