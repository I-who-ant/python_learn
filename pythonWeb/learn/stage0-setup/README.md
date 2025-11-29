# Stage 0: Flask 环境准备与第一个应用

> **目标**: 搭建 Flask 开发环境,运行第一个 Flask 程序
> **预计时间**: 0.5 天
> **前置要求**: Python 3.8+ 已安装

---

## 📋 学习任务清单

- [ ] 任务 1: 创建虚拟环境
- [ ] 任务 2: 安装 Flask 及依赖
- [ ] 任务 3: 理解 Flask 最小应用
- [ ] 任务 4: 运行并测试第一个应用
- [ ] 任务 5: 理解项目结构

---

## 步骤 1: 创建虚拟环境

### 为什么需要虚拟环境?

虚拟环境可以:
- 隔离项目依赖
- 避免不同项目间的包冲突
- 方便项目部署

### 操作步骤


```bash
# 1. 进入学习目录
cd /home/seeback/PycharmProjects/python/pythonWeb/learn/stage0-setup

# 2. 创建虚拟环境
python3 -m venv venv

# 3. 激活虚拟环境
source venv/bin/activate  # Linux/Mac
# venv\Scripts\activate   # Windows

# 4. 验证虚拟环境
which python  # 应该显示 venv/bin/python
python --version
```



### Java 对比
```
Python 虚拟环境 ≈ Java Maven/Gradle 的依赖隔离
每个项目有独立的 site-packages ≈ 每个项目有独立的 .m2/repository
```

---

## 步骤 2: 安装 Flask

```bash
# 1. 升级 pip (可选但推荐)
pip install --upgrade pip

# 2. 安装 Flask
pip install flask

# 3. 安装其他常用依赖
pip install python-dotenv  # 环境变量管理

# 4. 验证安装
python -c "import flask; print(flask.__version__)"
```

### 创建 requirements.txt

```bash
# 导出依赖
pip freeze > requirements.txt
```

**requirements.txt 内容**:
```
Flask==3.0.0
python-dotenv==1.0.0
```

### Java 对比
```
pip install flask     ≈  Maven: <dependency>flask</dependency>
requirements.txt      ≈  pom.xml / build.gradle
pip freeze            ≈  mvn dependency:tree
```

---

## 步骤 3: 创建第一个 Flask 应用

### 最小 Flask 应用

创建文件 `app.py`:

```python
from flask import Flask

# 创建 Flask 应用实例
app = Flask(__name__)


# 定义路由和视图函数
@app.route('/')
def hello():
    return 'Hello, Flask!'

# 仅在直接运行时启动服务器
if __name__ == '__main__':
    app.run(debug=True)
```

### 代码解析

#### 1. 导入 Flask 类

```python
from flask import Flask
```


- `Flask` 是核心类,用于创建应用实例

#### 2. 创建应用实例
```python
app = Flask(__name__)
```
- `__name__`: Python 内置变量,表示当前模块名
- Flask 使用它来确定资源位置 (模板,静态文件)

**Java 对比**:
```java
// Spring Boot
@SpringBootApplication
public class Application {
    public static void main(String[] args) {
        SpringApplication.run(Application.class, args);
    }
}
```

#### 3. 定义路由
```python
@app.route('/')
def hello():
    return 'Hello, Flask!'
```

- `@app.route('/')`: 装饰器,绑定 URL 路径到函数
- `'/'`: 根路径
- `hello()`: 视图函数,返回响应内容

**Java 对比**:
```java
@RestController
public class HelloController {
    @GetMapping("/")
    public String hello() {
        return "Hello, Spring!";
    }
}
```

#### 4. 运行服务器
```python
if __name__ == '__main__':
    app.run(debug=True)
```
- `debug=True`: 开启调试模式
  - 自动重载代码
  - 显示详细错误信息
  - **生产环境必须关闭!**

---

## 步骤 4: 运行应用

### 方式 1: 直接运行 (适合开发)

```bash
python app.py
```

输出:
```
 * Serving Flask app 'app'
 * Debug mode: on
WARNING: This is a development server. Do not use it in a production deployment.
 * Running on http://127.0.0.1:5000
Press CTRL+C to quit
```

### 方式 2: 使用 flask 命令 (推荐)

创建 `.flaskenv` 文件:
```
FLASK_APP=app.py
FLASK_DEBUG=1
```

运行:
```bash
flask run
```

### 测试应用

打开浏览器访问: `http://localhost:5000`

或使用 curl:

```bash
curl http://localhost:5000
# 输出: Hello, Flask!
```

---



## 步骤 5: 扩展第一个应用

### 添加更多路由


```python
from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return 'Welcome to Flask!'

@app.route('/hello')
def hello():
    return 'Hello, World!'

@app.route('/user/<name>')
def user(name):
    return f'Hello, {name}!'

@app.route('/post/<int:post_id>')
def show_post(post_id):
    return f'Post ID: {post_id}'

if __name__ == '__main__':
    app.run(debug=True)
```

### 新概念解析

#### 1. 动态路由

```python
@app.route('/user/<name>')
def user(name):
    return f'Hello, {name}!'
```

- `<name>`: URL 变量
- 访问 `/user/alice` → 输出 `Hello, alice!`

#### 2. 类型转换
```python
@app.route('/post/<int:post_id>')
def show_post(post_id):
    return f'Post ID: {post_id}'
```
- `<int:post_id>`: 自动转换为整数
- 访问 `/post/123` → `post_id = 123`
- 访问 `/post/abc` → 404 错误

**支持的类型**:
- `string`: 默认,接受任何文本 (不含斜杠)
- `int`: 整数
- `float`: 浮点数
- `path`: 类似 string,但接受斜杠
- `uuid`: UUID 字符串

**Java 对比**:
```java
@GetMapping("/user/{name}")
public String user(@PathVariable String name) {
    return "Hello, " + name + "!";
}

@GetMapping("/post/{postId}")
public String showPost(@PathVariable int postId) {
    return "Post ID: " + postId;
}
```

---

## 步骤 6: 理解项目结构

### 当前目录结构
```
stage0-setup/
├── venv/              # 虚拟环境 (不提交到 Git)
├── app.py             # 应用主文件
├── .flaskenv          # Flask 环境变量
└── requirements.txt   # 依赖列表
```

### 未来的项目结构 (预览)
```
my-flask-app/
├── venv/
├── app/
│   ├── __init__.py
│   ├── models.py      # 数据模型
│   ├── views.py       # 视图函数
│   ├── forms.py       # 表单
│   ├── templates/     # HTML 模板
│   │   └── base.html
│   └── static/        # 静态文件 (CSS, JS, 图片)
│       ├── css/
│       └── js/
├── tests/             # 测试文件
├── config.py          # 配置文件
├── requirements.txt
└── run.py             # 启动脚本
```

---

## 实战练习

### 练习 1: 个人名片页面

创建一个简单的个人名片 Web 应用:

**要求**:
1. 路由 `/` 显示欢迎信息
2. 路由 `/about` 显示个人介绍
3. 路由 `/contact` 显示联系方式
4. 路由 `/projects` 显示项目列表

**示例代码** (`exercise1.py`):
```python
from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return '''
    <h1>欢迎来到我的个人网站</h1>
    <p>导航: <a href="/about">关于我</a> |
       <a href="/contact">联系方式</a> |
       <a href="/projects">项目</a></p>
    '''

@app.route('/about')
def about():
    return '''
    <h1>关于我</h1>
    <p>我是一名 Python 开发者，正在学习 Flask。</p>
    <p><a href="/">返回首页</a></p>
    '''

@app.route('/contact')
def contact():
    return '''
    <h1>联系方式</h1>
    <ul>
        <li>Email: example@example.com</li>
        <li>GitHub: https://github.com/yourusername</li>
    </ul>
    <p><a href="/">返回首页</a></p>
    '''

@app.route('/projects')
def projects():
    return '''
    <h1>我的项目</h1>
    <ul>
        <li>Flask 学习项目</li>
        <li>个人博客</li>
    </ul>
    <p><a href="/">返回首页</a></p>
    '''

if __name__ == '__main__':
    app.run(debug=True)
```

### 练习 2: 简单计算器

创建一个 Web 计算器:

**要求**:
1. 路由 `/add/<int:a>/<int:b>` 返回两数之和
2. 路由 `/subtract/<int:a>/<int:b>` 返回两数之差
3. 路由 `/multiply/<int:a>/<int:b>` 返回两数之积
4. 路由 `/divide/<int:a>/<int:b>` 返回两数之商

**示例代码** (`exercise2.py`):
```python
from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return '''
    <h1>Web 计算器</h1>
    <p>使用方式:</p>
    <ul>
        <li>/add/5/3 - 加法</li>
        <li>/subtract/10/4 - 减法</li>
        <li>/multiply/6/7 - 乘法</li>
        <li>/divide/20/4 - 除法</li>
    </ul>
    '''

@app.route('/add/<int:a>/<int:b>')
def add(a, b):
    result = a + b
    return f'{a} + {b} = {result}'

@app.route('/subtract/<int:a>/<int:b>')
def subtract(a, b):
    result = a - b
    return f'{a} - {b} = {result}'

@app.route('/multiply/<int:a>/<int:b>')
def multiply(a, b):
    result = a * b
    return f'{a} × {b} = {result}'

@app.route('/divide/<int:a>/<int:b>')
def divide(a, b):
    if b == 0:
        return 'Error: Division by zero!', 400
    result = a / b
    return f'{a} ÷ {b} = {result:.2f}'

if __name__ == '__main__':
    app.run(debug=True)
```

---

## 常见问题 FAQ

### Q1: 为什么访问 http://localhost:5000 显示 404?
**A**: 检查:
1. Flask 应用是否正在运行
2. 路由是否正确定义
3. URL 是否拼写正确

### Q2: 修改代码后没有生效?
**A**: 确保:
1. `debug=True` 已开启 (自动重载)
2. 保存了文件
3. 检查终端是否有错误信息

### Q3: 如何停止 Flask 服务器?
**A**: 在终端按 `Ctrl + C`

### Q4: 端口 5000 被占用怎么办?
**A**: 指定其他端口:
```python
app.run(debug=True, port=8000)
```
或
```bash
flask run --port 8000
```

---

## 学习检查清单

完成后,确保你能:

- [ ] 解释什么是 Flask
- [ ] 创建并激活虚拟环境
- [ ] 安装 Flask 及依赖
- [ ] 编写最小 Flask 应用
- [ ] 理解路由和视图函数
- [ ] 使用动态路由
- [ ] 运行和测试 Flask 应用
- [ ] 完成两个练习项目

---

## 下一步

完成 Stage 0 后,你已经:
✅ 搭建了 Flask 开发环境
✅ 理解了 Flask 的基本概念
✅ 能够创建简单的 Web 应用

**准备进入 Stage 1**: Flask 基础 (路由、请求响应、模板)

---

## 参考资源

### 官方文档
- Flask 快速开始: https://flask.palletsprojects.com/quickstart/
- Flask 安装: https://flask.palletsprojects.com/installation/

### HelloFlask 示例
- `helloflask-main/examples/ch1/app.py` - 最小应用
- `helloflask-main/demos/hello/app.py` - Hello Flask

### 推荐阅读
- 《Flask 从入门到进阶》第 1 章
- Flask 官方教程: https://flask.palletsprojects.com/tutorial/

---

**创建时间**: 2025-11-26
**预计完成时间**: 0.5 天
**下一阶段**: Stage 1 - Flask 基础
