# 08 - Shell编程

> Bash 脚本编写和高级特性

## 目录

- [Bash基础](#bash基础)
- [变量和字符串](#变量和字符串)
- [数组](#数组)
- [条件判断](#条件判断)
- [循环结构](#循环结构)
- [函数](#函数)
- [输入输出](#输入输出)
- [管道和过滤器](#管道和过滤器)
- [正则表达式](#正则表达式)
- [高级特性](#高级特性)
- [实用脚本](#实用脚本)
- [调试技巧](#调试技巧)

---

## Bash基础

### 脚本结构

```bash
#!/bin/bash
# 这是注释

# 脚本从这里开始执行
echo "Hello, World!"
```

### 执行方式

```bash
# 1. 直接执行（需要可执行权限）
chmod +x script.sh
./script.sh

# 2. 用解释器执行
bash script.sh

# 3. 在当前shell执行
source script.sh
. script.sh
```

### 设置脚本安全

```bash
#!/bin/bash
set -euo pipefail
# -e: 任何命令失败就退出
# -u: 使用未定义变量时报错
# -o pipefail: 管道中任何失败都会导致整个管道失败

# 详细模式（调试）
# set -x

# 在脚本开头使用
set -euo pipefail

# 严格模式（更严格）
set -Eeuo pipefail
trap 'echo "Error occurred at line $LINENO"' ERR
```

### 行和注释

```bash
# 单行注释

: '
这是多行注释
可以跨越多行
但是不常用
'

# 或者使用 here doc
: <<'COMMENT'
This is a multi-line comment
COMMENT
```

---

## 变量和字符串

### 变量定义

```bash
# 基本赋值
name="John"
age=30

# 访问变量（加$）
echo $name
echo ${name}  # 大括号是更好的写法

# 只读变量
readonly PI=3.14159

# 删除变量（不常用）
unset name
```

### 字符串操作

```bash
# 获取长度
str="Hello World"
echo ${#str}  # 输出: 11

# 子字符串
echo ${str:0:5}    # 输出: Hello (从0开始，取5个字符)
echo ${str:6:5}    # 输出: World (从6开始，取5个字符)

# 删除前缀
file="/path/to/file.txt"
echo ${file##*/}   # 输出: file.txt (删除最长的*/前缀)
echo ${file#*/}    # 输出: path/to/file.txt (删除最短的*/前缀)

# 删除后缀
echo ${file%.*}    # 输出: /path/to/file (删除最短的.*后缀)
echo ${file%%.*}   # 输出: /path/to (删除最长的.*后缀)

# 替换
str="Hello World"
echo ${str/World/Linux}  # 输出: Hello Linux (替换第一个)
echo ${str//l/L}         # 输出: HeLLo WorLd (替换所有)

# 模式匹配删除
str="Hello World 2025"
echo ${str##*[0-9]}  # 输出: (删除开头到最后一个数字)

# 大小写转换（需要bash 4+）
echo ${str^^}  # 转为大写: HELLO WORLD 2025
echo ${str,,}  # 转为小写: hello world 2025
```

### 默认值

```bash
# 如果变量未定义，使用默认值
echo ${var:-default}  # var为空或未定义时使用default，但不赋值给var
echo ${var:=default}  # var为空或未定义时使用default，并赋值给var
echo ${var:?error}    # var为空或未定义时报错
echo ${var:+value}    # var已定义且不为空时返回value

# 示例
name=""
echo ${name:-"Unknown"}  # 输出: Unknown
echo $name               # 输出: (空，name未被改变)

echo ${name:="John"}     # 输出: John
echo $name               # 输出: John
```

### 字符串连接

```bash
# 简单连接
first="Hello"
second="World"
greeting="$first $second"
echo $greeting  # 输出: Hello World

# 使用大括号
echo "I am ${name}ing"  # 如果name=John，输出: Johning
```

### 数组和变量扩展

```bash
# 命令替换
current_date=$(date +%Y-%m-%d)
echo $current_date

# 数学运算
result=$((5 + 3))
echo $result  # 输出: 8

# 使用 expr（不推荐，更慢）
result=$(expr 5 + 3)

# 进制转换
echo $((16#FF))  # 输出: 255 (16进制FF转换为十进制)
```

---

## 数组

### 基本数组

```bash
# 定义数组
array=(one two three four)

# 访问元素
echo ${array[0]}  # 输出: one
echo ${array[1]}  # 输出: two

# 获取所有元素
echo ${array[@]}  # 输出: one two three four
echo ${array[*]}  # 输出: one two three four

# 获取数组长度
echo ${#array[@]}  # 输出: 4

# 获取单个元素长度
echo ${#array[2]}  # 输出: 5 (three的长度)
```

### 数组操作

```bash
# 修改元素
array[1]="TWO"
echo ${array[@]}  # 输出: one TWO three four

# 添加元素
array[4]="five"
echo ${array[@]}  # 输出: one TWO three four five

# 动态添加
array+=("six" "seven")
echo ${array[@]}  # 输出: one TWO three four five six seven

# 获取数组索引
echo ${!array[@]}  # 输出: 0 1 2 3 4 5 6
```

### 关联数组（字典）

```bash
# 启用关联数组（bash 4+）
declare -A person

# 定义键值对
person[name]="John"
person[age]="30"
person[city]="Beijing"

# 访问
echo ${person[name]}  # 输出: John

# 获取所有键
echo ${!person[@]}  # 输出: name age city

# 获取所有值
echo ${person[@]}  # 输出: John 30 Beijing
```

### 数组遍历

```bash
# 遍历元素
for item in "${array[@]}"; do
    echo $item
done

# 遍历索引
for i in "${!array[@]}"; do
    echo "$i: ${array[$i]}"
done

# 遍历关联数组
for key in "${!person[@]}"; do
    echo "$key: ${person[$key]}"
done
```

### 数组切片

```bash
array=(one two three four five)

# 从索引2开始，取2个元素
echo ${array[@]:2:2}  # 输出: three four

# 从索引1开始，取所有元素
echo ${array[@]:1}  # 输出: two three four five

# 负数索引（bash 4.2+）
echo ${array[@]: -2}  # 输出: four five (注意空格)
```

### 使用数组的注意事项

```bash
# 正确使用引号
echo "${array[@]}"  # 正确：保持元素完整
echo ${array[@]}    # 错误：可能导致分词

# 在函数中传递数组
func() {
    local arr=("$@")
    echo ${arr[@]}
}

# 返回数组
return_array() {
    echo "item1" "item2" "item3"
}

result=($(return_array))
echo ${result[@]}
```

---

## 条件判断

### if-else 结构

```bash
# 基本 if 语句
if [ 条件 ]; then
    echo "条件为真"
fi

# if-else
if [ 条件 ]; then
    echo "真"
else
    echo "假"
fi

# if-elif-else
if [ 条件1 ]; then
    echo "条件1为真"
elif [ 条件2 ]; then
    echo "条件2为真"
else
    echo "所有条件都为假"
fi
```

### 条件测试类型

**数值比较**:
```bash
# 使用 -eq, -ne, -lt, -le, -gt, -ge
if [ $a -eq $b ]; then  # 等于
    echo "a等于b"
fi

if [ $a -ne $b ]; then  # 不等于
    echo "a不等于b"
fi

if [ $a -lt $b ]; then  # 小于
    echo "a小于b"
fi

if [ $a -le $b ]; then  # 小于等于
    echo "a小于等于b"
fi

if [ $a -gt $b ]; then  # 大于
    echo "a大于b"
fi

if [ $a -ge $b ]; then  # 大于等于
    echo "a大于等于b"
fi
```

**字符串比较**:
```bash
# 使用 =, !=, <, >, -z, -n
if [ "$str1" = "$str2" ]; then  # 等于
    echo "字符串相等"
fi

if [ "$str1" != "$str2" ]; then  # 不等于
    echo "字符串不相等"
fi

if [ -z "$str" ]; then  # 字符串长度为0
    echo "字符串为空"
fi

if [ -n "$str" ]; then  # 字符串长度非0
    echo "字符串非空"
fi

# 字典序比较
if [[ "$str1" < "$str2" ]]; then
    echo "str1在str2前面"
fi

# 避免 < > 被解释为重定向，需要 [[ ]]
```

**文件测试**:
```bash
if [ -f file.txt ]; then      # 是普通文件
    echo "file.txt存在且是普通文件"
fi

if [ -d /path/to/dir ]; then  # 是目录
    echo "目录存在"
fi

if [ -e file.txt ]; then      # 文件或目录存在
    echo "存在"
fi

if [ -r file.txt ]; then      # 可读
    echo "可读"
fi

if [ -w file.txt ]; then      # 可写
    echo "可写"
fi

if [ -x file.txt ]; then      # 可执行
    echo "可执行"
fi

if [ -L file.txt ]; then      # 是符号链接
    echo "是符号链接"
fi

if [ -s file.txt ]; then      # 文件存在且非空
    echo "文件非空"
fi

if [ file1 -nt file2 ]; then  # file1比file2新
    echo "file1更新"
fi

if [ file1 -ot file2 ]; then  # file1比file2旧
    echo "file1更旧"
fi
```

### 逻辑运算符

**使用 [] 的情况**:
```bash
if [ 条件1 ] && [ 条件2 ]; then  # 逻辑与
    echo "条件1和条件2都为真"
fi

if [ 条件1 ] || [ 条件2 ]; then  # 逻辑或
    echo "条件1或条件2至少有一个为真"
fi

if ! [ 条件 ]; then  # 逻辑非
    echo "条件为假"
fi
```

**使用 [[ ]] 的情况（推荐）**:
```bash
if [[ 条件1 && 条件2 ]]; then
    echo "两个条件都为真"
fi

if [[ 条件1 || 条件2 ]]; then
    echo "至少一个条件为真"
fi

if ! [[ 条件 ]]; then
    echo "条件为假"
fi

# [[ ]] 允许使用 &&
# [ ] 允许使用 -a -o（已弃用）
```

**多条件组合**:
```bash
# 复杂条件
if [[ $age -ge 18 && $age -le 65 ]]; then
    echo "在法定工作年龄范围内"
fi

if [[ $status = "active" || $status = "pending" ]]; then
    echo "状态正常"
fi
```

### case 语句

```bash
case $variable in
    pattern1)
        echo "匹配pattern1"
        ;;
    pattern2)
        echo "匹配pattern2"
        ;;
    pattern3|pattern4)  # 或条件
        echo "匹配pattern3或pattern4"
        ;;
    *)
        echo "默认匹配"
        ;;
esac
```

**示例**:
```bash
# 检查文件扩展名
case $filename in
    *.txt)
        echo "文本文件"
        ;;
    *.jpg|*.jpeg|*.png)
        echo "图片文件"
        ;;
    *.mp3|*.wav|*.flac)
        echo "音频文件"
        ;;
    *)
        echo "未知文件类型"
        ;;
esac

# 检查参数
case $1 in
    start)
        echo "启动服务"
        ;;
    stop)
        echo "停止服务"
        ;;
    restart)
        echo "重启服务"
        ;;
    *)
        echo "用法: $0 {start|stop|restart}"
        exit 1
        ;;
esac
```

---

## 循环结构

### for 循环

**基本 for 循环**:
```bash
for i in 1 2 3 4 5; do
    echo $i
done

# 输出: 1 2 3 4 5

# 更简洁的写法
for i in {1..5}; do
    echo $i
done

# 指定步长
for i in {1..10..2}; do  # 1,3,5,7,9
    echo $i
done

# 遍历字符串
for word in "hello" "world"; do
    echo $word
done
```

**C 风格 for 循环**:
```bash
for ((i=0; i<10; i++)); do
    echo $i
done

# 多个变量
for ((i=0, j=10; i<10; i++, j--)); do
    echo "i=$i, j=$j"
done
```

**遍历数组**:
```bash
array=(one two three)
for item in "${array[@]}"; do
    echo $item
done

# 遍历索引
for i in "${!array[@]}"; do
    echo "$i: ${array[$i]}"
done
```

**遍历文件**:
```bash
# 当前目录的所有.txt文件
for file in *.txt; do
    echo "处理文件: $file"
done

# 递归遍历
for file in $(find . -type f); do
    echo "找到文件: $file"
done

# 空格安全的文件遍历（推荐）
while IFS= read -r file; do
    echo "处理: $file"
done < <(find . -type f)

# 读取目录内容（注意特殊字符）
for file in *; do
    [ -e "$file" ] || continue
    echo "处理: $file"
done
```

**命令输出**:
```bash
# 遍历命令输出
for line in $(ls -l); do
    echo $line
done

# 更安全（保留换行）
while IFS= read -r line; do
    echo $line
done < <(ls -l)
```

### while 循环

```bash
# 基本 while
count=0
while [ $count -lt 5 ]; do
    echo "count = $count"
    count=$((count + 1))
done

# 无限循环
while true; do
    echo "无限循环"
    sleep 1
done

# 从文件读取
while IFS= read -r line; do
    echo "读取: $line"
done < file.txt

# 从标准输入读取
while IFS= read -r line; do
    echo "输入: $line"
done

# 读取直到 EOF
cat file.txt | while IFS= read -r line; do
    echo $line
done

# 读取分区信息
while read -r filesystem size used avail use mount; do
    echo "文件系统: $filesystem, 使用率: $use";
done < <(df -h | tail -n +2)
```

### until 循环

```bash
# 直到条件为真才停止
count=10
until [ $count -lt 1 ]; do
    echo "倒计时: $count"
    count=$((count - 1))
done
```

### 循环控制

```bash
# break - 跳出循环
for i in {1..10}; do
    if [ $i -eq 5 ]; then
        break
    fi
    echo $i
done
# 输出: 1 2 3 4

# continue - 跳过当前迭代
for i in {1..5}; do
    if [ $i -eq 3 ]; then
        continue
    fi
    echo $i
done
# 输出: 1 2 4 5

# 嵌套循环
for i in {1..3}; do
    for j in {1..3}; do
        if [ $i -eq 2 ] && [ $j -eq 2 ]; then
            continue 2  # 跳过外层循环的当前迭代
        fi
        echo "i=$i, j=$j"
    done
done

# break n - 跳出 n 层循环
# continue n - 跳过 n 层循环的当前迭代
```

### select 循环（交互式菜单）

```bash
# 创建交互式菜单
echo "请选择:"
select option in "选项1" "选项2" "选项3" "退出"; do
    case $option in
        "选项1")
            echo "选择了选项1"
            ;;
        "选项2")
            echo "选择了选项2"
            ;;
        "选项3")
            echo "选择了选项3"
            ;;
        "退出")
            echo "退出菜单"
            break
            ;;
        *)
            echo "无效选择"
            ;;
    esac
done
```

---

## 函数

### 基本函数

```bash
# 定义函数
greet() {
    echo "Hello, World!"
}

# 调用函数
greet  # 输出: Hello, World!
```

### 带参数函数

```bash
greet() {
    echo "Hello, $1!"
}

greet "John"  # 输出: Hello, John!

# 访问参数
echo "第一个参数: $1"
echo "第二个参数: $2"
echo "所有参数: $@"
echo "参数个数: $#"
echo "脚本名称: $0"
```

### 返回值

```bash
# 函数只能返回数字 (0-255)
check_number() {
    if [ $1 -gt 10 ]; then
        return 0  # 成功
    else
        return 1  # 失败
    fi
}

check_number 15
if [ $? -eq 0 ]; then
    echo "数字大于10"
else
    echo "数字不大于10"
fi
```

### 字符串返回值

```bash
# 使用 echo 返回字符串
get_name() {
    echo "John"
}

name=$(get_name)
echo $name  # 输出: John
```

### 局部变量

```bash
test_local() {
    local var="local"
    echo $var
}

var="global"
echo $var  # 输出: global
test_local  # 输出: local
echo $var  # 输出: global (未改变)
```

### 函数数组

```bash
get_array() {
    local arr=("one" "two" "three")
    echo "${arr[@]}"
}

result=($(get_array))
echo ${result[@]}  # 输出: one two three

# 或者使用 nameref
get_array_ref() {
    local -n ref=$1
    ref=("one" "two" "three")
}

myarray=()
get_array_ref myarray
echo ${myarray[@]}  # 输出: one two three
```

### 递归函数

```bash
factorial() {
    local n=$1
    if [ $n -le 1 ]; then
        echo 1
    else
        local prev=$(factorial $((n-1)))
        echo $((n * prev))
    fi
}

factorial 5  # 输出: 120
```

### 函数和脚本

```bash
# 脚本的函数列表
script_functions() {
    echo "这是一个脚本函数"
}

# 检查函数是否已定义
if declare -f script_functions > /dev/null; then
    echo "函数已定义"
    script_functions
fi

# 列出所有函数
declare -F
```

---

## 输入输出

### 读取用户输入

```bash
# 基本输入
echo "请输入你的名字:"
read name
echo "你好, $name"

# 一行输入多个值
echo "请输入名字和年龄:"
read name age
echo "名字: $name, 年龄: $age"

# 隐藏输入（密码）
echo "请输入密码:"
read -s password
echo "密码已读取"

# 设置提示符
read -p "请输入用户名: " username
read -sp "请输入密码: " password
echo

# 限时输入
read -t 5 name
if [ $? -eq 0 ]; then
    echo "你好, $name"
else
    echo "超时"
fi

# 从文件读取
while IFS= read -r line; do
    echo "读取: $line"
done < input.txt

# 从标准输入读取
while IFS= read -r line; do
    echo $line
done
```

### 输出

```bash
# 基本输出
echo "Hello"
printf "Hello\n"

# 格式化输出
printf "姓名: %s, 年龄: %d\n" "John" 30

# 转义序列
echo -e "第一行\n第二行"  # -e 启用转义
printf "红色文本: \033[31m红色\033[0m\n"

# 不换行
echo -n "请输入: "
printf "请输入: "

# 输出到文件
echo "内容" > file.txt
echo "追加内容" >> file.txt

printf "格式化内容\n" > file.txt

# 输出到标准错误
echo "错误信息" >&2
printf "错误: %s\n" "出错了" >&2

# 输出到变量
var=$(echo "Hello")
```

### 文件描述符

```bash
# 标准文件描述符
# 0: 标准输入 (stdin)
# 1: 标准输出 (stdout)
# 2: 标准错误 (stderr)

# 重定向输出
echo "正常输出" > output.txt
echo "错误输出" 2> error.txt
echo "输出和错误" > all.txt 2>&1

# 追加重定向
echo "更多内容" >> output.txt

# 忽略错误
command 2> /dev/null

# 重定向到标准输出和文件
command | tee output.txt

# 从文件重定向输入
command < input.txt

# here document (多行输入)
cat << EOF
这是多行文本
可以包含变量: $VAR
EOF

# here string (单行)
cat <<< "这是字符串"
```

### tee 命令

```bash
# 输出到文件和屏幕
echo "内容" | tee output.txt

# 追加到文件
echo "更多内容" | tee -a output.txt

# 多个文件
echo "内容" | tee file1.txt file2.txt

# 静默模式（不显示）
echo "内容" | tee -s output.txt
```

---

## 管道和过滤器

### 管道基础

```bash
# 基本管道
ls | grep txt

# 多个管道
ps aux | grep python | grep -v grep

# 管道重定向
command1 | command2 | command3 > output.txt

# xargs - 将管道输入转换为命令行参数
find . -name "*.txt" | xargs rm
find . -name "*.txt" -print0 | xargs -0 rm  # 安全处理特殊字符

# xargs 选项
ls | xargs -n 2  # 每行最多2个参数
ls | xargs -I {} cp {} /backup/  # 使用占位符
```

### 常用过滤器

**head/tail**:
```bash
# 显示前10行
ps aux | head

# 显示最后10行
ls -t | head

# 显示前5行
cat file.txt | head -5

# 显示最后5行
tail file.txt

# 实时跟踪文件
tail -f /var/log/syslog
```

**grep**:
```bash
# 搜索文本
cat file.txt | grep "pattern"

# 显示行号
cat file.txt | grep -n "pattern"

# 忽略大小写
cat file.txt | grep -i "pattern"

# 显示不匹配的行
cat file.txt | grep -v "pattern"

# 使用正则
echo "abc123" | grep -E "[0-9]+"
```

**sort**:
```bash
# 排序
cat file.txt | sort

# 数字排序
echo -e "3\n1\n2" | sort -n

# 反向排序
cat file.txt | sort -r

# 去重排序
cat file.txt | sort -u

# 按列排序
ls -l | sort -k5 -n
```

**uniq**:
```bash
# 去除重复行（要求相邻）
cat file.txt | uniq

# 统计出现次数
cat file.txt | sort | uniq -c

# 只显示重复的行
cat file.txt | sort | uniq -d

# 只显示唯一的行
cat file.txt | sort | uniq -u
```

**wc**:
```bash
# 统计行数
cat file.txt | wc -l

# 统计单词数
cat file.txt | wc -w

# 统计字符数
cat file.txt | wc -m

# 统计字节数
cat file.txt | wc -c
```

**cut**:
```bash
# 提取列
echo "a,b,c,d" | cut -d',' -f2  # 输出: b

# 提取字符范围
echo "12345" | cut -c2-4  # 输出: 234

# 提取字节范围
echo "hello" | cut -b2-4
```

**tr**:
```bash
# 转换大小写
echo "HELLO" | tr 'A-Z' 'a-z'  # 输出: hello

# 删除字符
echo "hello123" | tr -d '0-9'  # 输出: hello

# 压缩字符
echo "aaabbbccc" | tr -s 'a-c'  # 输出: abc
```

---

## 正则表达式

### 基本正则

```bash
# grep 使用正则
echo "abc123" | grep "[0-9]"     # 匹配数字
echo "abc123" | grep -E "[0-9]+" # 匹配一个或多个数字

# 特殊字符
.   # 任意单个字符
^   # 行首
$   # 行尾
*   # 零个或多个前面的元素
+   # 一个或多个前面的元素
?   # 零个或一个前面的元素
[]  # 字符集
|   # 或
()  # 分组

# 示例
echo "hello" | grep "^h"      # 以h开头
echo "hello" | grep "o$"      # 以o结尾
echo "hllo" | grep "h.llo"    # 匹配
echo "goooal" | grep "go+al"  # 匹配
echo "color" | grep "colou?r" # 匹配（可选u）
echo "bat" | grep "b[ae]t"    # 匹配bet或bat
echo "123" | grep "^[0-9]+$"  # 纯数字行
```

### 扩展正则

```bash
# 使用 -E 或 egrep
echo "abcdef" | grep -E "(abc|def)"  # abc或def

# 重复次数
echo "ab" | grep -E "a{1,2}b"       # a出现1-2次
echo "aab" | grep -E "a{2}b"
```

### sed 和正则

```bash
# 替换
echo "hello" | sed 's/hello/goodbye/'  # 输出: goodbye

# 贪婪匹配
echo "abc xyz abc" | sed 's/a.*c/REPLACED/'  # 输出: REPLACED abc

# 保留分组
echo "John Doe" | sed 's/\([A-Z][a-z]*\) \([A-Z][a-z]*\)/\2, \1/'
# 输出: Doe, John
```

### awk 和正则

```bash
# 模式匹配
awk '/regex/ {print}' file.txt

# 列匹配
awk '$1 ~ /^[0-9]+/ {print}' file.txt  # 第一列是数字的行

# 不匹配
awk '$1 !~ /regex/ {print}' file.txt
```

---

## 高级特性

### 参数展开

```bash
# 基本参数展开
${var}

# 默认值
${var:-default}
${var:=default}
${var:?error}
${var:+value}

# 字符串操作
${#var}          # 长度
${var:start}     # 子串
${var:start:len} # 指定长度
${var#pattern}   # 删除前缀（最短）
${var##pattern}  # 删除前缀（最长）
${var%pattern}   # 删除后缀（最短）
${var%%pattern}  # 删除后缀（最长）
${var/pattern/str}  # 替换（第一个）
${var//pattern/str} # 替换（全部）
${var/#pattern/str} # 替换前缀
${var/%pattern/str} # 替换后缀
```

### 模式匹配

```bash
file="/path/to/file.txt"

# 提取文件名
${file##*/}  # file.txt
${file%.*}   # /path/to/file

# 提取目录
${file%/*}   # /path/to

# 提取扩展名
${file##*.}  # txt

# 提取名称（无扩展名）
${file##*/.*}  # file
```

### 进程替换

```bash
# 命令输出作为文件
diff <(command1) <(command2)

# 示例
diff <(ls /bin) <(ls /usr/bin)

# while 循环读取命令输出
while IFS= read -r line; do
    echo $line
done < <(command)

# 避免子shell的问题
value=0
while IFS= read -r line; do
    value=1
    echo $line
done < <(echo "test")
echo "value=$value"
```

### 协程

```bash
# 同时执行两个进程
coproc proc { while read line; do echo "Got: $line"; done; }

echo "Sending" >&${proc[1]}
read response <&${proc[0]}
echo "Got response: $response"

exec {proc[1}>&-
```

### 调试脚本

```bash
# 启用调试模式
set -x  # 显示执行的命令
set +x  # 禁用调试

# 调试特定行
echo "正常"
set -x
echo "这条命令会显示"
set +x
echo "正常"

# 调试函数
debug_func() {
    set -x
    echo "函数内"
    set +x
}
```

### trap - 信号处理

```bash
# 脚本退出时执行
cleanup() {
    echo "清理临时文件"
    rm -f /tmp/tempfile
    exit 0
}
trap cleanup EXIT

# Ctrl+C 时执行
trap 'echo "按了Ctrl+C"; exit 1' INT

# 错误时执行
trap 'echo "发生错误，退出"; exit 1' ERR

# 调试模式
trap 'echo "执行第 $LINENO 行"' DEBUG

# 查看trap
trap -p
```

### eval - 动态执行

```bash
# 动态构建命令
cmd="ls -l"
eval $cmd

# 动态变量名
name="var1"
${name}_value=10
echo $var1_value  # 输出: 10

# 避免eval（使用数组和关联数组）
# 好的做法：使用数组
commands=("ls -l" "cat file.txt" "echo done")
for cmd in "${commands[@]}"; do
    $cmd
done
```

### 数组和字符串

```bash
# 分割字符串为数组
string="apple,banana,orange"
IFS=',' read -ra fruits <<< "$string"
echo ${fruits[0]}  # apple

# 合并数组为字符串
array=("one" "two" "three")
IFS=',' echo "${array[*]}"  # 输出: one,two,three
```

---

## 实用脚本

### 系统信息脚本

```bash
#!/bin/bash
# system-info.sh

set -euo pipefail

print_info() {
    local label=$1
    local value=$2
    printf "%-20s: %s\n" "$label" "$value"
}

echo "=== 系统信息 ==="
echo

print_info "主机名" "$(hostname)"
print_info "内核版本" "$(uname -r)"
print_info "操作系统" "$(cat /etc/os-release | grep PRETTY_NAME | cut -d'"' -f2)"
print_info "运行时间" "$(uptime -p)"
print_info "当前时间" "$(date)"

echo
echo "=== 硬件信息 ==="
echo

if command -v lscpu > /dev/null; then
    print_info "CPU型号" "$(lscpu | grep "Model name" | cut -d':' -f2 | xargs)"
    print_info "CPU核心数" "$(nproc)"
fi

if [ -f /proc/meminfo ]; then
    total_mem=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    free_mem=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
    print_info "总内存" "$((total_mem / 1024)) MB"
    print_info "可用内存" "$((free_mem / 1024)) MB"
fi

echo
echo "=== 磁盘使用 ==="
echo

df -h | tail -n +2 | while read -r filesystem size used avail use mount; do
    print_info "$mount" "$use used ($avail free)"
done
```

### 备份脚本

```bash
#!/bin/bash
# backup.sh

set -euo pipefail

SOURCE_DIR=${1:-.}
BACKUP_DIR=${2:-./backups}
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="backup_${TIMESTAMP}.tar.gz"

echo "开始备份: $SOURCE_DIR -> $BACKUP_DIR/$BACKUP_FILE"

# 创建备份目录
mkdir -p "$BACKUP_DIR"

# 创建备份
tar -czf "$BACKUP_DIR/$BACKUP_FILE" \
    --exclude='*.tmp' \
    --exclude='*.log' \
    --exclude='.git' \
    "$SOURCE_DIR"

echo "备份完成: $BACKUP_DIR/$BACKUP_FILE"

# 删除7天前的备份
find "$BACKUP_DIR" -name "backup_*.tar.gz" -mtime +7 -delete

echo "清理完成"
```

### 批量重命名脚本

```bash
#!/bin/bash
# batch-rename.sh

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "用法: $0 <目录> <旧扩展名> <新扩展名>"
    echo "示例: $0 ./files txt md"
    exit 1
fi

DIR=${1:-$PWD}
OLD_EXT=$2
NEW_EXT=$3

cd "$DIR"

count=0
for file in *."$OLD_EXT"; do
    if [ -e "$file" ]; then
        new_name="${file%.*}.$NEW_EXT"
        mv -v "$file" "$new_name"
        ((count++))
    fi
done

echo "重命名了 $count 个文件"
```

### 进程监控脚本

```bash
#!/bin/bash
# monitor.sh

set -euo pipefail

SERVICE_NAME=$1
CHECK_INTERVAL=${2:-5}

check_service() {
    if pgrep -f "$SERVICE_NAME" > /dev/null; then
        echo "[$(date)] $SERVICE_NAME 正在运行"
        return 0
    else
        echo "[$(date)] $SERVICE_NAME 未运行，正在重启..."
        # 添加重启命令，例如:
        # systemctl start $SERVICE_NAME
        return 1
    fi
}

echo "开始监控: $SERVICE_NAME"
echo "检查间隔: ${CHECK_INTERVAL}秒"
echo

while true; do
    check_service
    sleep "$CHECK_INTERVAL"
done
```

### 日志分析脚本

```bash
#!/bin/bash
# log-analyzer.sh

set -euo pipefail

LOG_FILE=${1:-/var/log/syslog}

if [ ! -f "$LOG_FILE" ]; then
    echo "错误: 文件 $LOG_FILE 不存在"
    exit 1
fi

echo "=== 日志分析报告 ==="
echo "文件: $LOG_FILE"
echo "总行数: $(wc -l < "$LOG_FILE")"
echo

echo "=== 错误统计 ==="
error_count=$(grep -i error "$LOG_FILE" | wc -l)
warning_count=$(grep -i warning "$LOG_FILE" | wc -l)
echo "错误: $error_count"
echo "警告: $warning_count"
echo

echo "=== 顶级错误源 ==="
grep -i error "$LOG_FILE" | \
    grep -oE '\[[^]]+\]' | \
    sort | uniq -c | sort -rn | head -5
echo

echo "=== 今日错误 ==="
today=$(date +%Y-%m-%d)
grep "$today" "$LOG_FILE" | grep -i error | tail -10
```

### 包管理器脚本

```bash
#!/bin/bash
# package-update.sh

set -euo pipefail

echo "=== 更新 Arch Linux 系统 ==="
echo

# 更新包数据库
echo "1. 更新包数据库..."
sudo pacman -Sy

# 升级系统
echo
echo "2. 升级系统..."
sudo pacman -Su --noconfirm

# 清理孤立包
echo
echo "3. 清理孤立包..."
sudo pacman -Rns $(pacman -Qtdq) --noconfirm 2>/dev/null || true

# 清理缓存
echo
echo "4. 清理包缓存..."
sudo paccache -r

# 更新AUR包（如果使用yay）
if command -v yay > /dev/null; then
    echo
    echo "5. 更新AUR包..."
    yay -Sua --noconfirm
fi

echo
echo "=== 系统更新完成 ==="
```

### Web 服务器检查脚本

```bash
#!/bin/bash
# check-server.sh

URLS=(
    "https://example1.com"
    "https://example2.com"
    "https://example3.com"
)

echo "=== 服务器状态检查 ==="
echo

for url in "${URLS[@]}"; do
    echo "检查: $url"

    # 检查HTTP状态
    status=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 "$url")

    # 检查响应时间
    time_total=$(curl -s -o /dev/null -w "%{time_total}" --max-time 10 "$url")

    # 检查SSL证书
    ssl_status=$(echo | openssl s_client -servername "${url#https://}" -connect "${url#https://}:443" 2>/dev/null | openssl x509 -noout -dates 2>/dev/null)

    if [ "$status" -eq 200 ]; then
        echo "  ✓ HTTP 200 (响应时间: ${time_total}s)"
    else
        echo "  ✗ HTTP $status"
    fi

    if [ -n "$ssl_status" ]; then
        expiry=$(echo "$ssl_status" | grep notAfter | cut -d'=' -f2)
        echo "  ✓ SSL证书有效 (过期: $expiry)"
    else
        echo "  ✗ SSL证书问题"
    fi

    echo
done

echo "=== 检查完成 ==="
```

---

## 调试技巧

### 常用调试方法

```bash
#!/bin/bash

# 1. 启用详细模式
set -x
echo "这条命令会显示"
set +x

# 2. 显示命令但不执行
set -n
echo "这条命令不会执行"
echo "这条也不会"
set +x

# 3. 检查语法
bash -n script.sh

# 4. 跟踪变量
echo "${var@V}"  # 显示变量定义方式和值

# 5. 显示所有函数
declare -f

# 6. 显示所有变量
set
```

### 常见错误

**错误1: 引号问题**
```bash
# 错误
files=$(ls)
for f in $files; do  # 空格分隔会被解释为多个文件
    echo $f
done

# 正确
files=($(ls))
for f in "${files[@]}"; do
    echo "$f"
done
```

**错误2: 文件名包含空格**
```bash
# 错误
for f in *.txt; do
    cat $f  # 文件名有空格会失败
done

# 正确
for f in *.txt; do
    [ -e "$f" ] || continue
    cat "$f"
done
```

**错误3: 未初始化变量**
```bash
# 错误
if [ $var = "value" ]; then  # $var未定义会报错
    echo "匹配"
fi

# 正确
if [ "${var:-}" = "value" ]; then
    echo "匹配"
fi
# 或者
if [ "${var:-default}" = "value" ]; then
    echo "匹配"
fi
```

**错误4: glob 扩展**
```bash
# 在变量中存储文件列表
files="*.txt"
for f in $files; do  # 不会扩展glob
    echo $f
done

# 正确
for f in *.txt; do  # 直接使用glob
    echo $f
done
```

### 性能优化

```bash
# 1. 避免子shell
# 错误
count=$(cat file.txt | wc -l)

# 正确
count=$(wc -l < file.txt)

# 2. 使用内置命令
# 错误
num=$(echo "$string" | wc -c)

# 正确
num=${#string}

# 3. 减少管道
# 错误
ps aux | grep python | grep -v grep | awk '{print $2}'

# 正确
pgrep python

# 4. 使用 associative arrays
# 为大量数据创建查找表
declare -A lookup
for item in "${array[@]}"; do
    lookup[$item]=1
done

# 检查存在
if [ -n "${lookup[$key]}" ]; then
    echo "存在"
fi
```

---

## 相关资源

- [Bash参考手册](https://www.gnu.org/software/bash/manual/)
- [BashPitfalls](http://mywiki.wooledge.org/BashPitfalls)
- [ShellCheck](https://www.shellcheck.net/) - Shell脚本静态分析
- [Google Shell Style Guide](https://google.github.io/styleguide/shellguide.html)
- 上一章: [07-性能分析](../07-性能分析/)
- 下一章: [09-高级工具](../09-高级工具/)
