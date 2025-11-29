# 10 - Linux 发行版差异

> Arch Linux vs 其他主流发行版的命令和配置差异

## 目录

- [包管理器对比](#包管理器对比)
- [系统服务管理](#系统服务管理)
- [配置文件位置](#配置文件位置)
- [系统初始化](#系统初始化)
- [网络配置](#网络配置)
- [防火墙管理](#防火墙管理)

---

## 包管理器对比

### 基本操作对比

| 操作 | Arch (pacman) | Debian/Ubuntu (apt) | RedHat/Fedora (dnf) |
|------|---------------|---------------------|---------------------|
| **更新软件列表** | `sudo pacman -Sy` | `sudo apt update` | `sudo dnf check-update` |
| **升级所有软件** | `sudo pacman -Syu` | `sudo apt upgrade` | `sudo dnf upgrade` |
| **安装软件** | `sudo pacman -S pkg` | `sudo apt install pkg` | `sudo dnf install pkg` |
| **删除软件** | `sudo pacman -R pkg` | `sudo apt remove pkg` | `sudo dnf remove pkg` |
| **删除软件+依赖** | `sudo pacman -Rs pkg` | `sudo apt autoremove pkg` | `sudo dnf autoremove pkg` |
| **搜索软件** | `pacman -Ss keyword` | `apt search keyword` | `dnf search keyword` |
| **查看软件信息** | `pacman -Si pkg` | `apt show pkg` | `dnf info pkg` |
| **列出已安装** | `pacman -Q` | `apt list --installed` | `dnf list installed` |
| **清理缓存** | `sudo pacman -Sc` | `sudo apt autoclean` | `sudo dnf clean all` |
| **查找文件所属包** | `pacman -Qo /path` | `dpkg -S /path` | `dnf provides /path` |

### 包格式

| 发行版 | 包格式 | 扩展名 |
|--------|--------|--------|
| Arch | pacman | `.pkg.tar.zst` |
| Debian/Ubuntu | dpkg | `.deb` |
| RedHat/Fedora | rpm | `.rpm` |

### 仓库配置

**Arch Linux**:
```bash
# /etc/pacman.conf
[core]
Include = /etc/pacman.d/mirrorlist

[extra]
Include = /etc/pacman.d/mirrorlist

[community]
Include = /etc/pacman.d/mirrorlist

# 镜像列表
# /etc/pacman.d/mirrorlist
Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch
```

**Debian/Ubuntu**:
```bash
# /etc/apt/sources.list
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted
```

**Fedora**:
```bash
# /etc/yum.repos.d/fedora.repo
[fedora]
name=Fedora $releasever - $basearch
baseurl=http://mirrors.tuna.tsinghua.edu.cn/fedora/releases/$releasever/Everything/$basearch/os/
```

### 常用命令转换

**Arch → Debian/Ubuntu**:
```bash
# Arch
sudo pacman -Syu              # Debian: sudo apt update && sudo apt upgrade
sudo pacman -S package        # Debian: sudo apt install package
sudo pacman -Rs package       # Debian: sudo apt remove --purge package
pacman -Ss keyword            # Debian: apt search keyword
pacman -Ql package            # Debian: dpkg -L package
```

**Arch → RedHat/Fedora**:
```bash
# Arch
sudo pacman -Syu              # Fedora: sudo dnf upgrade
sudo pacman -S package        # Fedora: sudo dnf install package
sudo pacman -Rs package       # Fedora: sudo dnf remove package
pacman -Ss keyword            # Fedora: dnf search keyword
```

---

## 系统服务管理

### systemd 命令（通用）

所有现代发行版（Arch, Debian, Ubuntu, Fedora）都使用 **systemd**,命令基本相同:

```bash
# 启动服务
sudo systemctl start service_name

# 停止服务
sudo systemctl stop service_name

# 重启服务
sudo systemctl restart service_name

# 查看状态
sudo systemctl status service_name

# 开机自启
sudo systemctl enable service_name

# 禁止自启
sudo systemctl disable service_name

# 启用并立即启动
sudo systemctl enable --now service_name

# 查看所有服务
systemctl list-units --type=service

# 查看失败的服务
systemctl --failed

# 重载配置
sudo systemctl daemon-reload
```

### 旧系统的区别（已过时）

**老版本 Debian/Ubuntu（SysVinit）**:
```bash
sudo service apache2 start      # 现在: systemctl start apache2
sudo update-rc.d apache2 enable # 现在: systemctl enable apache2
```

**老版本 RedHat/CentOS（SysVinit）**:
```bash
sudo service httpd start        # 现在: systemctl start httpd
sudo chkconfig httpd on         # 现在: systemctl enable httpd
```

### 服务名称差异

| 服务 | Arch | Debian/Ubuntu | RedHat/Fedora |
|------|------|---------------|---------------|
| SSH | `sshd` | `ssh` | `sshd` |
| Apache | `httpd` | `apache2` | `httpd` |
| 网络 | `NetworkManager` | `NetworkManager` | `NetworkManager` |
| 防火墙 | `iptables` / `firewalld` | `ufw` | `firewalld` |
| 时间同步 | `systemd-timesyncd` | `systemd-timesyncd` | `chronyd` |

---

## 配置文件位置

### 网络配置

**Arch Linux**:
```bash
# systemd-networkd
/etc/systemd/network/

# NetworkManager
/etc/NetworkManager/system-connections/

# 静态IP示例
# /etc/systemd/network/20-wired.network
[Match]
Name=eth0

[Network]
Address=192.168.1.100/24
Gateway=192.168.1.1
DNS=8.8.8.8
```

**Debian/Ubuntu**:
```bash
# 旧版: /etc/network/interfaces
# 新版: /etc/netplan/*.yaml

# Netplan示例
network:
  version: 2
  ethernets:
    eth0:
      addresses:
        - 192.168.1.100/24
      gateway4: 192.168.1.1
      nameservers:
        addresses: [8.8.8.8]
```

**RedHat/Fedora**:
```bash
# /etc/sysconfig/network-scripts/ifcfg-eth0
TYPE=Ethernet
BOOTPROTO=static
NAME=eth0
DEVICE=eth0
ONBOOT=yes
IPADDR=192.168.1.100
NETMASK=255.255.255.0
GATEWAY=192.168.1.1
DNS1=8.8.8.8
```

### 主机名

| 发行版 | 配置文件 | 命令 |
|--------|----------|------|
| Arch | `/etc/hostname` | `hostnamectl set-hostname name` |
| Debian | `/etc/hostname` | `hostnamectl set-hostname name` |
| RedHat | `/etc/hostname` | `hostnamectl set-hostname name` |

### DNS 配置

**所有发行版**:
```bash
# /etc/resolv.conf
nameserver 8.8.8.8
nameserver 8.8.4.4
```

### 用户和组

**所有发行版相同**:
```bash
/etc/passwd     # 用户信息
/etc/shadow     # 密码信息
/etc/group      # 组信息
/etc/sudoers    # sudo权限
```

### 时区

**所有发行版**:
```bash
# 查看时区
timedatectl

# 设置时区
sudo timedatectl set-timezone Asia/Shanghai

# 配置文件
/etc/localtime -> /usr/share/zoneinfo/Asia/Shanghai
```

---

## 系统初始化

### 启动加载器

**Arch Linux**:
```bash
# 通常使用 GRUB 或 systemd-boot

# GRUB配置
/boot/grub/grub.cfg
sudo grub-mkconfig -o /boot/grub/grub.cfg

# systemd-boot
/boot/loader/entries/
bootctl update
```

**Debian/Ubuntu**:
```bash
# GRUB配置
/boot/grub/grub.cfg
sudo update-grub
```

**Fedora**:
```bash
# GRUB2配置
/boot/grub2/grub.cfg
sudo grub2-mkconfig -o /boot/grub2/grub.cfg
```

### 内核参数

**所有发行版**:
```bash
# 临时修改（重启失效）
sudo sysctl -w kernel.parameter=value

# 永久修改
# /etc/sysctl.conf 或 /etc/sysctl.d/*.conf
kernel.parameter = value

# 应用更改
sudo sysctl -p
```

---

## 网络配置

### 网络管理工具

| 发行版 | 默认工具 |
|--------|----------|
| Arch | `systemd-networkd` 或 `NetworkManager` |
| Debian/Ubuntu | `NetworkManager` 或 `netplan` |
| Fedora | `NetworkManager` |

### 使用 NetworkManager

**所有发行版通用**:
```bash
# 命令行工具
nmcli device status              # 查看设备
nmcli connection show            # 查看连接
nmcli device wifi list           # 扫描WiFi
nmcli device wifi connect SSID password PASSWORD

# TUI工具
nmtui
```

### 配置静态IP

**Arch (systemd-networkd)**:
```bash
# /etc/systemd/network/20-wired.network
[Match]
Name=eth0

[Network]
Address=192.168.1.100/24
Gateway=192.168.1.1
DNS=8.8.8.8

sudo systemctl restart systemd-networkd
```

**Debian/Ubuntu (netplan)**:
```bash
# /etc/netplan/01-netcfg.yaml
network:
  version: 2
  ethernets:
    eth0:
      addresses: [192.168.1.100/24]
      gateway4: 192.168.1.1
      nameservers:
        addresses: [8.8.8.8]

sudo netplan apply
```

**Fedora (NetworkManager)**:
```bash
# /etc/sysconfig/network-scripts/ifcfg-eth0
# 或使用 nmcli

nmcli con mod eth0 ipv4.addresses 192.168.1.100/24
nmcli con mod eth0 ipv4.gateway 192.168.1.1
nmcli con mod eth0 ipv4.dns "8.8.8.8"
nmcli con mod eth0 ipv4.method manual
nmcli con up eth0
```

---

## 防火墙管理

### Arch Linux

**iptables**:
```bash
# 安装
sudo pacman -S iptables

# 查看规则
sudo iptables -L

# 允许端口
sudo iptables -A INPUT -p tcp --dport 80 -j ACCEPT

# 保存规则
sudo iptables-save > /etc/iptables/iptables.rules
```

**firewalld**:
```bash
# 安装
sudo pacman -S firewalld
sudo systemctl enable --now firewalld

# 允许服务
sudo firewall-cmd --add-service=http --permanent
sudo firewall-cmd --reload
```

### Debian/Ubuntu

**ufw (简单)**:
```bash
# 安装和启用
sudo apt install ufw
sudo ufw enable

# 允许端口
sudo ufw allow 80/tcp
sudo ufw allow ssh

# 查看状态
sudo ufw status
```

**iptables (高级)**:
```bash
sudo apt install iptables-persistent
sudo iptables -A INPUT -p tcp --dport 80 -j ACCEPT
sudo netfilter-persistent save
```

### Fedora

**firewalld (默认)**:
```bash
# 查看状态
sudo firewall-cmd --state

# 允许服务
sudo firewall-cmd --add-service=http --permanent
sudo firewall-cmd --add-port=8080/tcp --permanent
sudo firewall-cmd --reload

# 查看规则
sudo firewall-cmd --list-all
```

---

## 常用软件名称差异

| 功能 | Arch | Debian/Ubuntu | Fedora |
|------|------|---------------|--------|
| Python 包管理 | `python-pip` | `python3-pip` | `python3-pip` |
| 构建工具 | `base-devel` | `build-essential` | `@development-tools` |
| 文本编辑器 | `vim` | `vim` | `vim-enhanced` |
| C编译器 | `gcc` | `gcc` | `gcc` |
| Java | `jdk-openjdk` | `openjdk-11-jdk` | `java-11-openjdk` |
| Node.js | `nodejs npm` | `nodejs npm` | `nodejs npm` |

---

## 系统管理命令对比

### 查看系统信息

**所有发行版通用**:
```bash
uname -a                # 内核信息
hostnamectl             # 主机信息
lsb_release -a          # 发行版信息（需安装）
cat /etc/os-release     # 发行版信息
```

### Arch 独有命令

```bash
# 包数据库
/var/lib/pacman/        # 包数据库
/var/cache/pacman/pkg/  # 包缓存

# Arch 构建系统
makepkg                 # 构建包
PKGBUILD                # 构建脚本

# AUR
yay / paru              # AUR辅助工具
```

### Debian/Ubuntu 独有

```bash
# 包管理
dpkg                    # 底层包管理
apt-get                 # 旧版本apt
apt-cache               # 包缓存查询

# 软件源
add-apt-repository      # 添加PPA（仅Ubuntu）

# 发行版升级
do-release-upgrade      # Ubuntu版本升级
```

### Fedora/RedHat 独有

```bash
# 包管理
rpm                     # 底层包管理
yum                     # 旧版本包管理器（CentOS 7）

# SELinux
sestatus                # SELinux状态
setenforce              # 设置模式
```

---

## 目录结构差异

### 日志位置

| 发行版 | 系统日志 |
|--------|----------|
| 所有（systemd）| `journalctl` 或 `/var/log/journal/` |
| Arch | `/var/log/` |
| Debian | `/var/log/` |
| Fedora | `/var/log/` |

### 文档位置

| 发行版 | 文档 |
|--------|------|
| Arch | `/usr/share/doc/` |
| Debian | `/usr/share/doc/` |
| Fedora | `/usr/share/doc/` |

---

## 哲学差异

### Arch Linux

- **滚动更新**: 始终最新软件
- **KISS原则**: Keep It Simple, Stupid
- **用户自主**: 高度可定制,需要手动配置
- **极简主义**: 只安装需要的
- **文档**: 优秀的 Arch Wiki

### Debian/Ubuntu

- **稳定性**: Debian稳定分支非常保守
- **易用性**: Ubuntu桌面开箱即用
- **长期支持**: LTS版本5年支持
- **软件仓库**: 庞大的软件库

### Fedora

- **前沿技术**: 新技术快速采用
- **RedHat**: RHEL的上游
- **SELinux**: 默认启用增强安全
- **创新**: 技术预览和实验

---

## 选择建议

**选择 Arch 如果你**:
- 想要最新软件
- 喜欢自己配置系统
- 需要AUR的大量软件
- 享受学习过程

**选择 Debian/Ubuntu 如果你**:
- 需要稳定的服务器
- 想要开箱即用
- 需要企业支持（Ubuntu）
- 新手入门

**选择 Fedora 如果你**:
- 需要最新技术但又相对稳定
- 计划使用 RHEL
- 需要强安全性（SELinux）
- 开发者环境

---

## 相关资源

- [Arch vs Debian](https://wiki.archlinux.org/title/Arch_compared_to_other_distributions)
- [包管理器对照表](https://wiki.archlinux.org/title/Pacman/Rosetta)
- [systemd文档](https://www.freedesktop.org/software/systemd/man/)
- [DistroWatch](https://distrowatch.com/) - 发行版信息
