# 04 - 网络工具

> 网络配置、诊断和数据传输工具

## 目录

- [网络配置](#网络配置)
- [网络诊断](#网络诊断)
- [文件传输](#文件传输)
- [DNS工具](#dns工具)
- [证书和安全](#证书和安全)
- [网络监控](#网络监控)
- [端口和防火墙](#端口和防火墙)

---

## 网络配置

### ip 命令（现代方式）

```bash
# 查看网络接口
ip addr show
ip a

# 查看接口统计信息
ip -s link

# 查看路由表
ip route show
ip r

# 查看邻居表（ARP缓存）
ip neigh show

# 启用/禁用接口
sudo ip link set eth0 up
sudo ip link set eth0 down

# 添加/删除 IP 地址
sudo ip addr add 192.168.1.100/24 dev eth0
sudo ip addr del 192.168.1.100/24 dev eth0

# 添加/删除默认网关
sudo ip route add default via 192.168.1.1 dev eth0
sudo ip route del default via 192.168.1.1

# 添加/删除路由
sudo ip route add 10.0.0.0/8 via 192.168.1.1
sudo ip route del 10.0.0.0/8

# 显示详细信息
ip -4 addr show
ip -6 addr show

# 以 JSON 格式输出
ip -j addr show
ip -j route show
```

### ifconfig（传统方式，已过时）

```bash
# 查看所有接口
ifconfig

# 查看指定接口
ifconfig eth0

# 启用/禁用接口
sudo ifconfig eth0 up
sudo ifconfig eth0 down

# 设置 IP 地址
sudo ifconfig eth0 192.168.1.100 netmask 255.255.255.0

# 设置 MTU
sudo ifconfig eth0 mtu 1500

# 查看接口统计
ifconfig -a
```

### netplan（Ubuntu 20.04+）

创建 `/etc/netplan/01-network-manager.yaml`:
```yaml
network:
  version: 2
  renderer: NetworkManager
  ethernets:
    eth0:
      dhcp4: false
      addresses:
        - 192.168.1.100/24
      gateway4: 192.168.1.1
      nameservers:
        addresses: [8.8.8.8, 8.8.4.4]
        search: [example.com]
```

应用配置:
```bash
sudo netplan apply
sudo netplan try
```

### systemd-networkd

创建配置文件 `/etc/systemd/network/20-wired.network`:
```ini
[Match]
Name=eth0

[Network]
Address=192.168.1.100/24
Gateway=192.168.1.1
DNS=8.8.8.8
DNS=8.8.4.4
DNSSEC=yes

[Link]
MTUBytes=1500
```

重启服务:
```bash
sudo systemctl restart systemd-networkd
```

---

## 网络诊断

### ping - 测试连通性

```bash
# 基本测试
ping google.com
ping -c 4 google.com  # 只ping 4次

# 连续ping
ping google.com

# 设置ping间隔
ping -i 2 google.com  # 2秒间隔

# 设置数据包大小
ping -s 100 google.com  # 100字节

# 详细输出
ping -v google.com

# IPv4/IPv6
ping -4 google.com
ping -6 google.com

# 以时间戳显示
ping -D google.com

# 停止ping
# Ctrl+C
```

### traceroute - 路径跟踪

```bash
# 跟踪路由
traceroute google.com

# 指定最大跳数
traceroute -m 15 google.com

# 设置超时时间
traceroute -w 2 google.com  # 2秒超时

# 使用特定源IP
traceroute -s 192.168.1.100 google.com

# 使用 ICMP（默认UDP）
traceroute -I google.com

# 跳数显示
traceroute -n google.com  # 不解析主机名

# 端口特定的探测
traceroute -p 443 google.com
```

### mtr - 实时 traceroute

```bash
# 实时跟踪（需要安装）
sudo pacman -S mtr
mtr google.com

# 仅输出模式
mtr --report google.com
mtr --report --report-cycles 10 google.com  # 10次报告

# 无主机名解析
mtr -n google.com

# 指定协议
mtr -r -c 10 -i 0.5 google.com  # 10次，0.5秒间隔
```

### netstat - 网络统计

```bash
# 显示所有监听端口
netstat -tuln

# 显示所有连接
netstat -an

# 显示路由表
netstat -r

# 显示接口统计
netstat -i

# 显示 TCP 连接
netstat -t

# 显示 UDP 连接
netstat -u

# 显示进程信息
netstat -tulnp  # 需要 root

# 统计信息
netstat -s

# IPv4 统计
netstat -4

# IPv6 统计
netstat -6
```

### ss - 现代网络统计（推荐）

```bash
# 显示所有监听端口
ss -tuln

# 显示所有连接
ss -an

# 显示 TCP 连接
ss -t

# 显示 UDP 连接
ss -u

# 显示进程信息
ss -tulnp

# 显示计时器
ss -o  # 显示重传计时器等

# 显示详细信息
ss -i  # 显示接口信息

# 显示内存信息
ss -m

# 显示 UDP 连接
ss -u

# IPv4 过滤
ss -4

# IPv6 过滤
ss -6

# 指定状态过滤
ss state listening  # 监听状态
ss state established  # 已连接状态
ss state time-wait  # 等待状态

# 指定源/目的端口过滤
ss sport = :80  # 源端口 80
ss dport = :80  # 目的端口 80
ss sport = :443 or dport = :443

# 组合过滤
ss -t state listening '( sport = :80 or sport = :443 )'
```

### telnet - 测试端口连通

```bash
# 测试端口是否开放
telnet google.com 80
telnet mail.example.com 25

# 退出 telnet
# Ctrl+] 然后输入 quit
```

### nc (netcat) - 网络瑞士军刀

```bash
# 测试端口连通
nc -zv google.com 80
nc -zv google.com 443
nc -zv mail.example.com 25

# 扫描多个端口
nc -zv -w 2 google.com 80 443 8080

# 监听端口
nc -l 12345

# 客户端
nc server.com 12345

# 发送文件
# 服务端: nc -l 12345 > file.txt
# 客户端: nc server.com 12345 < file.txt

# 端口扫描
nc -zv -w 1 server.com 21-80

# UDP 测试
nc -zv -u server.com 53
```

---

## 文件传输

### curl - HTTP 客户端

```bash
# 下载网页
curl http://example.com

# 保存到文件
curl -o page.html http://example.com/page.html

# 下载并重命名
curl -O http://example.com/page.html

# 多个文件
curl -O http://example.com/page1.html -O http://example.com/page2.html

# 断点续传
curl -C - -O http://example.com/largefile.zip

# 设置 User-Agent
curl -A "Mozilla/5.0" http://example.com

# 设置 cookies
curl -b "name=value" http://example.com
curl -c cookies.txt http://example.com  # 保存 cookies

# 使用 cookies
curl -b cookies.txt http://example.com

# 设置 HTTP 头
curl -H "Accept: application/json" http://api.example.com
curl -H "Content-Type: application/json" -X POST http://api.example.com

# 设置代理
curl -x http://proxy:8080 http://example.com
curl -x socks5://127.0.0.1:1080 http://example.com

# 跟随重定向
curl -L http://example.com

# 静默模式
curl -s http://example.com

# 详细输出
curl -v http://example.com

# 显示进度条
curl -# -O http://example.com/file.zip

# HTTP GET
curl http://api.example.com/data.json

# HTTP POST
curl -X POST -d "name=value&foo=bar" http://api.example.com/post
curl -X POST -H "Content-Type: application/json" -d '{"name":"value"}' http://api.example.com

# PUT 请求
curl -X PUT -d "data" http://api.example.com/endpoint

# DELETE 请求
curl -X DELETE http://api.example.com/resource

# 上传文件
curl -F "file=@/path/to/file.txt" http://api.example.com/upload

# 设置引用页面
curl -e "http://example.com/page" http://example.com/target

# 限制速度
curl --limit-rate 100K -O http://example.com/file.zip

# 超时设置
curl --connect-timeout 10 --max-time 300 http://example.com

# 基本认证
curl -u username:password http://example.com

# 证书认证
curl --cert cert.pem --key key.pem https://example.com

# SSL 验证
curl -k https://self-signed.example.com  # 跳过验证
curl --cacert ca.pem https://example.com  # 指定CA证书
```

### wget - 下载工具

```bash
# 下载文件
wget http://example.com/file.zip

# 下载并重命名
wget -O myfile.zip http://example.com/file.zip

# 断点续传
wget -c http://example.com/file.zip

# 后台下载
wget -b http://example.com/file.zip
tail -f wget-log  # 查看下载日志

# 限制速度
wget --limit-rate=100K http://example.com/file.zip

# 递归下载（整站）
wget -r http://example.com

# 递归下载指定深度
wget -r -l 2 http://example.com  # 2层深度

# 拒绝特定文件类型
wget -r --reject=pdf http://example.com

# 继续下载
wget -c http://example.com/file.zip

# 镜像网站
wget -m http://example.com

# 指定用户代理
wget --user-agent="Mozilla/5.0" http://example.com

# 设置 cookies
wget --load-cookies cookies.txt http://example.com

# 保存 cookies
wget --save-cookies cookies.txt http://example.com

# 忽略 robots.txt
wget -e robots=off http://example.com

# 不创建目录
wget -nH http://example.com/file.txt

# 限制重试次数
wget --tries=3 http://example.com

# 超时设置
wget --timeout=10 http://example.com

# 批量下载（从文件）
wget -i urls.txt

# HTTP 认证
wget --http-user=user --http-password=pass http://example.com

# FTP 认证
wget --ftp-user=user --ftp-password=pass ftp://example.com/file.zip

# 静默模式
wget -q http://example.com

# 详细输出
wget -v http://example.com

# HTTP 头
wget --header="Accept: application/json" http://api.example.com

# POST 请求
wget --post-data="name=value" http://api.example.com
```

### rsync - 同步文件

```bash
# 安装
sudo pacman -S rsync

# 基本同步（本地到本地）
rsync source/ destination/

# 同步并删除目的端多余文件
rsync -a --delete source/ destination/

# 显示进度
rsync -a --progress source/ destination/

# 压缩传输数据
rsync -az source/ destination/

# 显示详细信息
rsync -av source/ destination/

# 模拟运行（不实际执行）
rsync -avn --delete source/ destination/

# 排除特定文件
rsync -av --exclude='*.txt' source/ destination/

# 排除多个模式
rsync -av --exclude='*.txt' --exclude='*.log' source/ destination/

# 排除列表文件
rsync -av --exclude-from=exclude-list.txt source/ destination/

# 包含特定文件
rsync -av --include='*.txt' --exclude='*' source/ destination/

# 远程传输（需要SSH）
rsync -av source/ user@remote:/path/to/destination/

# 远程到本地
rsync -av user@remote:/path/to/source/ destination/

# 指定 SSH 端口
rsync -av -e "ssh -p 2222" source/ user@remote:/path/to/destination/

# 限制带宽
rsync -av --bwlimit=1000 source/ destination/

# 仅传输特定大小的文件
rsync -av --max-size=10M source/ destination/
rsync -av --min-size=1M source/ destination/

# 删除源文件（移动）
rsync -av --remove-source-files source/ destination/

# 链接保留
rsync -av -L source/ destination/

# 时间戳保留
rsync -avt source/ destination/

# 权限保留
rsync -avp source/ destination/

# 所有者保留
rsync -av --owner source/ destination/

# 组保留
rsync -av --group source/ destination/

# 设备文件保留
rsync -av --devices source/ destination/

# 符号链接保留
rsync -av --links source/ destination/

# 完整模式（归档模式）
rsync -a source/ destination/
# 等同于: -rlptgoD
```

### scp - 安全复制

```bash
# 本地到远程
scp file.txt user@remote:/path/to/destination/

# 远程到本地
scp user@remote:/path/to/file.txt ./destination/

# 递归复制目录
scp -r /local/directory user@remote:/remote/directory/

# 指定端口
scp -P 2222 file.txt user@remote:/path/

# 保留文件属性
scp -p file.txt user@remote:/path/

# 压缩传输
scp -C file.txt user@remote:/path/

# 多文件
scp file1.txt file2.txt user@remote:/path/

# 远程到远程（通过本地）
scp user1@remote1:/path/to/file user2@remote2:/path/to/

# 批量文件（通配符）
scp user@remote:/path/to/*.txt ./
```

### sftp - 安全 FTP

```bash
# 连接
sftp user@remote

# 查看帮助
sftp> help
sftp> ?

# 查看当前目录
sftp> pwd

# 查看远程目录
sftp> lpwd  # 本地pwd
sftp> ls

# 切换目录
sftp> cd /remote/directory
sftp> lcd /local/directory

# 下载文件
sftp> get remote-file.txt
sftp> get remote-file.txt local-file.txt

# 下载目录
sftp> -r get remote-directory/

# 上传文件
sftp> put local-file.txt
sftp> put local-file.txt remote-file.txt

# 上传目录
sftp> -r put local-directory/

# 删除远程文件
sftp> rm file.txt

# 创建目录
sftp> mkdir dirname

# 删除目录
sftp> rmdir dirname

# 查看帮助
sftp> help
```

### lftp - 高级 FTP 客户端

```bash
# 安装
sudo pacman -S lftp

# 连接
lftp user@remote

# 下载
lftp user@remote> get file.txt
lftp user@remote> mirror remote/ local/

# 上传
lftp user@remote> put file.txt
lftp user@remote> mirror local/ remote/

# 断点续传
lftp user@remote> pget file.txt
```

---

## DNS 工具

### dig - DNS 查询

```bash
# 基本查询
dig example.com

# 指定查询类型
dig example.com A     # IPv4地址
dig example.com AAAA  # IPv6地址
dig example.com MX    # 邮件交换记录
dig example.com TXT   # 文本记录
dig example.com NS    # 名称服务器
dig example.com CNAME # 别名记录
dig example.com SOA   # 权威记录

# 简化输出
dig +short example.com

# 显示完整响应
dig example.com +noall +answer

# 指定 DNS 服务器
dig @8.8.8.8 example.com
dig @1.1.1.1 example.com

# 反向查询
dig -x 8.8.8.8

# 查询 DKIM 记录
dig default._domainkey.example.com TXT

# 跟踪 DNS 解析路径
dig +trace example.com

# 设置超时时间
dig +timeout=3 example.com

# 迭代查询（默认）
dig +norec example.com

# 批量查询（从文件）
for domain in $(cat domains.txt); do dig $domain; done
```

### nslookup - 传统查询

```bash
# 基本查询
nslookup example.com

# 查询特定类型
nslookup -type=MX example.com

# 使用特定 DNS 服务器
nslookup example.com 8.8.8.8

# 交互模式
nslookup
> server 8.8.8.8
> example.com
> exit
```

### host - 简化查询

```bash
# 基本查询
host example.com

# 查询特定类型
host -t A example.com
host -t MX example.com

# 反向查询
host 8.8.8.8

# 列出所有类型
host -a example.com

# 使用特定 DNS 服务器
host example.com 8.8.8.8
```

---

## 证书和安全

### openssl - SSL/TLS 工具

```bash
# 测试 SSL 连接
openssl s_client -connect google.com:443

# 测试 SMTP SSL
openssl s_client -connect mail.example.com:465

# 测试 STARTTLS
openssl s_client -connect mail.example.com:587 -starttls smtp

# 测试 HTTPS 并显示证书
openssl s_client -connect example.com:443 -showcerts

# 测试 HTTP/2
openssl s_client -connect example.com:443 -alpn h2

# 生成随机数据
openssl rand -base64 32

# 生成散列值
echo "text" | openssl dgst -sha256

# Base64 编码
echo "text" | openssl base64

# Base64 解码
echo "dGV4dA==" | openssl base64 -d

# 生成密钥对
openssl genrsa -out private.key 2048

# 从私钥生成公钥
openssl rsa -in private.key -pubout -out public.key

# 查看证书信息
openssl x509 -in certificate.crt -text -noout

# 查看证书到期时间
openssl x509 -in certificate.crt -noout -enddate

# 生成自签名证书
openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout key.pem

# 从 CSR 生成证书
openssl x509 -req -in cert.csr -CA ca.pem -CAkey ca.key -CAcreateserial -out cert.pem -days 365
```

### ncat (netcat) - 简单服务器

```bash
# 监听端口
ncat -l 8080

# 发送文件
# 服务器: ncat -l 12345 < file.txt
# 客户端: ncat server.com 12345 > file.txt

# 简单 HTTP 服务器
ncat -l 8080 -k --exec "echo -e 'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello</h1>'"

# SSL 服务器
ncat -l 443 --ssl --cert cert.pem --key key.pem

# 端口扫描
ncat -z -w 1 server.com 80 443 22 21

# HTTP 客户端
ncat -C google.com 80
GET / HTTP/1.0

# WebSocket 连接
ncat -C --ssl google.com 443
GET / HTTP/1.1
Host: google.com
Upgrade: websocket
Connection: Upgrade
```

---

## 网络监控

### iftop - 实时流量监控

```bash
# 安装
sudo pacman -S iftop

# 基本监控
sudo iftop

# 指定接口
sudo iftop -i eth0

# 端口过滤
sudo iftop -i eth0 -P

# 显示历史
sudo iftop -i eth0 -P -t

# 显示详细流量
sudo iftop -i eth0 -i  # 显示详细信息
```

### nethogs - 进程流量监控

```bash
# 安装
sudo pacman -S nethogs

# 基本监控
sudo nethogs

# 指定接口
sudo nethogs eth0

# 显示详细信息
sudo nethogs -d 1  # 1秒更新

# 监控多个接口
sudo nethogs eth0 eth1
```

### tcpdump - 抓包工具

```bash
# 安装
sudo pacman -S tcpdump

# 抓包（需要 root）
sudo tcpdump

# 保存到文件
sudo tcpdump -w capture.pcap

# 读取文件
sudo tcpdump -r capture.pcap

# 指定接口
sudo tcpdump -i eth0

# 过滤主机
sudo tcpdump host 192.168.1.100
sudo tcpdump src 192.168.1.100
sudo tcpdump dst 192.168.1.100

# 过滤端口
sudo tcpdump port 80
sudo tcpdump src port 80

# 过滤协议
sudo tcpdump tcp
sudo tcpdump udp
sudo tcpdump icmp

# 组合过滤
sudo tcpdump host 192.168.1.100 and port 80
sudo tcpdump src 192.168.1.100 or dst 192.168.1.100

# 显示内容
sudo tcpdump -A  # ASCII
sudo tcpdump -X  # 十六进制

# 指定捕获数量
sudo tcpdump -c 100

# 显示时间戳
sudo tcpdump -tttt

# 显示详细信息
sudo tcpdump -v
```

### wireshark - 图形化抓包

```bash
# 安装
sudo pacman -S wireshark-qt

# 非 root 用户抓包
sudo usermod -a -G wireshark $USER
# 重新登录
```

---

## 端口和防火墙

### nmap - 端口扫描

```bash
# 安装
sudo pacman -S nmap

# 扫描单个端口
nmap -p 80 example.com

# 扫描常用端口
nmap example.com

# 扫描所有端口
nmap -p 1-65535 example.com

# TCP 扫描
nmap -sT example.com

# SYN 扫描（更快，更隐蔽）
nmap -sS example.com

# UDP 扫描
nmap -sU example.com

# 服务版本检测
nmap -sV example.com

# 操作系统检测
nmap -O example.com

# 组合扫描
nmap -sS -sV -O example.com

# 快速扫描
nmap -T4 example.com

# 详细输出
nmap -v example.com

# 探测脚本
nmap --script vuln example.com  # 漏洞检测
nmap --script ssl-enum-ciphers example.com  # SSL 证书检测

# 避开防火墙
nmap -f example.com  # 分片

# 从文件读取目标
nmap -iL targets.txt

# 输出格式
nmap -oN scan.txt example.com  # 普通
nmap -oX scan.xml example.com  # XML
nmap -oG scan.grep.txt example.com  # Grep
```

### ss - 快速端口检查

```bash
# 查看端口
ss -tuln | grep :80

# 统计端口
ss -tuln | wc -l

# 查看特定端口
ss -tulnp | grep :443

# 检查端口是否被占用
ss -tuln | grep :8080
```

### firewall-cmd - 防火墙

```bash
# 安装
sudo pacman -S firewalld
sudo systemctl enable --now firewalld

# 查看状态
sudo firewall-cmd --state
sudo firewall-cmd --list-all

# 查看区域
sudo firewall-cmd --get-zones

# 查看默认区域
sudo firewall-cmd --get-default-zone

# 查看活动区域
sudo firewall-cmd --get-active-zones

# 添加服务
sudo firewall-cmd --add-service=http --permanent
sudo firewall-cmd --add-service=https --permanent
sudo firewall-cmd --reload

# 删除服务
sudo firewall-cmd --remove-service=http --permanent

# 添加端口
sudo firewall-cmd --add-port=8080/tcp --permanent

# 删除端口
sudo firewall-cmd --remove-port=8080/tcp --permanent

# 查看服务列表
sudo firewall-cmd --list-services

# 查看端口列表
sudo firewall-cmd --list-ports

# 区域操作
sudo firewall-cmd --zone=public --add-service=http
```

### iptables - 传统防火墙

```bash
# 查看规则
sudo iptables -L

# 查看详细信息
sudo iptables -L -v -n

# 允许端口
sudo iptables -A INPUT -p tcp --dport 80 -j ACCEPT

# 允许服务
sudo iptables -A INPUT -p tcp -m state --state NEW -m tcp --dport 22 -j ACCEPT

# 丢弃端口
sudo iptables -A INPUT -p tcp --dport 22 -j DROP

# 保存规则
sudo iptables-save > /etc/iptables/rules.v4

# 从文件加载
sudo iptables-restore < /etc/iptables/rules.v4
```

---

## 实用脚本

### 网络速度测试

```bash
#!/bin/bash
# speed-test.sh

echo "网络速度测试..."

# 使用 curl 测试下载速度
echo "下载速度测试:"
curl -w "@-" -o /dev/null -s --max-time 10 <<'EOF'
     time_namelookup:  %{time_namelookup}\n
        time_connect:  %{time_connect}\n
     time_appconnect:  %{time_appconnect}\n
    time_pretransfer:  %{time_pretransfer}\n
       time_redirect:  %{time_redirect}\n
  time_starttransfer:  %{time_starttransfer}\n
                     ----------\n
          time_total:  %{time_total}\n
          size_total:  %{size_download} bytes\n
       speed_download:  %{speed_download} bytes/sec\n
EOF
```

### 检查网络连通性

```bash
#!/bin/bash
# check-network.sh

ping -c 1 8.8.8.8 > /dev/null
if [ $? -eq 0 ]; then
    echo "✅ 网络连接正常"
else
    echo "❌ 网络连接失败"
fi

# 检查 DNS
nslookup google.com > /dev/null
if [ $? -eq 0 ]; then
    echo "✅ DNS 解析正常"
else
    echo "❌ DNS 解析失败"
fi

# 检查端口
nc -zv -w 2 google.com 80 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✅ 端口 80 可达"
else
    echo "❌ 端口 80 不可达"
fi
```

### 批量端口扫描

```bash
#!/bin/bash
# port-scan.sh

if [ -z "$1" ]; then
    echo "用法: $0 <主机> [端口范围]"
    echo "示例: $0 example.com 1-1000"
    exit 1
fi

host=$1
ports=${2:-1-1000}

echo "扫描 $host 的端口: $ports"

nmap -p $ports -T4 -oN "$host-scan.txt" $host
```

---

## 相关资源

- [iptables 教程](https://wiki.archlinux.org/title/Iptables)
- [NetworkManager](https://wiki.archlinux.org/title/NetworkManager)
- [systemd-networkd](https://wiki.archlinux.org/title/Systemd-networkd)
- [netplan](https://netplan.io/)
- [nmap 手册](https://nmap.org/docs.html)
- [tcpdump 指南](https://www.tcpdump.org/manpages/tcpdump.1.html)
- 上一章: [03-系统管理](../03-系统管理/)
- 下一章: [05-开发工具](../05-开发工具/)
