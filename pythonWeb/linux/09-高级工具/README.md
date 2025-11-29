# 09 - 高级工具

> 容器、虚拟化、压缩等高级应用

## 目录

- [压缩工具](#压缩工具)
- [容器技术](#容器技术)
- [虚拟化](#虚拟化)
- [磁盘工具](#磁盘工具)
- [版本控制](#版本控制)
- [安全工具](#安全工具)
- [加密和哈希](#加密和哈希)
- [版本控制系统对比](#版本控制系统对比)
- [其他工具](#其他工具)

---

## 压缩工具

### tar - 磁带归档

```bash
# 基本语法
tar [选项] [文件]

# 创建压缩包
tar -czf archive.tar.gz files/      # gzip压缩
tar -cjf archive.tar.bz2 files/     # bzip2压缩
tar -cJf archive.tar.xz files/      # xz压缩（压缩率最高）

# 解压缩包
tar -xzf archive.tar.gz             # 解压到当前目录
tar -xzf archive.tar.gz -C /path/to/dir  # 解压到指定目录

# 查看压缩包内容
tar -tzf archive.tar.gz

# 提取特定文件
tar -xzf archive.tar.gz file.txt

# 保留权限和属性
tar -czpvf archive.tar.gz files/

# 排除文件
tar -czf archive.tar.gz files/ --exclude='*.log' --exclude='*.tmp'

# 增量备份
tar -czvf backup-$(date +%Y%m%d).tar.gz /path --listed-incremental=/var/log/backup.snar

# 排除目录
tar -czf archive.tar.gz files/ --exclude='files/ignore'

# 从标准输入创建
tar -czf - files/ | ssh user@server 'tar -xzf - -C /backup/'

# 从远程复制
tar -czf - files/ | nc -q 1 server 1234
```

### gzip / gunzip

```bash
# 压缩文件
gzip file.txt              # 生成 file.txt.gz，删除原文件
gzip -k file.txt           # 保留原文件
gzip -r directory/         # 递归压缩目录中所有文件

# 解压缩
gunzip file.txt.gz
gzip -d file.txt.gz

# 显示压缩比
gzip -l file.txt.gz

# 测试完整性
gzip -t file.txt.gz

# 解压到标准输出
gzip -dc file.txt.gz
```

### bzip2 / bunzip2

```bash
# 压缩（比gzip压缩率更高，但速度较慢）
bzip2 file.txt
bzip2 -k file.txt          # 保留原文件

# 解压缩
bunzip2 file.txt.bz2

# 解压到标准输出
bzip2 -dc file.txt.bz2
```

### xz / unxz

```bash
# 压缩（压缩率最高，速度最慢）
xz file.txt
xz -k file.txt             # 保留原文件

# 解压缩
unxz file.txt.xz

# 解压到标准输出
xz -dc file.txt.xz

# 多线程压缩
xz -T 4 file.txt           # 使用4个线程

# 极快模式
xz -0 file.txt
xz -1 file.txt             # 更快
xz -9 file.txt             # 最高压缩比
```

### zip / unzip

```bash
# 安装
sudo pacman -S zip unzip

# 创建zip文件
zip -r archive.zip directory/
zip -r archive.zip file1.txt file2.txt

# 设置压缩级别
zip -9 -r archive.zip directory/  # 最大压缩

# 排除文件
zip -r archive.zip directory/ -x "*.log" "*.tmp"

# 创建自解压文件
zip -A archive.zip

# 更新zip文件
zip -r archive.zip newfile.txt

# 删除zip文件中的文件
zip -d archive.zip file.txt

# 查看zip内容
unzip -l archive.zip

# 测试zip文件
unzip -t archive.zip

# 解压到指定目录
unzip archive.zip -d /path/to/dir

# 解压覆盖提示
unzip -o archive.zip       # 自动覆盖
unzip -q archive.zip       # 静默模式

# 解压单个文件
unzip archive.zip specific-file.txt

# 从标准输入创建zip
cat files | zip archive.zip -

# 带密码保护
zip -r -e archive.zip directory/
```

### 7z / p7zip

```bash
# 安装
sudo pacman -S p7zip

# 创建7z压缩包
7z a archive.7z files/

# 创建带密码的压缩包
7z a -p archive.7z files/

# 设置压缩级别
7z a -mx=9 archive.7z files/  # 最大压缩
7z a -mx=0 archive.7z files/  # 存储（不压缩）

# 解压7z文件
7z x archive.7z              # x=Extract with full paths

# 查看压缩包内容
7z l archive.7z

# 测试压缩包
7z t archive.7z

# 更新压缩包
7z u archive.7z newfile.txt

# 删除压缩包中的文件
7z d archive.7z file.txt

# 转换格式
7z x archive.zip -ooutput_dir/
```

### 压缩工具对比

| 工具 | 压缩率 | 速度 | 特性 |
|------|--------|------|------|
| gzip | 中等 | 快 | 广泛支持 |
| bzip2 | 较好 | 中 | 较好压缩率 |
| xz | 最好 | 慢 | 最高压缩率 |
| zip | 良好 | 中 | 广泛支持，跨平台 |
| 7z | 最好 | 慢 | 高压缩率，功能丰富 |

**选择建议**:
- 一般用途: `gzip`
- 需要最大压缩: `xz` 或 `7z`
- 跨平台: `zip`
- 快速传输: `gzip`
- 长时间存储: `xz` 或 `7z`

---

## 容器技术

### Docker 基础

```bash
# 安装 Docker
sudo pacman -S docker

# 启动 Docker 服务
sudo systemctl start docker
sudo systemctl enable docker

# 将用户添加到 docker 组
sudo usermod -aG docker $USER
# 重新登录或执行: newgrp docker

# 查看 Docker 版本
docker --version
docker version
docker info

# 查看帮助
docker --help
docker <command> --help
```

**镜像操作**:
```bash
# 搜索镜像
docker search ubuntu

# 拉取镜像
docker pull ubuntu:20.04
docker pull nginx:latest

# 列出镜像
docker images
docker image ls

# 删除镜像
docker rmi nginx:latest
docker image rm ubuntu:20.04

# 构建镜像
docker build -t myapp:1.0 .
# Dockerfile 示例:
cat > Dockerfile <<'EOF'
FROM ubuntu:20.04
RUN apt-get update && apt-get install -y nginx
COPY . /var/www/html
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
EOF

# 构建时指定 Dockerfile
docker build -f custom.dockerfile -t myapp:1.0 .

# 查看镜像详细信息
docker inspect ubuntu:20.04

# 清理未使用镜像
docker image prune
docker image prune -a  # 删除所有未使用镜像
```

**容器操作**:
```bash
# 运行容器
docker run ubuntu:20.04 /bin/bash

# 后台运行容器
docker run -d --name myapp nginx

# 运行容器并进入交互模式
docker run -it ubuntu:20.04 /bin/bash

# 运行容器并映射端口
docker run -d -p 8080:80 --name web nginx

# 运行容器并挂载目录
docker run -v /host/path:/container/path nginx

# 运行容器并设置环境变量
docker run -e MYSQL_ROOT_PASSWORD=secret mysql

# 查看运行中的容器
docker ps

# 查看所有容器（包括已停止）
docker ps -a

# 停止容器
docker stop myapp
docker stop $(docker ps -q)  # 停止所有运行中的容器

# 启动容器
docker start myapp

# 重启容器
docker restart myapp

# 删除容器
docker rm myapp
docker rm $(docker ps -aq)  # 删除所有容器

# 进入运行中的容器
docker exec -it myapp /bin/bash
docker attach myapp

# 查看容器日志
docker logs myapp
docker logs -f myapp        # 实时跟踪
docker logs --tail 100 myapp

# 查看容器状态
docker stats myapp

# 查看容器详细信息
docker inspect myapp

# 复制文件到容器
docker cp file.txt myapp:/path/to/

# 从容器复制文件
docker cp myapp:/path/to/file.txt ./

# 提交容器为镜像
docker commit myapp myapp:v1.0
```

**容器网络**:
```bash
# 查看网络
docker network ls

# 创建网络
docker network create mynetwork

# 运行容器并连接网络
docker run --network mynetwork nginx

# 查看网络详细信息
docker network inspect bridge

# 连接容器到网络
docker network connect mynetwork myapp

# 断开容器与网络的连接
docker network disconnect mynetwork myapp

# 删除网络
docker network rm mynetwork
```

**数据卷**:
```bash
# 创建数据卷
docker volume create myvolume

# 查看数据卷
docker volume ls

# 查看数据卷详细信息
docker volume inspect myvolume

# 运行容器并挂载数据卷
docker run -v myvolume:/data nginx

# 直接挂载主机目录
docker run -v /host/data:/data nginx

# 删除数据卷
docker volume rm myvolume
docker volume prune  # 删除未使用的卷
```

### Docker Compose

```bash
# 安装
sudo pacman -S docker-compose

# 创建 docker-compose.yml
cat > docker-compose.yml <<'EOF'
version: '3.8'

services:
  web:
    image: nginx:latest
    ports:
      - "8080:80"
    volumes:
      - ./html:/usr/share/nginx/html
    depends_on:
      - redis

  redis:
    image: redis:alpine
    ports:
      - "6379:6379"

  database:
    image: postgres:13
    environment:
      POSTGRES_USER: admin
      POSTGRES_PASSWORD: secret
      POSTGRES_DB: myapp
    volumes:
      - pgdata:/var/lib/postgresql/data

volumes:
  pgdata:
EOF

# 启动服务
docker-compose up -d

# 查看服务状态
docker-compose ps

# 查看日志
docker-compose logs -f web

# 执行命令
docker-compose exec web /bin/bash

# 停止服务
docker-compose down

# 停止并删除数据卷
docker-compose down -v

# 重建服务
docker-compose up -d --build

# 查看服务
docker-compose ps

# 扩展服务
docker-compose up -d --scale web=3
```

### Podman (Docker 替代品)

```bash
# 安装
sudo pacman -S podman

# 基本用法与 Docker 相似
podman --version
podman info

# 拉取镜像
podman pull nginx

# 运行容器
podman run -d -p 8080:80 --name web nginx

# 列出容器
podman ps

# 删除容器
podman rm web

# 列出镜像
podman images

# 构建镜像
podman build -t myapp:1.0 .

# 用户命名空间（无 root）
podman unshare  # 在用户命名空间中运行命令
```

**Podman vs Docker**:
- Podman: 无守护进程，rootless 更安全
- Docker: 生态更成熟，功能更全面
- 两者命令基本兼容（Docker → Podman 迁移容易）

---

## 虚拟化

### QEMU

```bash
# 安装
sudo pacman -S qemu

# 创建虚拟磁盘
qemu-img create -f qcow2 disk.qcow2 10G

# 安装系统
qemu-system-x86_64 \
    -enable-kvm \
    -m 2048 \
    -cpu host \
    -smp 4 \
    -drive if=virtio,file=disk.qcow2,format=qcow2 \
    -netdev user,id=net0 \
    -device virtio-net,netdev=net0 \
    -display sdl \
    -cdrom ubuntu.iso

# 运行已安装系统
qemu-system-x86_64 \
    -enable-kvm \
    -m 4096 \
    -cpu host \
    -smp 4 \
    -drive if=virtio,file=disk.qcow2,format=qcow2 \
    -netdev user,id=net0 \
    -device virtio-net,netdev=net0 \
    -display sdl

# 使用图形界面
qemu-system-x86_64 disk.qcow2

# 静默模式（无图形）
qemu-system-x86_64 -nographic disk.qcow2

# 快照
qemu-img snapshot -c initial disk.qcow2
qemu-img snapshot -l disk.qcow2

# 克隆镜像
qemu-img create -f qcow2 -b disk.qcow2 clone.qcow2

# 转换镜像格式
qemu-img convert -f qcow2 disk.qcow2 -O raw disk.raw
qemu-img convert -f qcow2 disk.qcow2 -O vmdk disk.vmdk
```

### libvirt / virsh

```bash
# 安装
sudo pacman -S libvirt qemu

# 启动 libvirt
sudo systemctl start libvirtd
sudo systemctl enable libvirtd

# 创建虚拟机
virsh define vm.xml
virsh create vm.xml

# 列出虚拟机
virsh list
virsh list --all

# 启动/停止虚拟机
virsh start vm-name
virsh shutdown vm-name
virsh destroy vm-name  # 强制关闭

# 挂起/恢复
virsh suspend vm-name
virsh resume vm-name

# 查看虚拟机信息
virsh dominfo vm-name
virsh vcpuinfo vm-name

# 编辑虚拟机配置
virsh edit vm-name

# 删除虚拟机
virsh undefine vm-name

# 监控资源使用
virsh cpu-stats vm-name
```

---

## 磁盘工具

### fdisk - 磁盘分区

```bash
# 查看磁盘
fdisk -l

# 交互式分区
sudo fdisk /dev/sdb

# 命令行操作（非交互）
sudo fdisk /dev/sdb << EOF
n
p
1

w
EOF

# 分区类型
# n - 新建分区
# p - 主分区
# e - 扩展分区
# l - 列出分区类型
# t - 更改分区类型
# w - 写入并退出
# q - 不保存退出
```

### parted - 高级分区

```bash
# 安装
sudo pacman -S parted

# 交互式分区
sudo parted /dev/sdb

# 命令行分区
sudo parted /dev/sdb mklabel gpt
sudo parted /dev/sdb mkpart primary 0% 100%
sudo parted /dev/sdb print
sudo parted /dev/sdb rm 1

# 设置文件系统
sudo mkfs.ext4 /dev/sdb1

# 调整文件系统大小
sudo resize2fs /dev/sdb1
```

### lsblk - 块设备

```bash
# 查看设备树
lsblk

# 显示文件系统
lsblk -f

# 显示权限
lsblk -m

# 显示从属关系
lsblk -p

# 显示 I/O 统计
lsblk -S

# 以 JSON 格式输出
lsblk -J
```

### dd - 磁盘复制

```bash
# 基本语法
dd if=input of=output bs=blocksize count=number

# 克隆磁盘
sudo dd if=/dev/sda of=/dev/sdb bs=64K status=progress

# 创建启动盘
sudo dd if=ubuntu.iso of=/dev/sdb bs=4M status=progress

# 备份 MBR
sudo dd if=/dev/sda of=mbr_backup.bin bs=512 count=1

# 恢复 MBR
sudo dd if=mbr_backup.bin of=/dev/sda bs=512 count=1

# 擦除磁盘（写入零）
sudo dd if=/dev/zero of=/dev/sdb bs=1M status=progress

# 随机数据（安全擦除）
sudo dd if=/dev/urandom of=/dev/sdb bs=1M status=progress

# 创建文件镜像
dd if=/dev/zero of=file.img bs=1G count=1

# 转换和复制并转换
dd if=/dev/sda of=disk.img bs=64K conv=noerror,sync

# 显示进度（需要新版 dd）
dd if=/dev/sda of=/dev/sdb bs=64K status=progress

# 监控 dd 进度
sudo kill -USR1 $(pgrep dd)
```

### ddrescue - 数据恢复

```bash
# 安装
sudo pacman -S ddrescue

# 复制数据（错误时继续）
ddrescue /dev/sda /dev/sdb mapfile.log

# 只读尝试（不写入）
ddrescue --read-only --verbose /dev/sda /dev/sdb mapfile.log

# 多次尝试
ddrescue --try-again /dev/sda /dev/sdb mapfile.log

# 查看日志
ddrescue --log-input /dev/sda mapfile.log | less
```

---

## 版本控制

### Git 基础（快速回顾）

```bash
# 初始化仓库
git init

# 克隆仓库
git clone https://github.com/user/repo.git

# 配置
git config --global user.name "Your Name"
git config --global user.email "email@example.com"
git config --global core.editor vim

# 基本工作流
git status
git add .
git commit -m "message"
git log --oneline --graph --all

# 分支操作
git branch
git checkout -b feature
git merge feature
git branch -d feature

# 远程操作
git remote -v
git push origin main
git pull origin main

# Git 高级技巧已在 [05-开发工具](../05-开发工具/) 详细讲解
```

### Git LFS - 大文件存储

```bash
# 安装
sudo pacman -S git-lfs

# 初始化
git lfs install

# 跟踪大文件
git lfs track "*.psd"
git lfs track "*.zip"
git lfs track "*.mov"

# 查看跟踪的文件
git lfs track

# 提交更改
git add .gitattributes
git add *.psd
git commit -m "Add large files"

# 推送
git push origin main

# 检查大文件使用情况
git lfs ls-files
```

### SVN (Subversion)

```bash
# 安装
sudo pacman -S subversion

# 检出仓库
svn checkout https://svn.example.com/repo/trunk repo

# 更新工作副本
svn update

# 添加文件
svn add file.txt
svn add *.php

# 提交更改
svn commit -m "message"

# 查看状态
svn status
svn status -u

# 查看日志
svn log
svn log -l 10  # 最近10条

# 查看差异
svn diff file.txt
svn diff -r 10:20 file.txt

# 解决冲突
svn resolve file.txt

# 切换分支
svn switch https://svn.example.com/repo/branches/branch-name

# 创建分支
svn copy https://svn.example.com/repo/trunk \
        https://svn.example.com/repo/branches/branch-name \
        -m "Create branch"

# 删除文件
svn delete file.txt
svn commit -m "Delete file"
```

### CVS (已过时，简要说明)

```bash
# 概念: 集中式版本控制（过时）
# 现在使用 Git 而不是 CVS

# 设置 CVSROOT
export CVSROOT=:pserver:user@server:/path/to/repo
cvs login

# 导出项目
cvs checkout project

# 更新
cd project
cvs update

# 添加文件
cvs add file.txt
cvs commit -m "message"

# 查看状态
cvs status
cvs diff
```

---

## 安全工具

### GPG - 加密和签名

```bash
# 安装
sudo pacman -S gnupg

# 生成密钥对
gpg --full-generate-key

# 查看密钥
gpg --list-keys
gpg --list-secret-keys

# 导出公钥
gpg --armor --export KEY_ID

# 导出私钥（危险！）
gpg --armor --export-secret-keys KEY_ID

# 导入密钥
gpg --import public_key.asc

# 加密文件
gpg --encrypt --recipient recipient@example.com file.txt
# 生成 file.txt.gpg

# 解密文件
gpg --decrypt file.txt.gpg > file.txt

# 对称加密（密码保护）
gpg --symmetric file.txt

# 签名文件
gpg --sign file.txt  # 生成 file.txt.gpg

# 签名文件（分离签名）
gpg --detach-sig file.txt  # 生成 file.txt.sig

# 验证签名
gpg --verify file.txt.sig

# 签名并加密
gpg --sign --encrypt --recipient recipient@example.com file.txt

# 编辑密钥
gpg --edit-key KEY_ID
# 子命令:
#   help, quit, save, fpr, list, uid, trust, sign, adduid, deluid,
#   addkey, delkey, addrevoker, expires, passwd

# 生成撤销证书
gpg --gen-revoke KEY_ID > revoke_cert.asc

# 刷新密钥
gpg --keyserver keys.openpgp.org --recv-keys KEY_ID
gpg --keyserver keys.openpgp.org --send-keys KEY_ID

# 清理密钥服务器上的密钥
gpg --keyserver keys.openpgp.org --send-keys KEY_ID

# 清理私钥
gpg --delete-secret-keys KEY_ID

# 清理公钥
gpg --delete-keys KEY_ID

# 修改信任度
gpg --edit-key KEY_ID
# 输入 trust, 选择信任级别

# 密钥服务器
gpg --keyserver hkps://keys.openpgp.org --recv-keys KEY_ID
gpg --keyserver hkps://keys.openpgp.org --send-keys KEY_ID

# 列出密钥服务器
gpg --keyserver-options verbose --keyserver keys.openpgp.org --send-key KEY_ID

# 清理已删除的密钥
gpg --expire-keys KEY_ID

# 清理密钥环
gpg --list-keys --keyid-format LONG
gpg --list-sigs
gpg --check-sigs
```

### ssh - 安全远程访问

```bash
# 基本语法
ssh user@hostname

# 指定端口
ssh -p 2222 user@hostname

# 指定私钥
ssh -i ~/.ssh/id_rsa_custom user@hostname

# X11 转发
ssh -X user@hostname

# 压缩传输
ssh -C user@hostname

# 静默模式
ssh -q user@hostname

# 详细输出
ssh -v user@hostname
ssh -vv user@hostname
ssh -vvv user@hostname

# 保持连接（心跳）
ssh -o ServerAliveInterval=60 user@hostname

# 通过中间主机连接
ssh -J jump-host target-host

# 本地端口转发
ssh -L 8080:target-host:80 user@jump-host

# 远程端口转发
ssh -R 9090:localhost:80 user@remote-host

# 动态端口转发（SOCKS代理）
ssh -D 1080 user@remote-host

# 在后台运行
ssh -f -N -L 8080:target:80 user@jump-host

# 执行命令
ssh user@hostname "ls -la"

# X11 转发
ssh -Y user@hostname

# 从配置文件加载
# ~/.ssh/config
cat >> ~/.ssh/config <<'EOF'
Host myserver
    HostName server.example.com
    User john
    Port 2222
    IdentityFile ~/.ssh/id_rsa_custom
    ForwardAgent yes
EOF

# 然后直接使用
ssh myserver
```

**SSH 密钥管理**:
```bash
# 生成密钥对
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
ssh-keygen -t ed25519 -C "your_email@example.com"

# 添加私钥到 ssh-agent
ssh-add ~/.ssh/id_rsa

# 查看已添加的密钥
ssh-add -l

# 复制公钥到远程服务器
ssh-copy-id user@hostname
# 手动复制
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys

# 权限设置
chmod 700 ~/.ssh
chmod 600 ~/.ssh/authorized_keys
chmod 600 ~/.ssh/id_rsa
chmod 644 ~/.ssh/id_rsa.pub
```

---

## 加密和哈希

### OpenSSL - 加密和证书

```bash
# 生成随机数据
openssl rand -base64 32  # 32字节base64编码
openssl rand -hex 16     # 16字节十六进制

# 计算哈希值
echo "hello" | openssl dgst -sha256
openssl dgst -md5 file.txt
openssl dgst -sha1 file.txt
openssl dgst -sha512 file.txt

# Base64 编码/解码
echo "hello" | openssl base64
echo "aGVsbG8=" | openssl base64 -d

# 对称加密
# AES-256-CBC 加密
openssl enc -aes-256-cbc -in file.txt -out file.enc
# 提示输入密码

# 解密
openssl enc -aes-256-cbc -d -in file.enc -out file.txt

# 使用口令
openssl enc -aes-256-cbc -in file.txt -out file.enc -pass pass:yourpassword

# 指定盐值
openssl enc -aes-256-cbc -salt -in file.txt -out file.enc

# 数字签名
openssl dgst -sha256 -sign private.key -out signature.txt file.txt

# 验证签名
openssl dgst -sha256 -verify public.key -signature signature.txt file.txt

# RSA 密钥对
openssl genrsa -out private.key 2048
openssl rsa -in private.key -pubout -out public.key
openssl rsa -in private.key -text -noout

# 查看证书
openssl x509 -in certificate.crt -text -noout

# 查看证书有效期
openssl x509 -in certificate.crt -noout -dates

# 检查证书
openssl verify -CAfile ca.crt certificate.crt

# 生成证书签名请求（CSR）
openssl req -new -key private.key -out cert.csr

# 自签名证书
openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout key.pem

# 从 CSR 生成证书
openssl x509 -req -in cert.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out cert.pem -days 365

# 测试 SSL 连接
openssl s_client -connect google.com:443
openssl s_client -connect mail.example.com:465

# 检查证书链
openssl s_client -connect example.com:443 -showcerts

# 检查协议支持
openssl s_client -connect example.com:443 -tls1_2
openssl s_client -connect example.com:443 -alpn http/1.1

# 解码证书
openssl x509 -in certificate.crt -text -noout | less
```

### Hash 值验证

```bash
# 计算多个哈希值
echo "计算 SHA256: $(sha256sum file.txt)"
echo "计算 SHA512: $(sha512sum file.txt)"
echo "计算 MD5: $(md5sum file.txt)"

# 批量验证
sha256sum -c checksums.txt

# 生成校验和文件
find /path/to/files -type f -exec sha256sum {} \; > checksums.txt

# 验证完整性
sha256sum -c checksums.txt --quiet
```

### 加密文件系统

```bash
# 使用 LUKS 加密
sudo cryptsetup luksFormat /dev/sdb

# 打开加密分区
sudo cryptsetup luksOpen /dev/sdb encrypted_disk

# 创建文件系统
sudo mkfs.ext4 /dev/mapper/encrypted_disk

# 挂载
sudo mount /dev/mapper/encrypted_disk /mnt/encrypted

# 卸载并关闭
sudo umount /mnt/encrypted
sudo cryptsetup luksClose encrypted_disk

# 创建文件作为加密容器
dd if=/dev/zero of=container.img bs=1M count=100
sudo losetup /dev/loop0 container.img
sudo cryptsetup luksFormat /dev/loop0
sudo cryptsetup luksOpen /dev/loop0 encrypted_loop
sudo mkfs.ext4 /dev/mapper/encrypted_loop
sudo mount /dev/mapper/encrypted_loop /mnt/encrypted
```

---

## 版本控制系统对比

### Git vs SVN

| 特性 | Git | SVN |
|------|-----|-----|
| **分布式** | 是 | 否 |
| **分支** | 轻量级，易创建/合并 | 重量级 |
| **离线工作** | 完全支持 | 需要网络 |
| **速度** | 快 | 较慢 |
| **存储空间** | 高效 | 效率较低 |
| **学习曲线** | 陡峭 | 平缓 |
| **权限控制** | 复杂 | 简单 |
| **适用团队** | 大团队，分布式 | 小团队，集中式 |
| **操作** | `git clone` | `svn checkout` |
| **更新** | `git pull` | `svn update` |
| **提交** | `git commit` | `svn commit` |
| **推送** | `git push` | `svn commit` (直接推送到服务器) |

### 选择建议

**选择 Git 如果**:
- 团队分散在不同地区
- 需要频繁创建和合并分支
- 离线工作
- 需要良好的性能

**选择 SVN 如果**:
- 团队规模较小，集中办公
- 有复杂的权限需求
- 需要简单的操作模式
- 已有 SVN 基础设施

---

## 其他工具

### curl - HTTP 客户端

```bash
# 已在 [04-网络工具](../04-网络工具/) 详细讲解

# 常见用法回顾
curl -I https://example.com    # 查看 HTTP 头
curl -L https://example.com    # 跟随重定向
curl -s https://api.example.com/data.json | jq  # 配合 jq 使用
```

### jq - JSON 处理

```bash
# 安装
sudo pacman -S jq

# 基本语法
echo '{"name":"John","age":30}' | jq '.'

# 提取字段
echo '{"name":"John","age":30}' | jq '.name'

# 格式化 JSON
cat data.json | jq '.'

# 提取数组元素
echo '[1,2,3]' | jq '.[0]'

# 过滤
cat data.json | jq '.users[] | select(.age > 18)'

# 映射
echo '[1,2,3]' | jq '.[] * 2'

# 组合
echo '{"user":{"name":"John"}}' | jq '.user.name'

# 数组操作
echo '[1,2,3]' | jq 'map(. * 2)'
```

### ack / rg - 代码搜索

```bash
# 安装
sudo pacman -S ripgrep

# 基本搜索
rg "pattern" ./

# 忽略大小写
rg -i "pattern" ./

# 只搜索特定文件类型
rg "pattern" --type py
rg "pattern" --type js --type ts

# 排除文件
rg "pattern" -g '!node_modules/'

# 显示行号
rg -n "pattern" ./

# 统计匹配
rg -c "pattern" ./

# 只显示文件名
rg -l "pattern" ./

# 只显示匹配部分
rg -o "pattern" ./

# 多模式
rg -e "pattern1" -e "pattern2" ./

# 显示上下文
rg -A 2 -B 2 "pattern" ./  # 显示前后2行

# 递归深度
rg --max-depth 3 "pattern" ./

# 搜索压缩文件
rg "pattern" -z ./

# 输出到文件
rg "pattern" ./ -o matches.txt
```

### tmux - 终端复用器

```bash
# 安装
sudo pacman -S tmux

# 启动 tmux
tmux

# 从 tmux 退出（不关闭窗口）
# 按 Ctrl+b 然后按 d

# 附加到会话
tmux attach

# 列出会话
tmux ls

# 新建命名会话
tmux new -s session_name

# 杀死会话
tmux kill-session -t session_name

# 窗口操作
# Ctrl+b c  新建窗口
# Ctrl+b w  列出窗口
# Ctrl+b n  下一个窗口
# Ctrl+b p  上一个窗口
# Ctrl+b 数字  切换到指定窗口
# Ctrl+b ,  重命名窗口
# Ctrl+b &  关闭窗口

# 面板操作
# Ctrl+b %  垂直分割
# Ctrl+b "  水平分割
# Ctrl+b 箭头键  切换面板
# Ctrl+b o  下一个面板
# Ctrl+b q  显示面板编号
# Ctrl+b {  移动面板到前面
# Ctrl+b }  移动面板到后面
# Ctrl+b !  将面板转成窗口
# Ctrl+b x  关闭面板
# Ctrl+b z  全屏/还原面板
# Ctrl+b space  重新排列面板

# 复制模式
# Ctrl+b [  进入复制模式
# q          退出复制模式

# 滚动
# Ctrl+b [ 然后使用方向键滚动

# 保存会话状态
tmux save-buffer /path/to/buffer.txt
```

### screen - 传统终端复用器

```bash
# 安装
sudo pacman -S screen

# 启动
screen

# 分离会话
# 按 Ctrl+a 然后 d

# 重新附加
screen -r

# 列出会话
screen -ls

# 新建命名会话
screen -S session_name

# 杀死会话
screen -S session_name -X quit

# 命令（在 screen 内）
# Ctrl+a c  新建窗口
# Ctrl+a "  列出窗口
# Ctrl+a 0-9  切换窗口
# Ctrl+a A  重命名窗口
# Ctrl+a k  杀死窗口
# Ctrl+a \  杀死所有窗口
# Ctrl+a [  进入复制模式
```

### 实用工具组合

```bash
# 监控日志
tail -f /var/log/syslog | grep --line-buffered ERROR

# 实时监控进程
watch -n 1 'ps aux | grep nginx'

# 查看大文件
du -sh /* | sort -hr

# 查找最近修改的文件
find /path -type f -mtime -1 -ls

# 统计代码行数
find . -name "*.py" -exec wc -l {} + | tail -1

# 批量重命名
ls *.txt | sed 's/\(.*\)\.txt/mv \1.txt \1.bak/' | bash

# 进程监控和重启
while true; do
    if ! pgrep myapp > /dev/null; then
        myapp &
    fi
    sleep 10
done

# 定时备份
#!/bin/bash
rsync -az --delete /data /backup/$(date +%Y%m%d)/
find /backup -type d -mtime +7 -exec rm -rf {} +

# 清理临时文件
find /tmp -type f -mtime +7 -delete

# 磁盘使用分析
du -h --max-depth=1 / | sort -hr | head -10
```

---

## 相关资源

- [Docker 官方文档](https://docs.docker.com/)
- [Podman 文档](https://docs.podman.io/)
- [libvirt 文档](https://libvirt.org/)
- [OpenSSL 文档](https://www.openssl.org/docs/)
- [tar 手册](https://www.gnu.org/software/tar/manual/)
- 上一章: [08-Shell编程](../08-Shell编程/)
- 下一章: [10-发行版差异](../10-发行版差异/)
