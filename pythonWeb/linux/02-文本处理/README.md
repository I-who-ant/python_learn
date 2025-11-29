# 02 - 文本处理

> 强大的文本处理和过滤工具

## 目录

- [grep - 文本搜索](#grep---文本搜索)
- [sed - 流编辑器](#sed---流编辑器)
- [awk - 文本分析工具](#awk---文本分析工具)
- [基础文本工具](#基础文本工具)
- [文本比较工具](#文本比较工具)
- [编辑器](#编辑器)

---

## grep - 文本搜索

### 基本用法

```bash
grep [选项] 模式 [文件...]
```

### 常用参数

```bash
grep -i     # 忽略大小写 (ignore case)
grep -v     # 反向匹配 (invert)
grep -n     # 显示行号 (line number)
grep -c     # 只显示匹配行数 (count)
grep -l     # 只显示文件名 (files with matches)
grep -L     # 显示不匹配的文件名
grep -w     # 全字匹配 (word)
grep -x     # 整行匹配 (line)
grep -r     # 递归搜索 (recursive)
grep -R     # 递归搜索（跟随符号链接）
grep -A 3   # 显示匹配行及后3行 (after)
grep -B 3   # 显示匹配行及前3行 (before)
grep -C 3   # 显示匹配行及前后3行 (context)
grep -E     # 扩展正则表达式 (等同于 egrep)
grep -F     # 固定字符串匹配 (等同于 fgrep)
grep -P     # Perl正则表达式
```

### 实用示例

```bash
# 基础搜索
grep "error" file.log

# 忽略大小写搜索
grep -i "error" file.log

# 显示行号
grep -n "error" file.log

# 递归搜索目录
grep -r "TODO" ./src/

# 搜索多个文件
grep "error" *.log

# 反向匹配（显示不包含的行）
grep -v "success" file.log

# 全字匹配
grep -w "log" file.txt  # 只匹配 "log"，不匹配 "login"

# 显示上下文
grep -C 3 "error" file.log  # 显示匹配行前后各3行

# 只显示匹配的文件名
grep -l "error" *.log

# 统计匹配行数
grep -c "error" file.log

# 搜索多个模式
grep -e "error" -e "warning" file.log

# 使用管道
ps aux | grep python
cat file.txt | grep -i "important"

# 排除某些文件
grep -r "error" --exclude="*.min.js" ./src/

# 排除某些目录
grep -r "error" --exclude-dir="node_modules" ./

# 彩色高亮显示
grep --color=auto "error" file.log
```

### 正则表达式

```bash
# 基础正则
grep "^start" file.txt      # 以start开头的行
grep "end$" file.txt         # 以end结尾的行
grep "^$" file.txt           # 空行
grep "a.b" file.txt          # a和b之间有任意一个字符
grep "a*b" file.txt          # a出现0次或多次后跟b

# 扩展正则 (-E)
grep -E "error|warning" file.log    # 或
grep -E "file[0-9]+" file.txt       # 数字一次或多次
grep -E "^(error|warning)" file.log # 以error或warning开头
grep -E "[0-9]{3}" file.txt         # 恰好3个数字

# Perl正则 (-P)
grep -P "\d{3}-\d{4}" file.txt      # 电话号码格式
grep -P "(?<=@)\w+" emails.txt      # 捕获@后的内容
```

### egrep 和 fgrep

```bash
# egrep = grep -E (扩展正则)
egrep "error|warning" file.log

# fgrep = grep -F (快速固定字符串匹配，不解释正则)
fgrep "file.txt" logs.txt  # 字面匹配 "file.txt"，点号不作为通配符
```

### 实用组合

```bash
# 查找包含error但不包含warning的行
grep "error" file.log | grep -v "warning"

# 查找进程并高亮
ps aux | grep --color=auto python

# 统计代码行数（排除空行和注释）
grep -v "^$" file.py | grep -v "^#" | wc -l

# 在压缩文件中搜索
zgrep "error" file.log.gz

# 递归搜索并显示相对路径
grep -r "error" . | sed "s|$(pwd)/||"
```

---

## sed - 流编辑器

### 基本用法

```bash
sed [选项] '命令' 文件
```

### 常用参数

```bash
sed -n      # 安静模式，只打印匹配的行
sed -e      # 多个编辑命令
sed -i      # 直接修改文件（危险！）
sed -i.bak  # 修改前备份
sed -r      # 扩展正则表达式
sed -E      # 扩展正则（BSD/macOS）
```

### 替换操作

```bash
# 基本替换（只替换每行第一个）
sed 's/old/new/' file.txt

# 全局替换（替换所有）
sed 's/old/new/g' file.txt

# 忽略大小写替换
sed 's/old/new/gi' file.txt

# 只替换每行第2个匹配
sed 's/old/new/2' file.txt

# 使用不同分隔符
sed 's|/old/path|/new/path|g' file.txt
sed 's#old#new#g' file.txt

# 替换并打印改变的行
sed -n 's/old/new/p' file.txt

# 直接修改文件（备份）
sed -i.bak 's/old/new/g' file.txt
```

### 删除操作

```bash
# 删除第3行
sed '3d' file.txt

# 删除第1到5行
sed '1,5d' file.txt

# 删除最后一行
sed '$d' file.txt

# 删除空行
sed '/^$/d' file.txt

# 删除包含pattern的行
sed '/pattern/d' file.txt

# 删除注释行
sed '/^#/d' file.txt

# 删除空行和注释
sed -e '/^$/d' -e '/^#/d' file.txt
```

### 插入和追加

```bash
# 在第2行前插入
sed '2i\新的一行' file.txt

# 在第2行后追加
sed '2a\新的一行' file.txt

# 在匹配行前插入
sed '/pattern/i\插入的内容' file.txt

# 在文件开头插入
sed '1i\第一行' file.txt

# 在文件结尾追加
sed '$a\最后一行' file.txt
```

### 打印操作

```bash
# 打印第3行
sed -n '3p' file.txt

# 打印第1到5行
sed -n '1,5p' file.txt

# 打印奇数行
sed -n '1~2p' file.txt

# 打印偶数行
sed -n '2~2p' file.txt

# 打印匹配行
sed -n '/pattern/p' file.txt

# 打印匹配行到文件尾
sed -n '/pattern/,$p' file.txt
```

### 替换行

```bash
# 替换第3行
sed '3c\新内容' file.txt

# 替换匹配的行
sed '/pattern/c\新内容' file.txt

# 替换第1到5行
sed '1,5c\新内容' file.txt
```

### 高级用法

```bash
# 使用捕获组
sed 's/\(old\)/[\1]/g' file.txt  # 给old加方括号

# 交换两个词
sed 's/\(word1\) \(word2\)/\2 \1/' file.txt

# 删除行首空格
sed 's/^[ \t]*//' file.txt

# 删除行尾空格
sed 's/[ \t]*$//' file.txt

# 在每行前加行号
sed = file.txt | sed 'N;s/\n/\t/'

# 多命令组合
sed -e 's/old1/new1/g' -e 's/old2/new2/g' file.txt

# 条件替换
sed '/pattern/s/old/new/g' file.txt  # 只在包含pattern的行中替换
```

### 实用示例

```bash
# 配置文件修改
sed -i 's/DEBUG=True/DEBUG=False/' settings.py

# 批量重命名文件
for f in *.txt; do
    mv "$f" "$(echo $f | sed 's/old/new/')"
done

# 提取IP地址
sed -n 's/.*inet \([0-9.]*\).*/\1/p' /etc/network/interfaces

# 删除HTML标签
sed 's/<[^>]*>//g' file.html

# 转换Windows换行符为Unix
sed -i 's/\r$//' file.txt

# 在指定行后插入多行
sed '/pattern/a\
第一行\
第二行\
第三行' file.txt

# 注释掉某行
sed '/pattern/s/^/#/' file.txt

# 取消注释
sed 's/^#//' file.txt
```

---

## awk - 文本分析工具

### 基本用法

```bash
awk [选项] 'pattern { action }' 文件
```

### 基础概念

```bash
# 内置变量
$0      # 整行内容
$1, $2  # 第1列，第2列
NR      # 当前行号
NF      # 当前行的字段数
FS      # 字段分隔符（默认空格）
OFS     # 输出字段分隔符
RS      # 记录分隔符（默认换行）
ORS     # 输出记录分隔符
FILENAME # 当前文件名
```

### 基础示例

```bash
# 打印整行
awk '{print}' file.txt
awk '{print $0}' file.txt

# 打印第1列
awk '{print $1}' file.txt

# 打印多列
awk '{print $1, $3}' file.txt

# 打印最后一列
awk '{print $NF}' file.txt

# 打印倒数第二列
awk '{print $(NF-1)}' file.txt

# 格式化输出
awk '{print "Name:", $1, "Age:", $2}' file.txt

# 使用制表符分隔
awk '{print $1 "\t" $2}' file.txt
```

### 字段分隔符

```bash
# 使用冒号作为分隔符
awk -F: '{print $1}' /etc/passwd

# 使用多个分隔符
awk -F'[,:]' '{print $1}' file.txt

# 指定输出分隔符
awk 'BEGIN{OFS=","} {print $1, $2}' file.txt

# CSV处理
awk -F, '{print $1, $3}' data.csv
```

### 模式匹配

```bash
# 匹配包含pattern的行
awk '/pattern/' file.txt

# 匹配正则表达式
awk '/^[0-9]+$/' file.txt  # 只有数字的行

# 匹配第3列
awk '$3 ~ /pattern/' file.txt

# 不匹配
awk '$3 !~ /pattern/' file.txt

# 条件判断
awk '$3 > 100' file.txt  # 第3列大于100
awk '$1 == "error"' file.txt  # 第1列等于error
awk '$2 != "success"' file.txt  # 第2列不等于success

# 逻辑组合
awk '$1 == "error" && $3 > 100' file.txt
awk '$1 == "error" || $1 == "warning"' file.txt
```

### 范围操作

```bash
# 从匹配pattern1到pattern2的行
awk '/pattern1/,/pattern2/' file.txt

# 第5到10行
awk 'NR==5,NR==10' file.txt

# 第10行开始到结束
awk 'NR>=10' file.txt
```

### BEGIN 和 END

```bash
# BEGIN块在处理前执行
awk 'BEGIN {print "开始处理"} {print $0} END {print "处理结束"}' file.txt

# 统计行数
awk 'END {print NR}' file.txt

# 求和
awk '{sum += $1} END {print sum}' numbers.txt

# 平均值
awk '{sum += $1; count++} END {print sum/count}' numbers.txt

# 初始化变量
awk 'BEGIN {FS=":"; OFS="\t"} {print $1, $3}' /etc/passwd
```

### 数学计算

```bash
# 求和
awk '{sum += $1} END {print sum}' file.txt

# 最大值
awk 'BEGIN {max = 0} {if ($1 > max) max = $1} END {print max}' file.txt

# 最小值
awk 'NR==1 {min = $1} {if ($1 < min) min = $1} END {print min}' file.txt

# 平均值
awk '{sum += $1; n++} END {print sum/n}' file.txt

# 计数
awk '$3 > 100 {count++} END {print count}' file.txt
```

### 数组

```bash
# 统计单词出现次数
awk '{for(i=1; i<=NF; i++) count[$i]++} END {for(w in count) print w, count[w]}' file.txt

# 去重
awk '!seen[$0]++' file.txt

# 按第一列分组求和
awk '{sum[$1] += $2} END {for (i in sum) print i, sum[i]}' file.txt
```

### 函数

```bash
# 字符串长度
awk '{print length($1)}' file.txt

# 转大写
awk '{print toupper($0)}' file.txt

# 转小写
awk '{print tolower($0)}' file.txt

# 子串
awk '{print substr($1, 1, 3)}' file.txt  # 前3个字符

# 查找和替换
awk '{gsub(/old/, "new"); print}' file.txt

# 匹配位置
awk '{print index($0, "pattern")}' file.txt

# 分割字符串
awk '{split($0, arr, ":"); print arr[1]}' file.txt
```

### 实用示例

```bash
# 分析Apache日志
awk '{print $1}' access.log | sort | uniq -c | sort -rn  # 访问IP统计

# 计算文件大小总和
ls -l | awk '{sum += $5} END {print "Total:", sum/1024/1024, "MB"}'

# CSV第二列求和
awk -F, '{sum += $2} END {print sum}' data.csv

# 打印特定列
ps aux | awk '{print $2, $11}'  # PID和命令

# 格式化表格
awk '{printf "%-10s %-20s %10s\n", $1, $2, $3}' file.txt

# 统计用户shell使用情况
awk -F: '{count[$NF]++} END {for(s in count) print s, count[s]}' /etc/passwd

# 提取JSON字段
awk -F'"' '{print $4}' file.json  # 简单JSON

# 去除重复行（保持顺序）
awk '!seen[$0]++' file.txt

# 合并行
awk '{printf "%s ", $0} END {print ""}' file.txt

# 打印奇数行
awk 'NR % 2 == 1' file.txt

# 打印偶数行
awk 'NR % 2 == 0' file.txt

# 交换两列
awk '{print $2, $1}' file.txt

# 添加行号
awk '{print NR, $0}' file.txt
```

---

## 基础文本工具

### `cut` - 剪切文本

```bash
# 按字符位置
cut -c 1-5 file.txt        # 第1到5个字符
cut -c -5 file.txt         # 前5个字符
cut -c 5- file.txt         # 第5个字符到结尾

# 按字段
cut -f 1 file.txt          # 第1字段（默认Tab分隔）
cut -f 1,3 file.txt        # 第1和第3字段
cut -f 1-3 file.txt        # 第1到3字段

# 指定分隔符
cut -d: -f1 /etc/passwd    # 提取用户名
cut -d, -f2,4 data.csv     # CSV第2和第4列

# 示例
echo "apple,banana,cherry" | cut -d, -f2  # 输出: banana
```

### `sort` - 排序

```bash
sort file.txt              # 字典序排序
sort -n file.txt           # 数字排序
sort -r file.txt           # 反向排序
sort -u file.txt           # 去重排序
sort -k2 file.txt          # 按第2列排序
sort -t: -k3 -n /etc/passwd  # 指定分隔符和列

# 组合参数
sort -nrk2 file.txt        # 第2列数字倒序

# 示例
ls -l | sort -k5 -n        # 按文件大小排序
du -h | sort -hr           # 按大小倒序
```

### `uniq` - 去重

```bash
uniq file.txt              # 去除相邻重复行
uniq -c file.txt           # 统计重复次数
uniq -d file.txt           # 只显示重复的行
uniq -u file.txt           # 只显示不重复的行
uniq -i file.txt           # 忽略大小写

# 注意: uniq只能去除相邻的重复，通常与sort配合
sort file.txt | uniq       # 去除所有重复
sort file.txt | uniq -c    # 统计出现次数

# 示例
cat access.log | awk '{print $1}' | sort | uniq -c | sort -rn  # IP访问统计
```

### `wc` - 统计

```bash
wc file.txt                # 行数 单词数 字节数
wc -l file.txt             # 只统计行数
wc -w file.txt             # 只统计单词数
wc -c file.txt             # 只统计字节数
wc -m file.txt             # 统计字符数

# 示例
ls | wc -l                 # 文件数量
cat file.txt | wc -w       # 单词总数
find . -name "*.py" | wc -l  # Python文件数量
```

### `tr` - 字符转换

```bash
tr 'a-z' 'A-Z' < file.txt  # 转大写
tr 'A-Z' 'a-z' < file.txt  # 转小写
tr -d '\r' < file.txt      # 删除回车符
tr -s ' ' < file.txt       # 压缩空格
tr ' ' '\n' < file.txt     # 空格替换为换行

# 删除字符
echo "hello123" | tr -d '0-9'  # 删除数字: hello

# 字符集替换
echo "apple" | tr 'apl' 'xyz'  # xzzye

# 删除非字母数字字符
echo "hello, world!" | tr -cd '[:alnum:]'  # helloworld
```

### `paste` - 合并文件

```bash
paste file1.txt file2.txt  # 并排合并（Tab分隔）
paste -d, file1.txt file2.txt  # 使用逗号分隔
paste -s file.txt          # 单文件所有行合并为一行

# 示例
paste names.txt ages.txt   # 合并姓名和年龄
paste -d: /etc/passwd /etc/shadow  # 合并两个配置文件
```

### `join` - 基于共同字段合并

```bash
join file1.txt file2.txt   # 基于第一列合并
join -1 2 -2 1 file1.txt file2.txt  # file1第2列与file2第1列
join -t: file1.txt file2.txt  # 指定分隔符

# 注意: join要求文件已排序
```

### `expand` / `unexpand` - Tab转换

```bash
expand file.txt            # Tab转空格
expand -t 4 file.txt       # Tab转4个空格
unexpand file.txt          # 空格转Tab
```

---

## 文本比较工具

### `diff` - 文件比较

```bash
diff file1.txt file2.txt   # 基本比较
diff -u file1.txt file2.txt  # 统一格式
diff -c file1.txt file2.txt  # 上下文格式
diff -y file1.txt file2.txt  # 并排格式
diff -q file1.txt file2.txt  # 只显示是否不同
diff -r dir1/ dir2/        # 递归比较目录

# 忽略选项
diff -i file1.txt file2.txt  # 忽略大小写
diff -w file1.txt file2.txt  # 忽略空白
diff -b file1.txt file2.txt  # 忽略空白数量变化

# 生成补丁
diff -u old.txt new.txt > changes.patch

# 应用补丁
patch old.txt < changes.patch
```

### `comm` - 逐行比较排序文件

```bash
comm file1.txt file2.txt   # 三列输出: 仅1 仅2 共同
comm -12 file1.txt file2.txt  # 只显示共同行
comm -23 file1.txt file2.txt  # 只显示file1独有
comm -13 file1.txt file2.txt  # 只显示file2独有

# 注意: 文件必须已排序
sort file1.txt -o file1.txt
sort file2.txt -o file2.txt
comm file1.txt file2.txt
```

### `cmp` - 字节比较

```bash
cmp file1.txt file2.txt    # 二进制比较
cmp -l file1.txt file2.txt  # 详细显示差异
cmp -s file1.txt file2.txt  # 静默模式（用于脚本）

# 返回值:
# 0 = 相同
# 1 = 不同
# 2 = 错误
```

---

## 编辑器

### Vim 快速入门

**三种模式**:
- 命令模式 (默认)
- 插入模式 (i, a, o)
- 命令行模式 (:)

**基本操作**:
```bash
# 启动和退出
vim file.txt       # 打开文件
:q                 # 退出
:q!                # 强制退出（不保存）
:w                 # 保存
:wq 或 :x          # 保存并退出
ZZ                 # 保存并退出（命令模式）

# 移动
h j k l            # 左下上右
w                  # 下一个单词
b                  # 上一个单词
0                  # 行首
$                  # 行尾
gg                 # 文件开头
G                  # 文件结尾
:10                # 跳到第10行

# 插入
i                  # 当前位置插入
a                  # 光标后插入
o                  # 下一行插入
O                  # 上一行插入

# 删除
x                  # 删除字符
dd                 # 删除行
dw                 # 删除单词
d$                 # 删到行尾

# 复制粘贴
yy                 # 复制行
yw                 # 复制单词
p                  # 粘贴

# 撤销重做
u                  # 撤销
Ctrl+r             # 重做

# 搜索替换
/pattern           # 向下搜索
?pattern           # 向上搜索
n                  # 下一个匹配
N                  # 上一个匹配
:s/old/new/        # 替换当前行第一个
:s/old/new/g       # 替换当前行所有
:%s/old/new/g      # 替换整个文件
:%s/old/new/gc     # 替换并确认

# 可视模式
v                  # 字符选择
V                  # 行选择
Ctrl+v             # 块选择
```

**配置文件** `~/.vimrc`:
```vim
set number          " 显示行号
set autoindent      " 自动缩进
set tabstop=4       " Tab宽度
set expandtab       " Tab转空格
syntax on           " 语法高亮
set hlsearch        " 搜索高亮
```

### Nano 快速入门

```bash
nano file.txt      # 打开文件

# 快捷键 (^ 表示 Ctrl)
^O                 # 保存
^X                 # 退出
^K                 # 剪切行
^U                 # 粘贴
^W                 # 搜索
^R                 # 读入文件
^G                 # 帮助
```

---

## 实战示例

### 日志分析

```bash
# 统计访问最多的IP
cat access.log | awk '{print $1}' | sort | uniq -c | sort -rn | head -10

# 统计HTTP状态码
cat access.log | awk '{print $9}' | sort | uniq -c | sort -rn

# 提取特定时间段的日志
sed -n '/2025-11-28 10:00/,/2025-11-28 11:00/p' app.log

# 统计错误数量
grep -c "ERROR" app.log

# 提取邮箱地址
grep -oP '\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b' file.txt
```

### 数据处理

```bash
# CSV第二列求和
awk -F, '{sum += $2} END {print sum}' data.csv

# 提取JSON字段（简单）
grep -oP '(?<="name":")[^"]*' data.json

# 格式化输出
awk '{printf "%-20s %10s\n", $1, $2}' file.txt

# 去重并排序
sort -u file.txt

# 统计单词频率
cat file.txt | tr -s ' ' '\n' | sort | uniq -c | sort -rn
```

### 文本转换

```bash
# Unix to DOS 换行符
sed -i 's/$/\r/' file.txt

# DOS to Unix 换行符
sed -i 's/\r$//' file.txt
# 或
dos2unix file.txt  # 需要安装: sudo pacman -S dos2unix

# 删除空行
sed '/^$/d' file.txt

# 删除注释行
sed '/^#/d' file.txt

# 在每行前加前缀
sed 's/^/PREFIX: /' file.txt

# 在每行后加后缀
sed 's/$/ SUFFIX/' file.txt
```

---

## 性能考虑

**速度对比**（从快到慢）:
1. `grep` - 简单搜索最快
2. `awk` - 字段处理快
3. `sed` - 流处理快
4. Perl/Python - 功能强大但较慢

**选择建议**:
- 简单搜索: `grep`
- 简单替换: `sed`
- 列处理/计算: `awk`
- 复杂逻辑: Python/Perl

---

## 相关资源

- 上一章: [01-基础命令](../01-基础命令/)
- 下一章: [03-系统管理](../03-系统管理/)
- 正则表达式测试: https://regex101.com/
- Vim教程: `vimtutor` (命令行运行)
- GNU工具手册: `info grep`, `info sed`, `info awk`
