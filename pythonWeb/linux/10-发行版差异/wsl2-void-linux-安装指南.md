# WSL2 中安装 Void Linux 完整指南

> 在 Windows Subsystem for Linux 2 中安装和配置 Void Linux

## 目录

- [前置要求](#前置要求)
- [方法一：使用官方 Rootfs 手动安装（推荐）](#方法一使用官方-rootfs-手动安装推荐)
- [方法二：使用第三方工具](#方法二使用第三方工具)
- [方法三：从 Docker 镜像导入](#方法三从-docker-镜像导入)
- [安装后配置](#安装后配置)
- [常见问题](#常见问题)
- [WSL2 特殊优化](#wsl2-特殊优化)

---

## 前置要求

### 1. 启用 WSL2

在 **PowerShell（管理员）** 中执行：

```powershell
# 启用 WSL 功能
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart

# 启用虚拟机平台
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

# 重启计算机
Restart-Computer
```

重启后，继续在 **PowerShell（管理员）** 中：

```powershell
# 设置 WSL2 为默认版本
wsl --set-default-version 2

# 更新 WSL 内核
wsl --update
```

### 2. 验证 WSL2 状态

```powershell
# 查看 WSL 版本
wsl --status

# 列出已安装的发行版
wsl --list --verbose
```

---

## 方法一：使用官方 Rootfs 手动安装（推荐）

这是最干净、最官方的安装方法。 

### 步骤 1: 下载 Void Linux Rootfs

访问 Void Linux 官方下载页面或使用镜像站点：

**官方下载地址**:
- https://repo-default.voidlinux.org/live/current/

**镜像站点**:
- 清华镜像: https://mirrors.tuna.tsinghua.edu.cn/voidlinux/live/current/
- 上海交大: https://mirror.sjtu.edu.cn/voidlinux/live/current/

选择合适的 rootfs 文件：

| 文件名 | 说明 | 推荐场景 |
|--------|------|----------|
| `void-x86_64-ROOTFS-*.tar.xz` | glibc 版本 | **通用，推荐** |
| `void-x86_64-musl-ROOTFS-*.tar.xz` | musl 版本 | 轻量、容器 |

使用 PowerShell 下载：

```powershell
# 创建下载目录
New-Item -ItemType Directory -Force -Path C:\wsl-distros

# 下载 glibc 版本（推荐）
# 使用清华镜像
$url = "https://mirrors.tuna.tsinghua.edu.cn/voidlinux/live/current/void-x86_64-ROOTFS-20230628.tar.xz"
$output = "C:\wsl-distros\void-rootfs.tar.xz"

# 下载
Invoke-WebRequest -Uri $url -OutFile $output

# 或使用 curl（Windows 10 1803+）
curl -L $url -o $output
```

### 步骤 2: 处理 Rootfs 文件

⚠️ **重要警告：不要在 Windows 上直接解压 Linux rootfs！**

**为什么会出现同步文件冲突？**

1. **符号链接问题**：Linux rootfs 包含大量符号链接，Windows 工具会将其当作普通文件处理，导致文件名冲突
2. **文件系统不兼容**：Linux 特有的文件权限、特殊字符、大小写敏感等在 Windows 上无法正确处理
3. **OneDrive/云同步冲突**：如果目录在同步文件夹中，同步软件会尝试上传造成锁定和冲突
4. **权限丢失**：解压后文件权限和所有者信息会丢失

---

**✅ 推荐方法：直接导入 .tar.xz（无需解压）**

**WSL 2.0+** 直接支持导入 `.tar.xz` 格式，这是**最简单、最安全**的方法！

```powershell
# 创建安装目录
New-Item -ItemType Directory -Force -Path C:\wsl-distros\VoidLinux

# 直接导入 tar.xz（推荐！）
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar.xz --version 2

# 验证安装
wsl --list --verbose
```

**跳过步骤 3**，直接进入步骤 4！

---

**如果 WSL 版本较旧，需要先解压，请使用以下方法：**

**方法 A: 在 WSL 内解压（推荐）**

如果已经有其他 WSL 发行版（如 Ubuntu）：

```powershell
# 启动已有的 WSL
wsl

# 在 WSL 中解压
cd /mnt/c/wsl-distros
xz -d void-rootfs.tar.xz
# 得到 void-rootfs.tar

exit
```

**方法 B: 使用 7-Zip（谨慎）**

⚠️ **注意事项**：
- 解压目录**不能**在 OneDrive、Dropbox、Google Drive 等同步文件夹中
- 解压目录**不能**在 WSL 可访问路径（如 `\\wsl$\` 下）
- 建议使用独立的本地目录（如 `C:\wsl-distros`）

```powershell
# 安装 7-Zip
# 下载：https://www.7-zip.org/

# 使用命令行解压
cd C:\wsl-distros
"C:\Program Files\7-Zip\7z.exe" x void-rootfs.tar.xz

# 或右键点击 → 7-Zip → 提取到此处
```

**方法 C: 使用 PowerShell + 7-Zip CLI**

```powershell
# 安装 7-Zip 命令行版
# 使用 Chocolatey
choco install 7zip.commandline

# 或使用 Scoop
scoop install 7zip

# 解压
cd C:\wsl-distros
7z x void-rootfs.tar.xz
```

### 步骤 3: 导入到 WSL2（仅当使用 .tar 文件时）
ｄｓａｓａｓｄａｓａｄｓａｄ
⚠️ **如果已使用推荐方法直接导入 .tar.xz，请跳过此步骤！**

如果已在步骤 2 中解压得到 `.tar` 文件，在 **PowerShell** 中：

```powershell
# 创建安装目录
New-Item -ItemType Directory -Force -Path C:\wsl-distros\VoidLinux
                    
# 导入 rootfs
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar --version 2

# 验证安装
wsl --list --verbose
```

输出示例：
```
  NAME         STATE           VERSION
* VoidLinux    Stopped         2
  Ubuntu       Running         2
```

### 步骤 4: 启动 Void Linux

```powershell
# 启动 Void Linux
wsl -d VoidLinux

# 或设为默认发行版后直接启动
wsl --set-default VoidLinux
wsl
```

### 步骤 5: 初始配置

进入 Void Linux 后：

```bash
# 1. 更新系统
xbps-install -Su

# 2. 安装基础工具
xbps-install -S base-devel vim git curl wget

# 3. 创建普通用户
useradd -m -G wheel -s /bin/bash yourusername
passwd yourusername

# 4. 配置 sudo
xbps-install -S sudo
visudo
# 取消注释: %wheel ALL=(ALL) ALL

# 5. 退出
exit
```

### 步骤 6: 设置默认用户

在 **PowerShell** 中创建 `/etc/wsl.conf`：

```powershell
wsl -d VoidLinux
```

在 Void Linux 中：

```bash
# 创建 wsl.conf
cat > /etc/wsl.conf << 'EOF'
[user]
default=yourusername

[boot]
systemd=false

[network]
generateResolvConf=true
generateHosts=true
EOF

exit
```

重启 WSL：

```powershell
# 关闭 VoidLinux
wsl --terminate VoidLinux

# 或关闭所有 WSL
wsl --shutdown

# 重新启动
wsl -d VoidLinux
```

---

## 方法二：使用第三方工具

### LxRunOffline（已过时，不推荐）

LxRunOffline 项目已不再维护，不推荐使用。

### VoidWSL（推荐第三方方案）

有些开发者维护了 Void Linux 的 WSL 安装包。

**GitHub 搜索**:
```powershell
# 在 GitHub 搜索 "void wsl" 或 "void linux wsl2"
# 例如: https://github.com/search?q=void+linux+wsl
```

**手动安装第三方包**:

1. 下载预构建的 `.zip` 或 `.appx` 包
2. 解压到指定目录
3. 运行安装脚本或直接使用 `wsl --import` 导入

---

## 方法三：从 Docker 镜像导入

使用 Docker Desktop 或现有 WSL 发行版。

### 在已有的 WSL 中操作

```bash
# 1. 启动已有的 WSL（如 Ubuntu）
wsl

# 2. 安装 Docker（如果没有）
curl -fsSL https://get.docker.com | sh

# 3. 导出 Void Linux 镜像
docker pull voidlinux/voidlinux
docker create --name void-temp voidlinux/voidlinux
docker export void-temp > /mnt/c/wsl-distros/void-docker.tar
docker rm void-temp

exit
```

在 **PowerShell** 中导入：

```powershell
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-docker.tar --version 2
```

**注意**: 从 Docker 导出的系统可能缺少一些工具，需要额外配置。

---

## 安装后配置

### 1. 配置软件源（使用国内镜像）

```bash
# 备份原配置
sudo cp /usr/share/xbps.d/00-repository-main.conf /usr/share/xbps.d/00-repository-main.conf.bak

# 创建本地配置
sudo mkdir -p /etc/xbps.d

# 使用清华镜像
sudo tee /etc/xbps.d/00-repository-main.conf << 'EOF'
repository=https://mirrors.tuna.tsinghua.edu.cn/voidlinux/current
repository=https://mirrors.tuna.tsinghua.edu.cn/voidlinux/current/nonfree
EOF

# 或使用上海交大镜像
sudo tee /etc/xbps.d/00-repository-main.conf << 'EOF'
repository=https://mirror.sjtu.edu.cn/voidlinux/current
repository=https://mirror.sjtu.edu.cn/voidlinux/current/nonfree
EOF

# 同步仓库
sudo xbps-install -S
```

### 2. 安装开发工具

```bash
# 基础开发工具
sudo xbps-install -S base-devel

# 常用工具
sudo xbps-install -S \
    git curl wget \
    vim neovim \
    tmux screen \
    htop btop \
    tree fd ripgrep \
    jq yq \
    unzip zip p7zip \
    openssh \
    python3 python3-pip \
    nodejs npm

# 构建工具
sudo xbps-install -S \
    gcc g++ make cmake \
    pkg-config autoconf automake
```

### 3. 配置 Git

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
git config --global core.autocrlf input
```

### 4. 配置 Shell

#### 安装和配置 zsh

```bash
# 安装 zsh
sudo xbps-install -S zsh

# 切换默认 shell
chsh -s /bin/zsh

# 安装 oh-my-zsh
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

# 或使用国内镜像
sh -c "$(curl -fsSL https://gitee.com/mirrors/oh-my-zsh/raw/master/tools/install.sh)"
```

#### 配置 bash

```bash
# 编辑 .bashrc
vim ~/.bashrc
```

添加常用配置：

```bash
# 别名
alias ll='ls -lh'
alias la='ls -lah'
alias ..='cd ..'
alias ...='cd ../..'

# XBPS 别名
alias xi='sudo xbps-install'
alias xs='xbps-query -Rs'
alias xr='sudo xbps-remove'
alias xu='sudo xbps-install -Su'

# 历史记录
export HISTSIZE=10000
export HISTFILESIZE=10000

# 颜色支持
export CLICOLOR=1
export LS_COLORS='di=34:ln=35:so=32:pi=33:ex=31:bd=34;46:cd=34;43:su=30;41:sg=30;46:tw=30;42:ow=30;43'
```

### 5. 配置 Windows Terminal 集成

编辑 Windows Terminal 配置文件（`settings.json`）：

```json
{
    "profiles": {
        "list": [
            {
                "guid": "{生成一个新的 GUID}",
                "name": "Void Linux",
                "commandline": "wsl.exe -d VoidLinux",
                "icon": "C:\\path\\to\\void-icon.png",
                "startingDirectory": "~",
                "colorScheme": "One Half Dark",
                "fontFace": "Cascadia Code",
                "fontSize": 11
            }
        ]
    }
}
```

生成 GUID：

```powershell
[guid]::NewGuid()
```

---

## 常见问题

### 0. 解压 tar.xz 时出现文件冲突或同步问题

**问题描述**：
- 使用 7-Zip 或 WinRAR 解压时出现"文件已存在"或"同步冲突"
- OneDrive/Dropbox 提示文件冲突
- 解压后文件异常或权限错误
- 导入 WSL 后系统无法正常启动

**根本原因**：

1. **符号链接冲突**
   ```
   例如: /usr/bin/python -> python3.11
   Windows 工具会创建两个文件：
   - python (文本文件，内容: "python3.11")
   - python (符号链接目标)
   导致文件名冲突！
   ```

2. **云同步软件干扰**
   - OneDrive/Dropbox 检测到大量文件变化
   - 尝试同步符号链接和特殊文件
   - 造成文件锁定和冲突

3. **文件名不兼容**
   - Linux 允许 `:` `*` `?` 等字符，Windows 不允许
   - 大小写敏感问题（Linux 区分，Windows 不区分）

**✅ 正确解决方案**：

**方案 1: 直接导入 tar.xz（强烈推荐）**

```powershell
# 无需解压，直接导入！
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar.xz --version 2
```

这是最安全、最简单的方法，WSL 会在内部正确处理所有 Linux 特性。

**方案 2: 在现有 WSL 中解压**

```powershell
# 启动已有的 WSL（如 Ubuntu）
wsl

# 在 Linux 环境中解压
cd /mnt/c/wsl-distros
xz -d void-rootfs.tar.xz  # 得到 .tar 文件
exit

# 然后导入
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar --version 2
```

**方案 3: 避免云同步冲突**

如果必须在 Windows 上解压：

```powershell
# 1. 确保目录不在同步文件夹中
# 错误示例：
# C:\Users\YourName\OneDrive\wsl-distros  ❌
# C:\Users\YourName\Dropbox\wsl-distros   ❌

# 正确示例：
# C:\wsl-distros                          ✅
# D:\wsl-distros                          ✅

# 2. 排除目录（OneDrive 示例）
# 右键文件夹 → 释放空间
# 或在 OneDrive 设置中排除此文件夹

# 3. 使用 7-Zip 解压（管理员模式）
cd C:\wsl-distros
"C:\Program Files\7-Zip\7z.exe" x void-rootfs.tar.xz -aoa

# 4. 立即导入（不要等待）
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar --version 2

# 5. 删除 .tar 文件
Remove-Item C:\wsl-distros\void-rootfs.tar
```

**预防措施**：

```powershell
# 检查目录是否在云同步中
Get-Item "C:\wsl-distros" | Select-Object -ExpandProperty Attributes
# 如果包含 "ReparsePoint" 或 "Cloud"，说明在云同步中

# 检查 WSL 版本
wsl --version
# 确保版本 >= 2.0.0 以支持 tar.xz 直接导入
```

**如果已经出现冲突**：

```powershell
# 1. 停止云同步
# OneDrive: 右键任务栏图标 → 暂停同步

# 2. 删除冲突文件
Remove-Item -Recurse -Force C:\wsl-distros\*

# 3. 重新下载 rootfs
$url = "https://mirrors.tuna.tsinghua.edu.cn/voidlinux/live/current/void-x86_64-ROOTFS-20230628.tar.xz"
Invoke-WebRequest -Uri $url -OutFile C:\wsl-distros\void-rootfs.tar.xz

# 4. 直接导入（不解压）
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\wsl-distros\void-rootfs.tar.xz --version 2
```

---

### 1. 无法连接网络

**问题**: DNS 解析失败

**解决方案**:

```bash
# 删除自动生成的 resolv.conf
sudo rm /etc/resolv.conf

# 手动创建
sudo tee /etc/resolv.conf << 'EOF'
nameserver 8.8.8.8
nameserver 8.8.4.4
nameserver 1.1.1.1
EOF

# 防止被覆盖
sudo chattr +i /etc/resolv.conf
```

或者配置 WSL 不自动生成：

```bash
sudo tee /etc/wsl.conf << 'EOF'
[network]
generateResolvConf = false
EOF
```

### 2. 时区不正确

```bash
# 设置时区
sudo ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime

# 或使用 timedatectl（如果可用）
sudo timedatectl set-timezone Asia/Shanghai
```

### 3. Locale 问题

```bash
# 编辑 locale 配置
sudo vim /etc/default/libc-locales

# 取消注释需要的 locale
# en_US.UTF-8 UTF-8
# zh_CN.UTF-8 UTF-8

# 生成 locale
sudo xbps-reconfigure -f glibc-locales

# 设置系统 locale
sudo tee /etc/locale.conf << 'EOF'
LANG=en_US.UTF-8
LC_ALL=en_US.UTF-8
EOF

# 或中文环境
sudo tee /etc/locale.conf << 'EOF'
LANG=zh_CN.UTF-8
LC_ALL=zh_CN.UTF-8
EOF
```

### 4. WSL2 内存占用过高

创建或编辑 `C:\Users\YourUsername\.wslconfig`:

```ini
[wsl2]
# 限制内存使用
memory=4GB

# 限制 CPU 核心数
processors=4

# 限制交换空间
swap=2GB

# 关闭页面文件
pageReporting=false

# 设置虚拟硬盘最大大小
[experimental]
sparseVhd=true
```

重启 WSL：

```powershell
wsl --shutdown
```

### 5. runit 在 WSL2 中不工作

WSL2 不使用传统的 init 系统，runit 的服务管理在 WSL2 中有限制。

**解决方案**:

```bash
# 手动启动服务而不是使用 runit
# 例如启动 sshd:
sudo /usr/bin/sshd

# 或创建启动脚本
sudo tee /usr/local/bin/start-services.sh << 'EOF'
#!/bin/bash
# 启动 SSH
/usr/bin/sshd

# 启动 Docker（如果安装）
dockerd > /dev/null 2>&1 &

# 其他服务...
EOF

sudo chmod +x /usr/local/bin/start-services.sh

# 在 .bashrc 或 .zshrc 中自动运行
echo 'sudo /usr/local/bin/start-services.sh' >> ~/.bashrc
```

**使用 systemd（实验性）**:

如果需要完整的 init 系统，可以尝试在 WSL2 中启用 systemd（Windows 11 22H2+）：

```bash
# 编辑 wsl.conf
sudo tee /etc/wsl.conf << 'EOF'
[boot]
systemd=true
EOF
```

重启 WSL：

```powershell
wsl --shutdown
wsl -d VoidLinux
```

**注意**: Void Linux 默认使用 runit，启用 systemd 可能导致冲突。

### 6. 访问 Windows 文件

Windows 驱动器挂载在 `/mnt/`:

```bash
# C 盘
cd /mnt/c

# D 盘
cd /mnt/d

# 用户目录
cd /mnt/c/Users/YourUsername

# 创建软链接方便访问
ln -s /mnt/c/Users/YourUsername ~/winuser
```

### 7. 从 Windows 访问 WSL 文件

在 Windows 文件资源管理器中：

```
\\wsl$\VoidLinux\home\yourusername
```

或使用 PowerShell：

```powershell
# 打开 WSL home 目录
explorer.exe \\wsl$\VoidLinux\home\yourusername
```

---

## WSL2 特殊优化

### 1. 提高文件系统性能

**在 WSL 文件系统中工作**，而不是 `/mnt/c/`：

```bash
# 好的做法：项目放在 WSL 文件系统
~/projects/myproject

# 不好的做法：项目放在 Windows 文件系统（性能差）
/mnt/c/Users/YourName/projects/myproject
```

**原因**: WSL2 的 ext4 文件系统比通过网络协议访问 NTFS 快得多。

### 2. 配置 Git 凭据

在 WSL 中使用 Windows 的 Git 凭据管理器：

```bash
# 配置 Git 使用 Windows 凭据管理器
git config --global credential.helper "/mnt/c/Program\ Files/Git/mingw64/bin/git-credential-manager-core.exe"

# 或使用更简单的路径
git config --global credential.helper "/mnt/c/Program\ Files/Git/mingw64/libexec/git-core/git-credential-wincred.exe"
```

### 3. SSH 密钥共享

共享 Windows 和 WSL 的 SSH 密钥：

```bash
# 创建软链接
ln -s /mnt/c/Users/YourUsername/.ssh ~/.ssh

# 修复权限（SSH 要求严格权限）
chmod 700 ~/.ssh
chmod 600 ~/.ssh/id_rsa
chmod 644 ~/.ssh/id_rsa.pub
```

### 4. 端口转发

WSL2 自动转发端口，但有时需要手动配置。

**在 WSL 中启动服务**:

```bash
# 启动 Web 服务器
python3 -m http.server 8000
```

**从 Windows 访问**:

```
http://localhost:8000
```

**手动端口转发**（PowerShell 管理员）:

```powershell
# 获取 WSL2 IP
wsl hostname -I

# 端口转发
netsh interface portproxy add v4tov4 listenport=8000 listenaddress=0.0.0.0 connectport=8000 connectaddress=<WSL2_IP>

# 查看转发规则
netsh interface portproxy show all

# 删除转发
netsh interface portproxy delete v4tov4 listenport=8000 listenaddress=0.0.0.0
```

### 5. 图形应用支持（WSLg）

Windows 11 支持原生运行 Linux 图形应用。

**安装图形应用**:

```bash
# 更新系统
sudo xbps-install -Su

# 安装 X11 应用
sudo xbps-install -S firefox

# 运行
firefox &
```

**检查显示**:

```bash
echo $DISPLAY
# 输出: :0
```

---

## 开发环境配置示例

### Python 开发环境

```bash
# 安装 Python 和工具
sudo xbps-install -S python3 python3-pip python3-devel

# 安装虚拟环境
sudo xbps-install -S python3-virtualenv

# 创建项目
mkdir ~/projects/myapp
cd ~/projects/myapp

# 创建虚拟环境
python3 -m venv venv
source venv/bin/activate

# 安装依赖
pip install flask django requests
```

### Node.js 开发环境

```bash
# 安装 Node.js
sudo xbps-install -S nodejs

# 配置 npm 全局目录（避免 sudo）
mkdir -p ~/.npm-global
npm config set prefix '~/.npm-global'

# 添加到 PATH
echo 'export PATH=~/.npm-global/bin:$PATH' >> ~/.bashrc
source ~/.bashrc

# 安装全局包
npm install -g yarn pnpm typescript
```

### Docker 开发环境

```bash
# 安装 Docker
sudo xbps-install -S docker

# 启动 Docker（手动）
sudo dockerd > /dev/null 2>&1 &

# 添加用户到 docker 组
sudo usermod -aG docker $USER

# 重新登录或使用
newgrp docker

# 测试
docker run hello-world
```

**自动启动 Docker**:

创建 `~/.bashrc` 或 `~/.zshrc` 脚本：

```bash
# 检查 Docker 是否运行
if ! pgrep -x dockerd > /dev/null; then
    sudo dockerd > /dev/null 2>&1 &
fi
```

---

## 管理命令速查

### WSL 基本命令

```powershell
# 列出所有发行版
wsl --list --verbose
wsl -l -v

# 启动指定发行版
wsl -d VoidLinux

# 设置默认发行版
wsl --set-default VoidLinux

# 关闭指定发行版
wsl --terminate VoidLinux

# 关闭所有 WSL
wsl --shutdown

# 注销发行版（删除）
wsl --unregister VoidLinux

# 导出发行版（备份）
wsl --export VoidLinux C:\backups\void-backup.tar

# 导入发行版（恢复）
wsl --import VoidLinux C:\wsl-distros\VoidLinux C:\backups\void-backup.tar --version 2

# 运行命令
wsl -d VoidLinux -- ls -la

# 以特定用户运行
wsl -d VoidLinux -u root

# 更新 WSL
wsl --update

# 查看 WSL 状态
wsl --status
```

### XBPS 常用命令

```bash
# 同步仓库
sudo xbps-install -S

# 更新系统
sudo xbps-install -Su

# 安装软件
sudo xbps-install package

# 搜索软件
xbps-query -Rs keyword

# 删除软件及依赖
sudo xbps-remove -R package

# 清理孤立包
sudo xbps-remove -o

# 清理缓存
sudo xbps-remove -O
```

---

## 卸载 Void Linux

### 方法 1: 使用 WSL 命令（推荐）

```powershell
# 注销发行版
wsl --unregister VoidLinux

# 手动删除文件（可选）
Remove-Item -Recurse -Force C:\wsl-distros\VoidLinux
```

### 方法 2: 手动删除

1. 在 Windows 设置 → 应用 → 应用和功能中查找并卸载
2. 或直接删除安装目录

---

## 性能对比

| 操作 | WSL1 | WSL2 | 说明 |
|------|------|------|------|
| **文件 I/O（Linux）** | 快 | **非常快** | WSL2 使用真实 Linux 内核 |
| **文件 I/O（Windows）** | **快** | 慢 | WSL2 通过网络访问 Windows 文件 |
| **系统调用** | 慢 | **快** | WSL2 原生 Linux 系统调用 |
| **内存占用** | 低 | 中等 | WSL2 需要虚拟机 |
| **启动速度** | **非常快** | 快 | WSL1 直接启动 |
| **网络性能** | **好** | 好 | 都不错 |
| **兼容性** | 差 | **好** | WSL2 兼容性更好 |

**建议**: 除非有特殊需求，否则使用 WSL2。

---

## 相关资源

### 官方文档

- **WSL 官方文档**: https://docs.microsoft.com/zh-cn/windows/wsl/
- **Void Linux 官网**: https://voidlinux.org/
- **Void Linux 文档**: https://docs.voidlinux.org/

### 下载地址

- **Void Linux Rootfs**: https://repo-default.voidlinux.org/live/current/
- **清华镜像**: https://mirrors.tuna.tsinghua.edu.cn/voidlinux/
- **上海交大镜像**: https://mirror.sjtu.edu.cn/voidlinux/

### 社区

- **WSL GitHub**: https://github.com/microsoft/WSL
- **Void Linux GitHub**: https://github.com/void-linux
- **Reddit**: r/voidlinux, r/WSL

---

## 总结

在 WSL2 中安装 Void Linux 的步骤总结：

1. ✅ 启用 WSL2 功能
2. ✅ 下载 Void Linux Rootfs
3. ✅ 解压 `.tar.xz` 到 `.tar`
4. ✅ 使用 `wsl --import` 导入
5. ✅ 配置用户和软件源
6. ✅ 安装开发工具
7. ✅ 优化 WSL2 设置

**关键点**:
- 使用 WSL2（不是 WSL1）
- 推荐 glibc 版本（兼容性好）
- 使用国内镜像加速
- 文件放在 WSL 文件系统中以获得最佳性能
- runit 在 WSL2 中受限，手动管理服务

**优势**:
- 真正的 Void Linux 环境
- 与 Windows 无缝集成
- 开发体验优秀
- 资源占用合理

祝使用愉快！🎉

---

*最后更新: 2025-12-07*
