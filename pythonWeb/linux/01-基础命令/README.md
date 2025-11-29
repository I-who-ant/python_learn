# 01 - 基础命令

> 日常最常用的文件和目录操作命令

## 目录

- [文件查看命令](#文件查看命令)
- [文件操作命令](#文件操作命令)
- [目录管理命令](#目录管理命令)
- [文件查找命令](#文件查找命令)
- [权限管理命令](#权限管理命令)
- [文件属性命令](#文件属性命令)

---

## 文件查看命令

### `ls` - 列出目录内容

**基本用法**:

```bash
ls [选项] [目录]
```

**常用参数**:
```bash
ls -l      # 详细列表格式 (long)
ls -a      # 显示隐藏文件 (all)
ls -h      # 人类可读的文件大小 (human-readable)
ls -t      # 按修改时间排序 (time)
ls -r      # 反向排序 (reverse)
ls -R      # 递归显示子目录 (recursive)
ls -S      # 按文件大小排序 (size)
```

**实用组合**:
```bash
ls -lah    # 详细列表 + 隐藏文件 + 可读大小
ls -lt     # 按时间排序的详细列表
ls -lS     # 按大小排序的详细列表
ls -lR     # 递归显示所有子目录
```

**示例**:
```bash
# 查看当前目录
ls

# 查看详细信息
ls -l
# 输出: drwxr-xr-x 2 user group 4096 Nov 28 10:30 folder

# 查看所有文件（包括隐藏）
ls -la

# 按时间倒序（最新的在前）
ls -lt

# 只显示目录
ls -d */

# 显示完整路径
ls -d $PWD/*
```

**输出解释**:
```
drwxr-xr-x  2  user  group  4096  Nov 28 10:30  folder
│   │         │   │     │      │      │           └─ 文件名
│   │         │   │     │      │      └─ 修改时间
│   │         │   │     │      └─ 大小（字节）
│   │         │   │     └─ 所属组
│   │         │   └─ 所有者
│   │         └─ 硬链接数
│   └─ 权限 (rwx: 读写执行)
└─ 文件类型 (d=目录, -=文件, l=链接)
```

---

### `cat` - 查看文件内容

**基本用法**:
```bash
cat [选项] 文件名
```

**常用参数**:
```bash
cat -n     # 显示行号
cat -b     # 非空行显示行号
cat -s     # 压缩连续空行
cat -A     # 显示所有字符（包括不可见字符）
```

**示例**:
```bash
# 查看文件
cat file.txt

# 显示行号
cat -n file.txt

# 合并多个文件
cat file1.txt file2.txt > merged.txt

# 追加内容
cat >> file.txt << EOF
新内容
EOF

# 创建新文件
cat > newfile.txt << EOF
第一行
第二行
EOF
```

---

### `less` - 分页查看文件

**基本用法**:
```bash
less [选项] 文件名
```

**常用快捷键**:

```
空格      # 下一页
b         # 上一页
/pattern  # 向下搜索
?pattern  # 向上搜索
n         # 下一个匹配
N         # 上一个匹配
g         # 跳到文件开头
G         # 跳到文件结尾
q         # 退出
```

**常用参数**:
```bash
less -N    # 显示行号
less -S    # 不换行显示长行
less -i    # 搜索时忽略大小写
```

**示例**:
```bash
# 查看日志文件
less /var/log/syslog

# 显示行号
less -N file.txt

# 实时查看文件更新（类似 tail -f）
less +F file.txt
```

---

### `more` - 简单分页查看

**基本用法**:
```bash
more 文件名
```

**快捷键**:
```
空格      # 下一页
回车      # 下一行
q         # 退出
/pattern  # 搜索
```

**示例**:
```bash
# 查看文件
more file.txt

# 从第10行开始
more +10 file.txt
```

---

### `head` - 查看文件开头

**基本用法**:
```bash
head [选项] 文件名
```

**常用参数**:
```bash
head -n 10    # 显示前10行（默认也是10行）
head -n -5    # 显示除最后5行外的所有行
head -c 100   # 显示前100字节
```

**示例**:
```bash
# 查看前10行
head file.txt

# 查看前20行
head -n 20 file.txt

# 查看多个文件的开头
head -n 5 *.txt
```

---

### `tail` - 查看文件结尾

**基本用法**:
```bash
tail [选项] 文件名
```

**常用参数**:
```bash
tail -n 10    # 显示最后10行
tail -n +5    # 从第5行开始显示到结尾
tail -f       # 实时跟踪文件更新（follow）
tail -F       # 类似-f，但文件被删除重建时继续跟踪
```

**示例**:
```bash
# 查看最后10行
tail file.txt

# 实时监控日志
tail -f /var/log/syslog

# 显示最后20行并持续监控
tail -n 20 -f app.log

# 同时监控多个文件
tail -f file1.log file2.log
```

---

## 文件操作命令

### `cp` - 复制文件

**基本用法**:
```bash
cp [选项] 源文件 目标文件
```

**常用参数**:
```bash
cp -r      # 递归复制目录 (recursive)
cp -i      # 覆盖前询问 (interactive)
cp -f      # 强制覆盖 (force)
cp -p      # 保留文件属性 (preserve)
cp -a      # 归档模式（= -dpr）
cp -u      # 仅复制更新的文件 (update)
cp -v      # 显示详细过程 (verbose)
```

**示例**:
```bash
# 复制文件
cp file.txt backup.txt

# 复制到目录
cp file.txt /path/to/dir/

# 复制目录
cp -r dir1/ dir2/

# 保留属性复制
cp -p file.txt backup.txt

# 备份文件（不覆盖）
cp -n file.txt backup.txt

# 创建符号链接而不是复制
cp -s file.txt link.txt
```

---

### `mv` - 移动/重命名文件

**基本用法**:
```bash
mv [选项] 源文件 目标文件
```

**常用参数**:
```bash
mv -i      # 覆盖前询问
mv -f      # 强制覆盖
mv -n      # 不覆盖已存在文件
mv -v      # 显示详细过程
```

**示例**:
```bash
# 重命名文件
mv oldname.txt newname.txt

# 移动文件
mv file.txt /path/to/dir/

# 移动多个文件
mv file1.txt file2.txt /path/to/dir/

# 移动目录
mv dir1/ /path/to/new/location/

# 批量重命名（结合其他命令）
for file in *.txt; do
    mv "$file" "${file%.txt}.bak"
done
```

---

### `rm` - 删除文件

**基本用法**:
```bash
rm [选项] 文件名
```

**常用参数**:
```bash
rm -r      # 递归删除目录
rm -f      # 强制删除，不询问
rm -i      # 删除前询问
rm -v      # 显示删除过程
```

**示例**:
```bash
# 删除文件
rm file.txt

# 删除多个文件
rm file1.txt file2.txt

# 删除目录
rm -r dir/

# 强制删除（危险！）
rm -rf dir/

# 安全删除（询问确认）
rm -i file.txt

# 删除所有txt文件
rm *.txt
```

**⚠️ 安全提示**:
```bash
# 永远不要执行（会删除整个系统）
rm -rf /

# Arch 推荐使用别名
alias rm='rm -i'  # 添加到 ~/.bashrc

# 或使用回收站工具
sudo pacman -S trash-cli
trash file.txt  # 移到回收站而不是直接删除
```

---

### `touch` - 创建文件/更新时间戳

**基本用法**:
```bash
touch [选项] 文件名
```

**常用参数**:
```bash
touch -a      # 只更新访问时间
touch -m      # 只更新修改时间
touch -c      # 不创建新文件
touch -t      # 使用指定时间
```

**示例**:
```bash
# 创建空文件
touch newfile.txt

# 创建多个文件
touch file1.txt file2.txt file3.txt

# 更新文件时间戳为当前时间
touch existing.txt

# 设置指定时间 (格式: YYYYMMDDhhmm)
touch -t 202511281030 file.txt

# 批量创建文件
touch file{1..10}.txt  # 创建 file1.txt 到 file10.txt
```

---

## 目录管理命令

### `cd` - 切换目录

**基本用法**:
```bash
cd [目录]
```

**快捷路径**:
```bash
cd          # 回到家目录 (~)
cd ~        # 回到家目录
cd -        # 回到上一个目录
cd ..       # 上一级目录
cd ../..    # 上两级目录
cd /        # 根目录
```

**示例**:
```bash
# 切换到指定目录
cd /home/user/documents

# 使用相对路径
cd ./subfolder

# 回到家目录
cd ~

# 在两个目录间切换
cd /var/log
cd -  # 返回之前的目录
cd -  # 再次返回 /var/log
```

---

### `pwd` - 显示当前目录

**基本用法**:
```bash
pwd [选项]
```

**参数**:
```bash
pwd -P     # 显示物理路径（解析符号链接）
pwd -L     # 显示逻辑路径（默认）
```

**示例**:
```bash
# 显示当前路径
pwd
# 输出: /home/user/documents

# 在脚本中使用
CURRENT_DIR=$(pwd)
echo "当前在: $CURRENT_DIR"
```

---

### `mkdir` - 创建目录

**基本用法**:
```bash
mkdir [选项] 目录名
```

**常用参数**:
```bash
mkdir -p    # 递归创建（创建父目录）
mkdir -m    # 设置权限
mkdir -v    # 显示详细过程
```

**示例**:
```bash
# 创建单个目录
mkdir newfolder

# 递归创建多级目录
mkdir -p dir1/dir2/dir3

# 创建多个目录
mkdir dir1 dir2 dir3

# 创建带权限的目录
mkdir -m 755 publicdir

# 批量创建
mkdir -p project/{src,tests,docs,config}
```

---

### `rmdir` - 删除空目录

**基本用法**:
```bash
rmdir [选项] 目录名
```

**参数**:
```bash
rmdir -p    # 递归删除空目录
rmdir -v    # 显示详细过程
```

**示例**:
```bash
# 删除空目录
rmdir emptydir

# 递归删除空目录
rmdir -p dir1/dir2/dir3

# 注意: rmdir 只能删除空目录
# 删除非空目录用: rm -r
```

---

### `tree` - 树状显示目录结构

**安装**:
```bash
sudo pacman -S tree
```

**基本用法**:
```bash
tree [选项] [目录]
```

**常用参数**:
```bash
tree -L 2    # 限制显示层级
tree -a      # 显示隐藏文件
tree -d      # 只显示目录
tree -f      # 显示完整路径
tree -h      # 显示文件大小
tree -p      # 显示权限
tree -u      # 显示所有者
```

**示例**:
```bash
# 显示当前目录树
tree

# 只显示2层
tree -L 2

# 显示目录和文件大小
tree -h

# 只显示目录
tree -d

# 输出到文件
tree > structure.txt
```

---

## 文件查找命令

### `find` - 强大的文件查找工具

**基本用法**:
```bash
find [路径] [条件] [动作]
```

**按名称查找**:
```bash
find . -name "*.txt"          # 查找txt文件
find . -iname "*.TXT"         # 忽略大小写
find . -name "file*"          # 通配符查找
find . -not -name "*.txt"     # 排除txt文件
```

**按类型查找**:
```bash
find . -type f     # 查找文件
find . -type d     # 查找目录
find . -type l     # 查找符号链接
```

**按大小查找**:
```bash
find . -size +10M      # 大于10MB
find . -size -1k       # 小于1KB
find . -size 100c      # 正好100字节
find . -size +1G       # 大于1GB
```

**按时间查找**:
```bash
find . -mtime -7       # 7天内修改过
find . -mtime +30      # 30天前修改
find . -atime -1       # 1天内访问过
find . -ctime -7       # 7天内状态改变
```

**按权限查找**:
```bash
find . -perm 755       # 权限正好是755
find . -perm -644      # 至少有644权限
find . -perm /u+x      # 所有者可执行
```

**组合条件**:
```bash
# AND 条件
find . -name "*.txt" -size +1M

# OR 条件
find . -name "*.txt" -o -name "*.md"

# NOT 条件
find . ! -name "*.txt"
```

**执行动作**:
```bash
# 删除找到的文件
find . -name "*.tmp" -delete

# 对每个文件执行命令
find . -name "*.txt" -exec cat {} \;

# 批量处理（更高效）
find . -name "*.txt" -exec cat {} +

# 交互式确认
find . -name "*.bak" -ok rm {} \;
```

**实用示例**:
```bash
# 查找大文件（大于100MB）
find / -type f -size +100M 2>/dev/null

# 查找空文件
find . -type f -empty

# 查找并移动文件
find . -name "*.log" -exec mv {} /var/log/ \;

# 查找最近修改的文件
find . -type f -mtime -1

# 查找并统计行数
find . -name "*.py" -exec wc -l {} +

# 查找并改变权限
find . -type f -name "*.sh" -exec chmod +x {} \;
```

---

### `locate` - 快速文件查找

**安装和配置**:
```bash
sudo pacman -S mlocate
sudo updatedb  # 更新数据库
```

**基本用法**:
```bash
locate [选项] 模式
```

**常用参数**:
```bash
locate -i      # 忽略大小写
locate -c      # 只显示数量
locate -r      # 使用正则表达式
locate -b      # 只匹配basename
```

**示例**:
```bash
# 查找文件
locate file.txt

# 忽略大小写
locate -i README

# 限制结果数量
locate -n 10 *.conf

# 正则表达式查找
locate -r "\.txt$"

# 更新数据库
sudo updatedb
```

**对比 find**:
- `locate`: 快速，使用数据库，可能不是最新
- `find`: 慢，实时搜索，结果准确

---

### `which` - 查找命令位置

**基本用法**:
```bash
which [命令]
```

**示例**:
```bash
# 查找命令路径
which python
# 输出: /usr/bin/python

# 查找多个命令
which ls cp mv

# 查找所有匹配项
which -a python
```

---

### `whereis` - 查找二进制、源码和手册

**基本用法**:
```bash
whereis [选项] 命令
```

**参数**:
```bash
whereis -b     # 只查找二进制
whereis -m     # 只查找手册
whereis -s     # 只查找源码
```

**示例**:
```bash
# 查找命令相关文件
whereis ls
# 输出: ls: /usr/bin/ls /usr/share/man/man1/ls.1.gz

# 只查找二进制
whereis -b gcc

# 只查找手册
whereis -m python
```

---

## 权限管理命令

### Linux 权限基础

**权限表示**:
```
rwxrwxrwx
│││││││││
││││││└┴┴─ 其他用户 (others)
│││└┴┴─── 组用户 (group)
└┴┴───── 所有者 (owner)

r = 读 (4)
w = 写 (2)
x = 执行 (1)
```

**数字权限**:
```
7 = rwx = 4+2+1
6 = rw- = 4+2
5 = r-x = 4+1
4 = r-- = 4
3 = -wx = 2+1
2 = -w- = 2
1 = --x = 1
0 = --- = 0
```

---

### `chmod` - 修改文件权限

**基本用法**:
```bash
chmod [选项] 模式 文件
```

**数字模式**:
```bash
chmod 755 file.txt    # rwxr-xr-x
chmod 644 file.txt    # rw-r--r--
chmod 600 file.txt    # rw-------
chmod 777 file.txt    # rwxrwxrwx (不推荐)
```

**符号模式**:
```bash
chmod u+x file.txt    # 所有者添加执行权限
chmod g-w file.txt    # 组删除写权限
chmod o=r file.txt    # 其他用户只读
chmod a+x file.txt    # 所有人添加执行权限
chmod u=rwx,g=rx,o=r file.txt  # 组合设置
```

**符号说明**:
```
u = user (所有者)
g = group (组)
o = others (其他)
a = all (所有)

+ = 添加权限
- = 删除权限
= = 设置权限
```

**常用参数**:
```bash
chmod -R 755 dir/     # 递归修改目录
chmod -v 755 file     # 显示详细信息
chmod -c 755 file     # 只显示改变的文件
```

**实用示例**:
```bash
# 使脚本可执行
chmod +x script.sh

# 设置私有文件
chmod 600 private.txt

# 设置公共目录
chmod 755 public_html/

# 递归设置目录权限
chmod -R 755 /var/www/

# 只给所有者权限
chmod 700 secret/

# 设置共享目录
chmod 770 shared/
```

**常见权限组合**:
```bash
644  # 文件常用 (rw-r--r--)
755  # 目录和可执行文件 (rwxr-xr-x)
600  # 私有文件 (rw-------)
700  # 私有目录 (rwx------)
666  # 所有人可读写 (rw-rw-rw-)
777  # 完全开放 (rwxrwxrwx) - 不安全!
```

---

### `chown` - 修改文件所有者

**基本用法**:
```bash
chown [选项] 所有者[:组] 文件
```

**示例**:
```bash
# 改变所有者
sudo chown user file.txt

# 改变所有者和组
sudo chown user:group file.txt

# 只改变组
sudo chown :group file.txt

# 递归修改
sudo chown -R user:group dir/

# 使用当前用户
sudo chown $USER file.txt

# 参考其他文件
sudo chown --reference=ref.txt file.txt
```

---

### `chgrp` - 修改文件所属组

**基本用法**:
```bash
chgrp [选项] 组 文件
```

**示例**:
```bash
# 改变组
sudo chgrp group file.txt

# 递归修改
sudo chgrp -R group dir/

# 显示详细信息
sudo chgrp -v group file.txt
```

---

### `umask` - 设置默认权限

**基本用法**:
```bash
umask [掩码]
```

**说明**:
- umask 定义新建文件/目录的默认权限
- 文件最大权限: 666
- 目录最大权限: 777
- 实际权限 = 最大权限 - umask

**示例**:
```bash
# 查看当前 umask
umask
# 输出: 0022

# 设置 umask
umask 022   # 新文件: 644, 新目录: 755
umask 077   # 新文件: 600, 新目录: 700
umask 002   # 新文件: 664, 新目录: 775

# 计算示例
umask 022:
  文件: 666 - 022 = 644 (rw-r--r--)
  目录: 777 - 022 = 755 (rwxr-xr-x)

# 永久设置（添加到 ~/.bashrc）
echo "umask 022" >> ~/.bashrc
```

---

## 文件属性命令

### `file` - 识别文件类型

**基本用法**:
```bash
file [选项] 文件
```

**示例**:
```bash
# 识别文件类型
file file.txt
# 输出: file.txt: ASCII text

file image.png
# 输出: image.png: PNG image data, 800 x 600, 8-bit/color RGB

# 查看多个文件
file *

# 不解析符号链接
file -h link.txt

# 输出MIME类型
file -i file.txt
# 输出: file.txt: text/plain; charset=utf-8
```

---

### `stat` - 显示文件详细信息

**基本用法**:
```bash
stat [选项] 文件
```

**示例**:
```bash
# 查看文件详细信息
stat file.txt
# 输出:
#   File: file.txt
#   Size: 1024        Blocks: 8          IO Block: 4096
#   Device: 803h/2051d    Inode: 12345      Links: 1
#   Access: (0644/-rw-r--r--)  Uid: ( 1000/    user)
#   Access: 2025-11-28 10:30:00
#   Modify: 2025-11-28 10:25:00
#   Change: 2025-11-28 10:25:00

# 简化输出
stat -c "%n %s" file.txt  # 文件名和大小
stat -c "%a" file.txt     # 八进制权限
stat -c "%U %G" file.txt  # 所有者和组
```

---

### `du` - 磁盘使用情况

**基本用法**:
```bash
du [选项] [目录/文件]
```

**常用参数**:
```bash
du -h      # 人类可读格式
du -s      # 只显示总计
du -a      # 显示所有文件
du -c      # 显示总计
du -d 1    # 限制深度
```

**示例**:
```bash
# 查看当前目录大小
du -sh .

# 查看子目录大小
du -h -d 1

# 查看所有文件大小
du -ah

# 按大小排序
du -h | sort -hr

# 查看最大的10个目录
du -h -d 1 | sort -hr | head -10

# 排除某些目录
du -h --exclude="*.git" .
```

---

### `df` - 磁盘空间使用

**基本用法**:
```bash
df [选项] [文件系统]
```

**常用参数**:
```bash
df -h      # 人类可读格式
df -T      # 显示文件系统类型
df -i      # 显示 inode 信息
df -a      # 显示所有文件系统
```

**示例**:
```bash
# 查看所有磁盘
df -h

# 查看指定分区
df -h /home

# 查看文件系统类型
df -Th

# 查看 inode 使用情况
df -ih

# 只显示本地文件系统
df -hl
```

**输出示例**:
```
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda1       100G   45G   50G  48% /
/dev/sdb1       500G  200G  275G  42% /home
```

---

## 快捷技巧

### 通配符

```bash
*       # 匹配任意字符
?       # 匹配单个字符
[abc]   # 匹配 a, b, 或 c
[a-z]   # 匹配 a 到 z
[!abc]  # 不匹配 a, b, c

# 示例
ls *.txt        # 所有txt文件
ls file?.txt    # file1.txt, file2.txt
ls [abc]*.txt   # a开头, b开头, c开头的txt文件
```

### Brace Expansion (花括号展开)

```bash
# 创建多个文件
touch file{1,2,3}.txt
# 等同于: touch file1.txt file2.txt file3.txt

# 创建序列
mkdir dir{1..10}
# 创建 dir1 到 dir10

# 组合使用
touch {a,b,c}{1..3}.txt
# 创建: a1.txt a2.txt a3.txt b1.txt b2.txt b3.txt c1.txt c2.txt c3.txt
```

### 有用的别名 (添加到 ~/.bashrc)

```bash
# 安全别名
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

# 增强 ls
alias ll='ls -lah'
alias la='ls -A'
alias l='ls -CF'

# 常用命令
alias ..='cd ..'
alias ...='cd ../..'
alias grep='grep --color=auto'

# Arch 特定
alias pacup='sudo pacman -Syu'
alias pacin='sudo pacman -S'
alias pacre='sudo pacman -R'
```

---

## 最佳实践

### 1. 安全操作
```bash
# 使用 -i 参数防止误删
rm -i important.txt

# 备份重要文件
cp important.txt{,.bak}

# 使用 trash-cli 而不是直接删除
sudo pacman -S trash-cli
trash file.txt
```

### 2. 提高效率
```bash
# 使用 Tab 补全
# 使用历史命令 (Ctrl+R)
# 使用别名简化命令
# 学习快捷键: Ctrl+A, Ctrl+E, Ctrl+U, Ctrl+K
```

### 3. 目录导航
```bash
# 使用 pushd/popd 管理目录栈
pushd /var/log
pushd /etc
popd  # 返回 /var/log
popd  # 返回原目录

# 使用 z/autojump 快速跳转
sudo pacman -S autojump
# 添加到 ~/.bashrc
[[ -s /etc/profile.d/autojump.sh ]] && source /etc/profile.d/autojump.sh
```

---

## 相关资源

- 下一章: [02-文本处理](../02-文本处理/)
- man 手册: `man ls`, `man find`, `man chmod`
- info 文档: `info coreutils`
- Arch Wiki: https://wiki.archlinux.org/title/Core_utilities
