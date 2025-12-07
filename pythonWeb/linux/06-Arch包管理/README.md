# 06 - Arch Linux 包管理

> Arch Linux 独有的包管理系统 - 这是 Arch 的灵魂

## 目录

- [pacman - 官方包管理器](#pacman---官方包管理器)
- [makepkg - 构建包](#makepkg---构建包)
- [PKGBUILD - 构建脚本](#pkgbuild---构建脚本)
- [AUR - Arch用户仓库](#aur---arch用户仓库)
- [AUR辅助工具](#aur辅助工具)
- [包维护最佳实践](#包维护最佳实践)

---

## pacman - 官方包管理器

### 基本概念

**pacman** 是 Arch Linux 的官方包管理器,特点:
- 快速、轻量
- 依赖解析
- 滚动更新
- 二进制包管理

**配置文件**: `/etc/pacman.conf`

### 安装软件

```bash
# 安装单个包
sudo pacman -S package_name

# 安装多个包
sudo pacman -S package1 package2 package3

# 安装包组
sudo pacman -S gnome

# 重新安装包
sudo pacman -S --needed package_name

# 安装本地包文件
sudo pacman -U /path/to/package.pkg.tar.zst

# 从网络安装
sudo pacman -U https://example.com/package.pkg.tar.zst

# 安装为依赖项
sudo pacman -S --asdeps package_name
```

### 删除软件

```bash
# 删除包（保留依赖）
sudo pacman -R package_name

# 删除包及其依赖
sudo pacman -Rs package_name

# 删除包、依赖和配置文件
sudo pacman -Rns package_name

# 删除孤立包（不被任何包依赖的包）
sudo pacman -Rs $(pacman -Qtdq)

# 或使用更安全的方式
sudo pacman -Rns $(pacman -Qtdq)
```

### 更新系统

```bash
# 同步数据库
sudo pacman -Sy

# 同步数据库并升级所有包（推荐）
sudo pacman -Syu

# 强制刷新数据库并升级
sudo pacman -Syyu

# 仅下载更新不安装
sudo pacman -Syw

# 忽略某个包更新
sudo pacman -Syu --ignore package_name
```

⚠️ **重要**: 永远不要单独运行 `pacman -Sy`! 应该使用 `pacman -Syu` 避免部分更新。

### 查询软件

```bash
# 搜索包
pacman -Ss keyword

# 搜索已安装的包
pacman -Qs keyword

# 显示包详细信息
pacman -Si package_name

# 显示已安装包的详细信息
pacman -Qi package_name

# 列出所有已安装的包
pacman -Q

# 列出明确安装的包
pacman -Qe

# 列出依赖包
pacman -Qd

# 列出孤立包
pacman -Qtd

# 查询文件属于哪个包
pacman -Qo /path/to/file

# 列出包含的文件
pacman -Ql package_name

# 搜索包含特定文件的包（需要同步数据库）
pacman -F filename
sudo pacman -Fy  # 更新文件数据库
```

### 清理缓存

```bash
# 清理未安装包的缓存
sudo pacman -Sc

# 清理所有缓存
sudo pacman -Scc

# 只保留最近3个版本（需要 paccache）
sudo paccache -r

# 删除未安装包的缓存
sudo paccache -ruk0
```

### 依赖管理

```bash
# 查看包的依赖
pactree package_name

# 查看包被谁依赖
pactree -r package_name

# 查看可选依赖
pacman -Qi package_name | grep "Optional Deps"

# 安装所有可选依赖
sudo pacman -S --asdeps $(pacman -Qi package_name | sed -n '/^Optional Deps/,/^Conflicts/p' | grep "None" | cut -d: -f1)
```

### 实用别名

添加到 `~/.bashrc`:

```bash
# 更新系统
alias update='sudo pacman -Syu'

# 安装包
alias install='sudo pacman -S'

# 删除包
alias remove='sudo pacman -Rns'

# 搜索包
alias search='pacman -Ss'

# 查询已安装
alias installed='pacman -Q'

# 清理系统
alias cleanup='sudo pacman -Rns $(pacman -Qtdq) && sudo pacman -Sc'

# 列出明确安装的包
alias explicit='pacman -Qe'

# 查看孤立包
alias orphans='pacman -Qtd'
```

### 常见问题

**1. 签名错误**
```bash
# 更新密钥环
sudo pacman -Sy archlinux-keyring
sudo pacman-key --refresh-keys
```

**2. 数据库锁定**
```bash
# 删除锁文件（确保pacman未运行）
sudo rm /var/lib/pacman/db.lck
```

**3. 损坏的包**
```bash
# 重新安装
sudo pacman -S --needed package_name
```

**4. 降级包**
```bash
# 从缓存降级
sudo pacman -U /var/cache/pacman/pkg/package-old-version.pkg.tar.zst

# 使用 downgrade 工具
sudo pacman -S downgrade
sudo downgrade package_name
```

---

## makepkg - 构建包

### 基本用法

```bash
# 基本构建
makepkg

# 构建并安装依赖
makepkg -s

# 构建后安装
makepkg -i

# 组合（最常用）
makepkg -si

# 清理构建文件
makepkg -c

# 强制重新构建
makepkg -f

# 构建源码包
makepkg --source

# 不解压源码
makepkg -o

# 提取源码并准备
makepkg -e
```

### 完整流程示例

```bash
# 1. 创建构建目录
mkdir ~/builds/mypackage
cd ~/builds/mypackage

# 2. 获取 PKGBUILD
# （从AUR或自己编写）

# 3. 检查 PKGBUILD
cat PKGBUILD

# 4. 构建并安装
makepkg -si

# 5. 清理
makepkg -c
```

### 配置文件

**全局配置**: `/etc/makepkg.conf`
**用户配置**: `~/.makepkg.conf`

**常用配置**:

```bash
# 编辑 ~/.makepkg.conf

# 构建目录
BUILDDIR=/tmp/makepkg

# 使用所有CPU核心编译
MAKEFLAGS="-j$(nproc)"

# 压缩方式
PKGEXT='.pkg.tar.zst'

# 编译器优化
CFLAGS="-march=native -O2 -pipe -fno-plt"
CXXFLAGS="${CFLAGS}"

# 调试包
OPTIONS=(strip docs !libtool !staticlibs emptydirs zipman purge !debug)
```

### 常见参数详解

```bash
-s, --syncdeps     # 使用pacman安装依赖
-i, --install      # 构建后安装包
-c, --clean        # 构建后清理
-f, --force        # 强制覆盖已存在的包
-r, --rmdeps       # 删除安装的依赖
-d, --nodeps       # 跳过依赖检查
-e, --noextract    # 不解压源文件
-o, --nobuild      # 下载并解压源码但不构建
-L, --log          # 启用日志
--skipchecksums    # 跳过校验和
--skippgpcheck     # 跳过PGP签名验证
--nosign           # 不签名包
```

---

## PKGBUILD - 构建脚本

### 基本结构

```bash
# PKGBUILD 示例

# 维护者信息
# Maintainer: Your Name <email@example.com>

# 包基本信息
pkgname=mypackage
pkgver=1.0.0
pkgrel=1
pkgdesc="A simple example package"
arch=('x86_64')
url="https://example.com"
license=('GPL')

# 依赖
depends=('glibc')
makedepends=('gcc' 'make')
optdepends=('optional-package: for extra feature')
conflicts=('conflicting-package')
provides=('virtual-package')
replaces=('old-package')

# 源码
source=("https://example.com/$pkgname-$pkgver.tar.gz")
sha256sums=('SKIP')  # 或实际的SHA256值

# 构建函数
prepare() {
    cd "$pkgname-$pkgver"
    # 准备步骤: 打补丁等
    patch -p1 < "$srcdir/fix.patch"
}

build() {
    cd "$pkgname-$pkgver"
    # 构建步骤
    ./configure --prefix=/usr
    make
}

check() {
    cd "$pkgname-$pkgver"
    # 测试步骤
    make test
}

package() {
    cd "$pkgname-$pkgver"
    # 安装到 $pkgdir
    make DESTDIR="$pkgdir" install
}
```

### 变量说明

**必需变量**:
```bash
pkgname    # 包名
pkgver     # 版本号
pkgrel     # 发布号
arch       # 架构: x86_64, i686, any
```

**可选但推荐**:
```bash
pkgdesc    # 包描述
url        # 项目主页
license    # 许可证
depends    # 运行依赖
makedepends # 编译依赖
source     # 源码URL或文件
md5sums    # MD5校验和
sha256sums # SHA256校验和
```

**其他变量**:

```bash
optdepends   # 可选依赖
conflicts    # 冲突包
provides     # 提供的虚拟包
replaces     # 替换的包
backup       # 需要备份的配置文件
install      # .install 脚本
options      # 构建选项
```

### 常用函数

```bash
prepare()   # 源码准备（打补丁等）
build()     # 编译
check()     # 运行测试
package()   # 安装文件到 $pkgdir
```

### 分包示例

```bash
# 基础包
pkgbase=myproject
pkgname=('myproject' 'myproject-docs')
pkgver=1.0.0
pkgrel=1

build() {
    cd "$pkgbase-$pkgver"
    make
}

package_myproject() {
    pkgdesc="Main package"
    depends=('glibc')

    cd "$pkgbase-$pkgver"
    make DESTDIR="$pkgdir" install
}

package_myproject-docs() {
    pkgdesc="Documentation for myproject"
    arch=('any')

    cd "$pkgbase-$pkgver"
    make DESTDIR="$pkgdir" install-docs
}
```

### Git 包示例

```bash
# Maintainer: Your Name <email@example.com>

pkgname=myapp-git
pkgver=r123.abc1234
pkgrel=1
pkgdesc="Description of the git package"
arch=('x86_64')
url="https://github.com/user/myapp"
license=('MIT')
depends=('glibc')
makedepends=('git' 'gcc')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
source=("git+https://github.com/user/myapp.git")
md5sums=('SKIP')

pkgver() {
    cd "$srcdir/${pkgname%-git}"
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd "$srcdir/${pkgname%-git}"
    make
}

package() {
    cd "$srcdir/${pkgname%-git}"
    make DESTDIR="$pkgdir" PREFIX=/usr install
}
```

### 实用技巧

**1. 生成校验和**
```bash
updpkgsums  # 自动更新PKGBUILD中的校验和
```

**2. 检查PKGBUILD**
```bash
namcap PKGBUILD  # 检查PKGBUILD规范
sudo pacman -S namcap  # 安装namcap
```

**3. 检查构建的包**
```bash
namcap package.pkg.tar.zst
```

**4. 源码URL变量**
```bash
source=("$pkgname-$pkgver.tar.gz::https://example.com/archive/v$pkgver.tar.gz")
```

---

## AUR - Arch用户仓库

### 什么是AUR?

**AUR (Arch User Repository)** 是社区维护的软件仓库:
- 包含官方仓库没有的软件
- 用户提交和维护PKGBUILD
- 不提供二进制包,需要本地构建
- 网址: https://aur.archlinux.org/

### 手动使用AUR

```bash
# 1. 搜索包
# 访问 https://aur.archlinux.org/

# 2. 克隆包
git clone https://aur.archlinux.org/package_name.git
cd package_name

# 3. 查看PKGBUILD（重要！）
cat PKGBUILD
cat .SRCINFO

# 4. 构建并安装
makepkg -si

# 5. 更新包
cd package_name
git pull
makepkg -si
```

### AUR安全须知

⚠️ **安全建议**:
1. **始终检查PKGBUILD** - AUR包可以运行任意代码!
2. 查看评论和投票数
3. 检查维护者和最后更新时间
4. 对于复杂的PKGBUILD,理解每一行的作用
5. 优先选择有很多投票和活跃维护的包

```bash
# 检查PKGBUILD中的可疑内容
grep -E '(rm -rf|curl.*sh|wget.*sh|eval)' PKGBUILD
```

---

## AUR辅助工具

### yay - Yet Another Yogurt

**安装yay**:
```bash
# 手动安装
cd /tmp
git clone https://aur.archlinux.org/yay.git
cd yay
makepkg -si
```

**基本用法**:
```bash
# 搜索包
yay -Ss package_name

# 安装包
yay -S package_name

# 更新所有包（包括AUR）
yay -Syu

# 只更新AUR包
yay -Sua

# 删除包
yay -Rns package_name

# 查询已安装
yay -Qs package_name

# 清理不需要的依赖
yay -Yc

# 显示包统计信息
yay -Ps
```

**高级用法**:
```bash
# 编辑PKGBUILD before build
yay -S package_name --editmenu

# 不确认直接安装
yay -S package_name --noconfirm

# 重新下载PKGBUILD
yay -S package_name --rebuild

# 清理构建缓存
yay -Sc

# 显示AUR包的详细信息
yay -Si package_name
```

### paru - Paru AUR Helper

**安装paru**:
```bash
cd /tmp
git clone https://aur.archlinux.org/paru.git
cd paru
makepkg -si
```

**基本用法**:
```bash
# 与yay类似
paru -Ss package_name
paru -S package_name
paru -Syu

# paru特有功能
paru -Gp package_name  # 打印PKGBUILD
paru -c                # 清理不需要的包
```

### 对比

| 功能 | yay | paru |
|------|-----|------|
| 语言 | Go | Rust |
| 速度 | 快 | 更快 |
| 功能 | 丰富 | 丰富且现代化 |
| 配置 | 简单 | 更灵活 |
| 推荐 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## 包维护最佳实践

### 系统维护

```bash
# 1. 定期更新系统
sudo pacman -Syu  # 每天或每周

# 2. 清理孤立包
sudo pacman -Rns $(pacman -Qtdq)

# 3. 清理包缓存
sudo paccache -r  # 保留最近3个版本

# 4. 检查系统文件
sudo pacman -Qkk  # 检查所有包的文件

# 5. 更新文件数据库
sudo pacman -Fy
```

### 备份和恢复

**1. 备份已安装包列表**
```bash
# 明确安装的包
pacman -Qqe > pkglist.txt

# 包括AUR包
pacman -Qqem > aurlist.txt

# 官方仓库包
pacman -Qqen > repolist.txt
```

**2. 恢复包**
```bash
# 官方包
sudo pacman -S --needed - < repolist.txt

# AUR包（使用yay）
yay -S --needed - < aurlist.txt
```

**3. 备份pacman数据库**
```bash
sudo tar -czvf pacman-database.tar.gz /var/lib/pacman/local
```

### 镜像管理

**1. 更新镜像列表**
```bash
# 安装 reflector
sudo pacman -S reflector

# 自动选择最快的镜像
sudo reflector --country China --age 12 --protocol https --sort rate --save /etc/pacman.d/mirrorlist

# 或手动编辑
sudo vim /etc/pacman.d/mirrorlist
```

**2. 添加中国镜像**
```bash
# 编辑 /etc/pacman.d/mirrorlist
# 添加到文件开头
Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch
Server = https://mirrors.ustc.edu.cn/archlinux/$repo/os/$arch
Server = https://mirrors.aliyun.com/archlinux/$repo/os/$arch
```

### 故障排除

**1. 依赖问题**
```bash
# 查看依赖树
pactree package_name

# 强制忽略依赖
sudo pacman -Rdd package_name  # 不推荐!
```

**2. 文件冲突**
```bash
# 查看冲突文件属于哪个包
pacman -Qo /path/to/file

# 强制覆盖（小心！）
sudo pacman -S --overwrite /path/to/file package_name
```

**3. 部分更新**
```bash
# 避免部分更新
# 永远不要: pacman -Sy package_name
# 应该用: pacman -Syu
```

### Pacman钩子

创建自定义钩子 `/etc/pacman.d/hooks/`:

**示例: 清理内核**
```bash
# /etc/pacman.d/hooks/linux-remove.hook
[Trigger]
Operation = Remove
Type = Package
Target = linux

[Action]
Description = Cleaning up old kernels...
When = PostTransaction
Exec = /usr/bin/rm -rf /boot/vmlinuz-linux-old
```

**示例: 更新GRUB**
```bash
# /etc/pacman.d/hooks/grub-update.hook
[Trigger]
Operation = Upgrade
Type = Package
Target = grub

[Action]
Description = Updating GRUB...
When = PostTransaction
Exec = /usr/bin/grub-mkconfig -o /boot/grub/grub.cfg
```

---

## 实用脚本

### 自动更新脚本

```bash
#!/bin/bash
# update-system.sh

echo "=== 更新Arch系统 ==="

echo "1. 更新包数据库..."
sudo pacman -Sy

echo "2. 更新所有包..."
sudo pacman -Syu --noconfirm

echo "3. 更新AUR包..."
yay -Sua --noconfirm

echo "4. 清理孤立包..."
sudo pacman -Rns $(pacman -Qtdq) --noconfirm

echo "5. 清理包缓存..."
sudo paccache -r

echo "6. 更新文件数据库..."
sudo pacman -Fy

echo "=== 更新完成 ==="
```

### 包搜索和安装脚本

```bash
#!/bin/bash
# search-install.sh

if [ -z "$1" ]; then
    echo "用法: $0 <搜索关键词>"
    exit 1
fi

echo "搜索: $1"
yay -Ss "$1"

echo -n "安装包名 (留空取消): "
read pkg

if [ -n "$pkg" ]; then
    yay -S "$pkg"
fi
```

---

## 高级技巧

### 降级包

```bash
# 使用downgrade
sudo pacman -S downgrade
sudo downgrade package_name

# 手动从缓存
ls /var/cache/pacman/pkg/ | grep package_name
sudo pacman -U /var/cache/pacman/pkg/package-old-version.pkg.tar.zst

# 从Arch Archive
# https://archive.archlinux.org/packages/
```

### 锁定包版本

```bash
# 编辑 /etc/pacman.conf
# 添加到 [options] 部分
IgnorePkg = package_name

# 或临时忽略
sudo pacman -Syu --ignore package_name
```

### 自定义仓库

```bash
# 1. 创建仓库目录
mkdir -p /path/to/repo

# 2. 复制包文件
cp *.pkg.tar.zst /path/to/repo/

# 3. 创建数据库
repo-add /path/to/repo/custom.db.tar.gz /path/to/repo/*.pkg.tar.zst

# 4. 编辑 /etc/pacman.conf
[custom]
SigLevel = Optional TrustAll
Server = file:///path/to/repo

# 5. 更新并安装
sudo pacman -Sy
sudo pacman -S package_from_custom_repo
```

---

## 相关资源

- [Arch Wiki - pacman](https://wiki.archlinux.org/title/Pacman)
- [Arch Wiki - PKGBUILD](https://wiki.archlinux.org/title/PKGBUILD)
- [Arch Wiki - AUR](https://wiki.archlinux.org/title/Arch_User_Repository)
- [AUR 首页](https://aur.archlinux.org/)
- [PKGBUILD 示例](https://gitlab.archlinux.org/pacman/pacman/-/tree/master/proto)

---

**提示**: Arch Linux 采用滚动更新模式,建议:
- 定期更新系统（至少每周一次）
- 更新前查看 [Arch News](https://archlinux.org/news/)
- 重要系统更新前备份数据
- 了解 PKGBUILD 后再构建 AUR 包
