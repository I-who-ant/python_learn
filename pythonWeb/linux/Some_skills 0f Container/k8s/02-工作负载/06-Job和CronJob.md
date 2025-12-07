# Job和CronJob

> 批处理任务、定时任务、并行性控制

## 概述

【本文档是 Kubernetes 知识体系的一部分】

Job 和 CronJob 用于运行批处理任务和定时任务。Job 确保 Pod 成功完成指定次数,CronJob 则按时间计划创建 Job。

## 核心概念

### 什么是 Job

Job 用于运行一次性或有限次数的任务,确保 Pod 成功完成后退出。

**核心特性:**
- **完成保证**: 确保任务成功完成指定次数
- **失败重试**: 自动重启失败的 Pod
- **并行执行**: 支持并行运行多个 Pod
- **自动清理**: 任务完成后可自动清理

### 什么是 CronJob

CronJob 按照 Cron 表达式定时创建 Job。

**核心特性:**
- **定时执行**: 按时间计划运行任务
- **历史记录**: 保留成功和失败的 Job
- **并发策略**: 控制任务重叠行为
- **挂起暂停**: 可暂停定时任务

### 为什么需要

在生产环境中,我们需要 Job/CronJob 来处理:

1. **数据处理**: 批量数据导入、ETL 任务
2. **定时备份**: 数据库备份、日志归档
3. **定期清理**: 清理临时文件、过期数据
4. **报表生成**: 定时生成统计报表
5. **健康检查**: 定期检查系统状态

## Job 详解

### 基本使用

```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: pi-calculation
spec:
  # 成功完成次数
  completions: 1

  # 并行数
  parallelism: 1

  # 失败重试次数
  backoffLimit: 4

  # 任务超时时间 (秒)
  activeDeadlineSeconds: 600

  # Pod 模板
  template:
    spec:
      restartPolicy: Never  # Job 必须是 Never 或 OnFailure
      containers:
      - name: pi
        image: perl:5.34
        command: ["perl", "-Mbignum=bpi", "-wle", "print bpi(2000)"]
```

### 完成模式

**单次任务 (completions=1)**
```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: single-job
spec:
  completions: 1
  parallelism: 1
  template:
    spec:
      restartPolicy: Never
      containers:
      - name: task
        image: busybox
        command: ['sh', '-c', 'echo "Processing..."; sleep 10; echo "Done!"']
```

**多次完成 (completions=N)**
```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: multi-completion-job
spec:
  completions: 5  # 需要成功完成 5 次
  parallelism: 2  # 每次并行 2 个 Pod
  template:
    spec:
      restartPolicy: Never
      containers:
      - name: worker
        image: busybox
        command: ['sh', '-c', 'echo "Task $HOSTNAME"; sleep 5']
```

**执行流程:**
```
并行度: 2, 完成次数: 5

时刻1: 启动 Pod-1, Pod-2
时刻2: Pod-1 完成, 启动 Pod-3
时刻3: Pod-2 完成, 启动 Pod-4
时刻4: Pod-3 完成, 启动 Pod-5
时刻5: Pod-4 完成
时刻6: Pod-5 完成
总计: 5 个 Pod 成功完成
```

**工作队列模式 (无 completions)**
```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: queue-job
spec:
  # 不设置 completions,由 Pod 自行决定何时完成
  parallelism: 3
  template:
    spec:
      restartPolicy: Never
      containers:
      - name: worker
        image: my-queue-worker:latest
        env:
        - name: QUEUE_URL
          value: "redis://queue:6379"
```

### 失败处理

```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: retry-job
spec:
  # 最多重试 6 次
  backoffLimit: 6

  # 总超时时间 10 分钟
  activeDeadlineSeconds: 600

  template:
    spec:
      restartPolicy: Never  # 失败不重启,创建新 Pod
      containers:
      - name: task
        image: busybox
        command:
        - sh
        - -c
        - |
          # 模拟可能失败的任务
          if [ $RANDOM -gt 20000 ]; then
            echo "Success!"
            exit 0
          else
            echo "Failed, will retry..."
            exit 1
          fi
```

**失败策略:**
```
restartPolicy: Never
- Pod 失败后创建新 Pod
- 重试次数受 backoffLimit 限制

restartPolicy: OnFailure
- Pod 失败后原地重启容器
- 重启次数受 backoffLimit 限制
```

### kubectl 命令

```bash
# 创建 Job
kubectl apply -f job.yaml
kubectl create job test --image=busybox -- echo "Hello"

# 查看 Job
kubectl get jobs
kubectl get job pi-calculation

# 查看 Job 详情
kubectl describe job pi-calculation

# 查看 Job 的 Pod
kubectl get pods -l job-name=pi-calculation

# 查看 Job 日志
kubectl logs -l job-name=pi-calculation

# 删除 Job (同时删除 Pod)
kubectl delete job pi-calculation

# 删除完成的 Job
kubectl delete job --field-selector status.successful=1
```

## CronJob 详解

### 基本使用

```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: backup-job
spec:
  # Cron 表达式: 每天凌晨 2:00
  schedule: "0 2 * * *"

  # 时区 (可选,K8s 1.25+)
  timeZone: "Asia/Shanghai"

  # 并发策略
  concurrencyPolicy: Forbid

  # 保留历史 Job 数量
  successfulJobsHistoryLimit: 3
  failedJobsHistoryLimit: 1

  # Job 模板
  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: backup
            image: backup-tool:latest
            command:
            - sh
            - -c
            - |
              echo "Starting backup at $(date)"
              # 执行备份逻辑
              pg_dump mydb > /backup/backup-$(date +%Y%m%d).sql
              echo "Backup completed"
```

### Cron 表达式

```
格式: 分 时 日 月 周
     ┌───────────── 分钟 (0 - 59)
     │ ┌───────────── 小时 (0 - 23)
     │ │ ┌───────────── 日 (1 - 31)
     │ │ │ ┌───────────── 月 (1 - 12)
     │ │ │ │ ┌───────────── 星期 (0 - 6, 0=周日)
     │ │ │ │ │
     * * * * *
```

**常用示例:**
```yaml
# 每分钟执行
schedule: "* * * * *"

# 每小时执行
schedule: "0 * * * *"

# 每天凌晨 2:30
schedule: "30 2 * * *"

# 每周一上午 9:00
schedule: "0 9 * * 1"

# 每月 1 号凌晨 1:00
schedule: "0 1 1 * *"

# 每 5 分钟
schedule: "*/5 * * * *"

# 工作日早上 8:00
schedule: "0 8 * * 1-5"

# 每季度第一天
schedule: "0 0 1 1,4,7,10 *"
```

### 并发策略

**Allow (默认)**
```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: allow-concurrent
spec:
  schedule: "*/1 * * * *"
  concurrencyPolicy: Allow  # 允许并发运行
  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: task
            image: busybox
            command: ['sh', '-c', 'sleep 120']  # 任务需要 2 分钟
```

**执行流程:**
```
0分: Job-1 启动
1分: Job-2 启动 (Job-1 还在运行)
2分: Job-3 启动 (Job-1, Job-2 还在运行)
Job-1, Job-2, Job-3 并发运行
```

**Forbid (禁止并发)**
```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: forbid-concurrent
spec:
  schedule: "*/1 * * * *"
  concurrencyPolicy: Forbid  # 如果上次未完成,跳过本次
  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: task
            image: busybox
            command: ['sh', '-c', 'sleep 120']
```

**执行流程:**
```
0分: Job-1 启动
1分: 跳过 (Job-1 还在运行)
2分: Job-1 完成, Job-2 启动
3分: 跳过 (Job-2 还在运行)
```

**Replace (替换)**
```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: replace-job
spec:
  schedule: "*/1 * * * *"
  concurrencyPolicy: Replace  # 终止旧 Job,启动新 Job
  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: task
            image: busybox
            command: ['sh', '-c', 'sleep 120']
```

**执行流程:**
```
0分: Job-1 启动
1分: 终止 Job-1, Job-2 启动
2分: 终止 Job-2, Job-3 启动
```

### 挂起和恢复

```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: pausable-job
spec:
  schedule: "0 * * * *"
  suspend: true  # 暂停定时任务
  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: task
            image: busybox
            command: ['echo', 'Hello']
```

操作:
```bash
# 暂停 CronJob
kubectl patch cronjob pausable-job -p '{"spec":{"suspend":true}}'

# 恢复 CronJob
kubectl patch cronjob pausable-job -p '{"spec":{"suspend":false}}'
```

### kubectl 命令

```bash
# 创建 CronJob
kubectl apply -f cronjob.yaml
kubectl create cronjob test --image=busybox --schedule="*/1 * * * *" -- echo "Hello"

# 查看 CronJob
kubectl get cronjobs
kubectl get cj  # 简写

# 查看 CronJob 详情
kubectl describe cronjob backup-job

# 查看 CronJob 创建的 Job
kubectl get jobs -l cronjob.kubernetes.io/parent=backup-job

# 手动触发一次
kubectl create job manual-backup --from=cronjob/backup-job

# 暂停 CronJob
kubectl patch cronjob backup-job -p '{"spec":{"suspend":true}}'

# 恢复 CronJob
kubectl patch cronjob backup-job -p '{"spec":{"suspend":false}}'

# 删除 CronJob
kubectl delete cronjob backup-job
```

## 实战案例

### 案例 1: 数据库备份

```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: db-backup
  namespace: production
spec:
  # 每天凌晨 3:00 备份
  schedule: "0 3 * * *"
  timeZone: "Asia/Shanghai"
  concurrencyPolicy: Forbid

  successfulJobsHistoryLimit: 7  # 保留 7 天
  failedJobsHistoryLimit: 3

  jobTemplate:
    spec:
      backoffLimit: 2
      activeDeadlineSeconds: 3600  # 1小时超时

      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: backup
            image: postgres:13
            command:
            - sh
            - -c
            - |
              BACKUP_FILE="/backup/postgres-$(date +%Y%m%d-%H%M%S).sql.gz"
              echo "Starting backup to $BACKUP_FILE"

              # 执行备份
              PGPASSWORD=$POSTGRES_PASSWORD pg_dump \
                -h $POSTGRES_HOST \
                -U $POSTGRES_USER \
                -d $POSTGRES_DB \
                | gzip > $BACKUP_FILE

              # 验证备份
              if [ -f "$BACKUP_FILE" ] && [ -s "$BACKUP_FILE" ]; then
                echo "Backup successful: $(ls -lh $BACKUP_FILE)"

                # 上传到对象存储 (可选)
                # aws s3 cp $BACKUP_FILE s3://backups/

                # 清理 30 天前的备份
                find /backup -name "postgres-*.sql.gz" -mtime +30 -delete

                exit 0
              else
                echo "Backup failed!"
                exit 1
              fi

            env:
            - name: POSTGRES_HOST
              value: "postgres-service"
            - name: POSTGRES_DB
              value: "mydb"
            - name: POSTGRES_USER
              valueFrom:
                secretKeyRef:
                  name: postgres-secret
                  key: username
            - name: POSTGRES_PASSWORD
              valueFrom:
                secretKeyRef:
                  name: postgres-secret
                  key: password

            volumeMounts:
            - name: backup
              mountPath: /backup

          volumes:
          - name: backup
            persistentVolumeClaim:
              claimName: backup-pvc
```

### 案例 2: 数据处理任务

```yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: data-processing
spec:
  completions: 10  # 处理 10 个批次
  parallelism: 3   # 并行 3 个

  backoffLimit: 6
  activeDeadlineSeconds: 7200  # 2小时

  template:
    metadata:
      labels:
        app: data-processor
    spec:
      restartPolicy: OnFailure
      containers:
      - name: processor
        image: data-processor:1.0
        command:
        - python
        - process.py
        args:
        - --batch-size
        - "1000"
        - --output
        - "/output"

        env:
        - name: DATABASE_URL
          valueFrom:
            secretKeyRef:
              name: db-secret
              key: url
        - name: JOB_COMPLETION_INDEX
          valueFrom:
            fieldRef:
              fieldPath: metadata.annotations['batch.kubernetes.io/job-completion-index']

        resources:
          requests:
            memory: "1Gi"
            cpu: "500m"
          limits:
            memory: "2Gi"
            cpu: "1000m"

        volumeMounts:
        - name: output
          mountPath: /output

      volumes:
      - name: output
        persistentVolumeClaim:
          claimName: data-output-pvc
```

### 案例 3: 定期清理任务

```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: cleanup-job
spec:
  # 每天凌晨 1:00
  schedule: "0 1 * * *"
  concurrencyPolicy: Replace

  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          serviceAccountName: cleanup-sa

          containers:
          - name: cleanup
            image: bitnami/kubectl:latest
            command:
            - sh
            - -c
            - |
              echo "=== Starting cleanup at $(date) ==="

              # 清理完成的 Job (保留 24 小时)
              echo "Cleaning up old jobs..."
              kubectl delete jobs --field-selector status.successful=1 \
                --all-namespaces \
                --ignore-not-found=true \
                --dry-run=client -o name | \
                while read job; do
                  AGE=$(kubectl get $job -o jsonpath='{.status.completionTime}')
                  if [ ! -z "$AGE" ]; then
                    kubectl delete $job
                  fi
                done

              # 清理 Evicted Pods
              echo "Cleaning up evicted pods..."
              kubectl get pods --all-namespaces --field-selector=status.phase=Failed \
                -o json | \
                jq -r '.items[] | select(.status.reason=="Evicted") |
                       "\(.metadata.namespace) \(.metadata.name)"' | \
                while read namespace pod; do
                  kubectl delete pod -n $namespace $pod
                done

              # 清理未使用的 ConfigMap 和 Secret (慎用!)
              # ...

              echo "=== Cleanup completed at $(date) ==="
```

### 案例 4: 报表生成

```yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: daily-report
spec:
  # 每天早上 8:00 生成前一天报表
  schedule: "0 8 * * *"
  timeZone: "Asia/Shanghai"
  concurrencyPolicy: Forbid

  successfulJobsHistoryLimit: 30

  jobTemplate:
    spec:
      template:
        spec:
          restartPolicy: OnFailure
          containers:
          - name: report-generator
            image: report-generator:latest
            command:
            - python
            - generate_report.py
            args:
            - --date
            - "yesterday"
            - --format
            - "pdf"
            - --email
            - "team@example.com"

            env:
            - name: DATABASE_URL
              valueFrom:
                secretKeyRef:
                  name: db-secret
                  key: url
            - name: SMTP_HOST
              valueFrom:
                configMapKeyRef:
                  name: mail-config
                  key: host
            - name: SMTP_USER
              valueFrom:
                secretKeyRef:
                  name: mail-secret
                  key: username
            - name: SMTP_PASSWORD
              valueFrom:
                secretKeyRef:
                  name: mail-secret
                  key: password

            resources:
              requests:
                memory: "512Mi"
                cpu: "250m"
              limits:
                memory: "1Gi"
                cpu: "500m"
```

## 最佳实践

### 1. 设置合理超时

```yaml
spec:
  activeDeadlineSeconds: 3600  # 1小时超时
  backoffLimit: 3  # 最多重试 3 次
```

### 2. 资源限制

```yaml
resources:
  requests:
    memory: "256Mi"
    cpu: "250m"
  limits:
    memory: "512Mi"
    cpu: "500m"
```

### 3. 历史清理

```yaml
# Job 自动清理 (K8s 1.23+)
spec:
  ttlSecondsAfterFinished: 86400  # 完成后 24 小时删除

# CronJob 历史限制
spec:
  successfulJobsHistoryLimit: 3
  failedJobsHistoryLimit: 1
```

### 4. 幂等性

```bash
# 任务应该是幂等的,可以安全重试
# 使用锁或唯一标识避免重复执行
```

### 5. 监控告警

```yaml
# 配置 Prometheus 监控
# 告警规则示例
- alert: CronJobFailed
  expr: kube_cronjob_status_last_schedule_time - time() > 3600
  annotations:
    summary: "CronJob {{ $labels.cronjob }} failed"
```

## 常见问题

### Q1: Job 一直不完成?

**排查:**
```bash
kubectl describe job my-job
kubectl logs -l job-name=my-job
kubectl get pods -l job-name=my-job
```

**常见原因:**
- 程序未正常退出
- restartPolicy 设置错误
- 超时未设置

### Q2: CronJob 未按时执行?

**排查:**
```bash
kubectl describe cronjob my-cronjob
kubectl get jobs -l cronjob.kubernetes.io/parent=my-cronjob

# 检查时区
kubectl get cronjob my-cronjob -o yaml | grep timeZone
```

**常见原因:**
- schedule 表达式错误
- suspend 设置为 true
- 并发策略阻止执行

### Q3: Job 失败重试次数过多?

**解决:**
```yaml
spec:
  backoffLimit: 3  # 限制重试次数
  activeDeadlineSeconds: 600  # 设置总超时
```

### Q4: 如何查看 CronJob 下次执行时间?

```bash
kubectl get cronjob my-cronjob -o jsonpath='{.status.lastScheduleTime}'
```

## 总结

- ✅ 理解 Job 和 CronJob 的使用场景
- ✅ 掌握并行执行和失败重试机制
- ✅ 熟悉 Cron 表达式和并发策略
- ✅ 能够实现备份、清理等定时任务
- ✅ 了解最佳实践和故障排查

## 参考资源

- [Job 官方文档](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/job/)
- [CronJob 官方文档](https://kubernetes.io/zh-cn/docs/concepts/workloads/controllers/cron-jobs/)
- [Cron 表达式](https://crontab.guru/)

---

*更新日期: 2025-12-03*
