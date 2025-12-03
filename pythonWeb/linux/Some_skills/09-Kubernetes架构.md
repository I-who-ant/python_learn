# 09 - Kubernetes架构

> 容器编排系统

## 核心概念

【本文档是容器技术学习系列的第 09 篇】


## K8s 组件

### 控制平面
- kube-apiserver
- etcd
- kube-scheduler
- kube-controller-manager

### 节点组件
- kubelet
- kube-proxy
- container runtime

## 核心对象

```yaml
# Pod
apiVersion: v1
kind: Pod
metadata:
  name: nginx
spec:
  containers:
  - name: nginx
    image: nginx
```

## 下一步

学习 [10-K8s实战运维](./10-K8s实战运维.md)

---

*更新日期: 2025-12-02*
