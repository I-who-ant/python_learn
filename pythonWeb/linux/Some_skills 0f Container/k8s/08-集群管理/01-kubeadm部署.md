# kubeadm部署

> 集群初始化、节点管理、高可用配置

## 概述

【本文档是 Kubernetes 知识体系的一部分】

kubeadm 是 Kubernetes 官方提供的集群部署工具,遵循最佳实践,适合生产环境使用。本文档详细介绍使用 kubeadm 部署单主和高可用 Kubernetes 集群的完整流程。

## 环境准备

### 系统要求

- **操作系统**: Ubuntu 20.04+、CentOS 7+、Debian 10+
- **CPU**: 2核+
- **内存**: 2GB+ (生产环境 4GB+)
- **磁盘**: 20GB+
- **网络**: 节点间网络互通

### 节点规划

**单主集群示例**:
```
master1:  192.168.1.10
worker1:  192.168.1.11
worker2:  192.168.1.12
```

**高可用集群示例**:
```
lb:       192.168.1.9   (VIP)
master1:  192.168.1.10
master2:  192.168.1.11
master3:  192.168.1.12
worker1:  192.168.1.13
worker2:  192.168.1.14
```

### 前置配置(所有节点)

```bash
# 1. 设置主机名
hostnamectl set-hostname master1  # 每个节点设置对应名称

# 2. 配置 hosts
cat >> /etc/hosts << EOF
192.168.1.10 master1
192.168.1.11 worker1
192.168.1.12 worker2
EOF

# 3. 关闭 swap
swapoff -a
sed -i '/ swap / s/^/#/' /etc/fstab

# 4. 禁用 SELinux (CentOS)
setenforce 0
sed -i 's/^SELINUX=enforcing$/SELINUX=permissive/' /etc/selinux/config

# 5. 配置内核参数
cat > /etc/sysctl.d/k8s.conf << EOF
net.bridge.bridge-nf-call-iptables  = 1
net.bridge.bridge-nf-call-ip6tables = 1
net.ipv4.ip_forward                 = 1
EOF

modprobe br_netfilter
sysctl --system

# 6. 配置时间同步
apt-get install -y chrony  # Ubuntu/Debian
# yum install -y chrony    # CentOS
systemctl enable --now chronyd
```

## 安装容器运行时

### 安装 containerd (推荐)

```bash
# 1. 安装依赖
apt-get update
apt-get install -y ca-certificates curl gnupg lsb-release

# 2. 添加 Docker GPG 密钥
mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# 3. 添加仓库
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list

# 4. 安装 containerd
apt-get update
apt-get install -y containerd.io

# 5. 配置 containerd
mkdir -p /etc/containerd
containerd config default > /etc/containerd/config.toml

# 6. 修改配置使用 systemd cgroup
sed -i 's/SystemdCgroup = false/SystemdCgroup = true/' /etc/containerd/config.toml

# 7. 重启 containerd
systemctl restart containerd
systemctl enable containerd
```

## 安装 kubeadm、kubelet、kubectl

### Ubuntu/Debian

```bash
# 1. 更新包索引并安装依赖
apt-get update
apt-get install -y apt-transport-https ca-certificates curl

# 2. 添加 Kubernetes GPG 密钥
curl -fsSL https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add -

# 3. 添加 Kubernetes 仓库
cat << EOF > /etc/apt/sources.list.d/kubernetes.list
deb https://apt.kubernetes.io/ kubernetes-xenial main
EOF

# 4. 安装指定版本
apt-get update
apt-get install -y kubelet=1.28.0-00 kubeadm=1.28.0-00 kubectl=1.28.0-00

# 5. 锁定版本
apt-mark hold kubelet kubeadm kubectl

# 6. 启动 kubelet
systemctl enable kubelet
```

### CentOS/RHEL

```bash
# 1. 添加 Kubernetes 仓库
cat << EOF > /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://packages.cloud.google.com/yum/repos/kubernetes-el7-x86_64
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
EOF

# 2. 安装
yum install -y kubelet-1.28.0 kubeadm-1.28.0 kubectl-1.28.0

# 3. 启动 kubelet
systemctl enable kubelet
```

## 单主集群部署

### 1. 初始化控制平面

```bash
# 在 master 节点执行
kubeadm init \
  --apiserver-advertise-address=192.168.1.10 \
  --pod-network-cidr=10.244.0.0/16 \
  --service-cidr=10.96.0.0/12 \
  --kubernetes-version=v1.28.0 \
  --control-plane-endpoint=master1:6443

# 配置 kubectl
mkdir -p $HOME/.kube
cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
chown $(id -u):$(id -g) $HOME/.kube/config
```

### 2. 安装网络插件

**Calico**:
```bash
kubectl apply -f https://docs.projectcalico.org/manifests/calico.yaml
```

**Flannel**:
```bash
kubectl apply -f https://raw.githubusercontent.com/flannel-io/flannel/master/Documentation/kube-flannel.yml
```

### 3. 加入工作节点

```bash
# 在 worker 节点执行(使用 kubeadm init 输出的命令)
kubeadm join master1:6443 --token <token> \
  --discovery-token-ca-cert-hash sha256:<hash>

# 如果 token 过期,在 master 生成新 token
kubeadm token create --print-join-command
```

### 4. 验证集群

```bash
# 查看节点
kubectl get nodes

# 查看组件状态
kubectl get cs

# 查看 Pod 状态
kubectl get pods -A
```

## 高可用集群部署

### 架构说明

使用堆叠 etcd 模式(etcd 和控制平面在同一节点)。

### 1. 部署负载均衡器

**使用 HAProxy + Keepalived**:

```bash
# 在 lb 节点安装
apt-get install -y haproxy keepalived

# 配置 HAProxy
cat > /etc/haproxy/haproxy.cfg << 'EOF'
global
    log /dev/log local0
    chroot /var/lib/haproxy
    stats socket /run/haproxy/admin.sock mode 660
    stats timeout 30s
    user haproxy
    group haproxy
    daemon

defaults
    log     global
    mode    tcp
    option  tcplog
    option  dontlognull
    timeout connect 5000
    timeout client  50000
    timeout server  50000

frontend kubernetes-apiserver
    bind *:6443
    mode tcp
    option tcplog
    default_backend kubernetes-apiserver

backend kubernetes-apiserver
    mode tcp
    balance roundrobin
    option tcp-check
    server master1 192.168.1.10:6443 check fall 3 rise 2
    server master2 192.168.1.11:6443 check fall 3 rise 2
    server master3 192.168.1.12:6443 check fall 3 rise 2
EOF

# 启动 HAProxy
systemctl restart haproxy
systemctl enable haproxy

# 配置 Keepalived (主节点)
cat > /etc/keepalived/keepalived.conf << 'EOF'
global_defs {
    router_id LVS_DEVEL
}

vrrp_script check_haproxy {
    script "killall -0 haproxy"
    interval 2
    weight 2
}

vrrp_instance VI_1 {
    state MASTER
    interface eth0
    virtual_router_id 51
    priority 100
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass 1234
    }
    virtual_ipaddress {
        192.168.1.9
    }
    track_script {
        check_haproxy
    }
}
EOF

systemctl restart keepalived
systemctl enable keepalived
```

### 2. 初始化第一个控制平面

```bash
# 在 master1 执行
kubeadm init \
  --control-plane-endpoint="192.168.1.9:6443" \
  --upload-certs \
  --pod-network-cidr=10.244.0.0/16 \
  --kubernetes-version=v1.28.0

# 记录输出的命令,包含:
# - worker 节点加入命令
# - 其他控制平面节点加入命令

# 配置 kubectl
mkdir -p $HOME/.kube
cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
```

### 3. 加入其他控制平面节点

```bash
# 在 master2 和 master3 执行
kubeadm join 192.168.1.9:6443 --token <token> \
  --discovery-token-ca-cert-hash sha256:<hash> \
  --control-plane --certificate-key <cert-key>

# 配置 kubectl
mkdir -p $HOME/.kube
cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
```

### 4. 安装网络插件

```bash
# 在任一 master 执行
kubectl apply -f https://docs.projectcalico.org/manifests/calico.yaml
```

### 5. 加入工作节点

```bash
# 在 worker 节点执行
kubeadm join 192.168.1.9:6443 --token <token> \
  --discovery-token-ca-cert-hash sha256:<hash>
```

## 常用操作

### 节点管理

```bash
# 查看节点
kubectl get nodes

# 查看节点详情
kubectl describe node master1

# 标记节点不可调度
kubectl cordon worker1

# 恢复调度
kubectl uncordon worker1

# 驱逐节点上的 Pod
kubectl drain worker1 --ignore-daemonsets --delete-emptydir-data

# 删除节点
kubectl delete node worker1

# 在 worker 节点重置
kubeadm reset
```

### 证书管理

```bash
# 查看证书有效期
kubeadm certs check-expiration

# 续期所有证书
kubeadm certs renew all

# 续期特定证书
kubeadm certs renew apiserver

# 更新 kubeconfig
cp /etc/kubernetes/admin.conf ~/.kube/config
```

### 集群升级

```bash
# 1. 升级 kubeadm
apt-get update && apt-get install -y kubeadm=1.29.0-00

# 2. 检查升级计划
kubeadm upgrade plan

# 3. 升级控制平面
kubeadm upgrade apply v1.29.0

# 4. 升级 kubelet
apt-get install -y kubelet=1.29.0-00 kubectl=1.29.0-00
systemctl restart kubelet

# 5. 依次升级其他节点
```

### 配置修改

```bash
# 查看配置
kubectl -n kube-system get configmap kubeadm-config -o yaml

# 编辑配置
kubectl -n kube-system edit configmap kubeadm-config
```

## 故障排查

### 常见问题

**1. kubeadm init 失败**:
```bash
# 查看日志
journalctl -xeu kubelet

# 重置后重试
kubeadm reset
rm -rf /etc/cni/net.d
rm -rf $HOME/.kube/config
```

**2. 节点 NotReady**:
```bash
# 检查网络插件
kubectl get pods -n kube-system

# 检查 kubelet
systemctl status kubelet
journalctl -u kubelet
```

**3. Pod 无法调度**:
```bash
# 查看节点资源
kubectl describe node <node-name>

# 查看 Pod 事件
kubectl describe pod <pod-name>
```

## 最佳实践

1. **版本选择**
   - 使用稳定版本
   - 测试后再升级生产

2. **高可用配置**
   - 至少 3 个 Master
   - 使用外部负载均衡

3. **安全加固**
   - 定期更新证书
   - 配置 RBAC
   - 启用审计日志

4. **监控备份**
   - 部署监控系统
   - 定期备份 etcd
   - 测试恢复流程

## 总结

- ✅ 掌握 kubeadm 部署流程
- ✅ 理解单主和高可用架构
- ✅ 掌握节点和证书管理
- ✅ 了解集群升级方法
- ✅ 能够排查常见问题

## 参考资源

- [kubeadm 官方文档](https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/)
- [高可用集群](https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/high-availability/)

---

*更新日期: 2025-12-03*
