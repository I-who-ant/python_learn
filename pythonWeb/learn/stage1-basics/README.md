# Stage 1: Flask 基础

> **目标**: 掌握 Flask 核心功能 - 路由、请求响应、模板渲染
> **预计时间**: 2-3 天
> **前置要求**: 完成 Stage 0

---

## 📋 学习任务清单

- [ ] 任务 1: 掌握路由系统
- [ ] 任务 2: 理解请求与响应
- [ ] 任务 3: 学习 Jinja2 模板
- [ ] 任务 4: 完成实战项目

---

## 第一部分: 路由系统 (Routing)

### 1.1 基础路由

```python
from flask import Flask

app = Flask(__name__)

# 1. 静态路由
@app.route('/')
def index():
    return 'Home Page'

@app.route('/about')
def about():
    return 'About Page'

# 2. 动态路由
@app.route('/user/<username>')
def show_user(username):
    return f'User: {username}'

# 3. 类型转换
@app.route('/post/<int:post_id>')
def show_post(post_id):
    return f'Post ID: {post_id}'

@app.route('/path/<path:subpath>')
def show_subpath(subpath):
    return f'Subpath: {subpath}'
```

### 1.2 HTTP 方法


```python
from flask import request

# 支持多种 HTTP 方法
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        return 'Processing login...'
    else:
        return 'Login form'

# 只接受 POST
@app.route('/api/data', methods=['POST'])
def api_data():
    return {'message': 'Data received'}
```

### 1.3 URL 构建与重定向

```python
from flask import url_for, redirect

@app.route('/admin')
def admin():
    # 重定向到登录页
    return redirect(url_for('login'))

@app.route('/old-page')
def old_page():
    # 永久重定向 (301)
    return redirect(url_for('new_page'), code=301)

@app.route('/new-page')
def new_page():
    return 'This is the new page'
```

**Java 对比**:
```java
// Spring Boot
@GetMapping("/")
public String index() {
    return "Home Page";
}

@GetMapping("/user/{username}")
public String showUser(@PathVariable String username) {
    return "User: " + username;
}

@RequestMapping(value = "/login", method = {RequestMethod.GET, RequestMethod.POST})
public String login(HttpServletRequest request) {
    if ("POST".equals(request.getMethod())) {
        return "Processing login...";
    }
    return "Login form";
}
```

---

## 第二部分: 请求与响应

### 2.1 请求对象 (request)

```python
from flask import request

@app.route('/search')
def search():
    # 1. 查询参数 (GET)
    keyword = request.args.get('q', '')
    page = request.args.get('page', 1, type=int)

    # 2. 表单数据 (POST)
    username = request.form.get('username')

    # 3. JSON 数据
    data = request.get_json()

    # 4. 请求头
    user_agent = request.headers.get('User-Agent')

    # 5. Cookie
    session_id = request.cookies.get('session_id')

    return f'Keyword: {keyword}, Page: {page}'

@app.route('/upload', methods=['POST'])
def upload():
    # 6. 文件上传
    file = request.files.get('file')
    if file:
        filename = file.filename
        file.save(f'uploads/{filename}')
        return f'File {filename} uploaded'
    return 'No file'
```

### 2.2 响应对象 (response)

```python
from flask import make_response, jsonify

@app.route('/text')
def text_response():
    # 1. 简单文本响应
    return 'Hello, World!'

@app.route('/json')
def json_response():
    # 2. JSON 响应
    return jsonify({
        'name': 'Alice',
        'age': 25,
        'items': ['apple', 'banana']
    })

@app.route('/custom')
def custom_response():
    # 3. 自定义响应
    resp = make_response('Custom Response')
    resp.status_code = 200
    resp.headers['X-Custom-Header'] = 'Value'
    resp.set_cookie('username', 'alice', max_age=3600)
    return resp

@app.route('/download')
def download():
    # 4. 文件下载
    from flask import send_file
    return send_file('report.pdf', as_attachment=True)
```

### 2.3 Cookie 和 Session


```python
from flask import session

app.secret_key = 'your-secret-key-here'  # 必须设置

@app.route('/set-session')
def set_session():
    session['username'] = 'alice'
    session['logged_in'] = True
    return 'Session set'

@app.route('/get-session')
def get_session():
    username = session.get('username')
    if username:
        return f'Hello, {username}!'
    return 'Not logged in'

@app.route('/clear-session')
def clear_session():
    session.pop('username', None)
    # 或清空所有
    # session.clear()
    return 'Session cleared'
```



**Java 对比**:
```java
// Spring Boot
@GetMapping("/search")
public String search(
    @RequestParam(required = false) String q,
    @RequestParam(defaultValue = "1") int page
) {
    return "Keyword: " + q + ", Page: " + page;
}

@PostMapping("/upload")
public String upload(@RequestParam("file") MultipartFile file) {
    file.transferTo(new File("uploads/" + file.getOriginalFilename()));
    return "File uploaded";
}

@GetMapping("/json")
public Map<String, Object> jsonResponse() {
    return Map.of("name", "Alice", "age", 25);
}
```



---



## 第三部分: Jinja2 模板


### 3.1 基础模板


创建 `templates/index.html`:
```html
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }}</title>
</head>
<body>
    <h1>Welcome, {{ username }}!</h1>
    <p>Current time: {{ current_time }}</p>
</body>
</html>
```

Python 代码:
```python
from flask import render_template
from datetime import datetime

@app.route('/hello/<name>')
def hello(name):
    return render_template(
        'index.html',
        title='Hello Page',
        username=name,
        current_time=datetime.now()
    )
```

### 3.2 模板语法

```html
{# 这是注释 #}

{# 1. 变量 #}
<p>{{ user.name }}</p>
<p>{{ items[0] }}</p>

{# 2. 控制结构 - if #}
{% if user.is_admin %}
    <p>Welcome, Admin!</p>
{% elif user.is_authenticated %}
    <p>Welcome, {{ user.name }}!</p>
{% else %}
    <p>Please login</p>
{% endif %}

{# 3. 控制结构 - for #}
<ul>
{% for item in items %}
    <li>{{ loop.index }}: {{ item }}</li>
{% endfor %}
</ul>

{# 4. 过滤器 #}
<p>{{ text|upper }}</p>              {# 大写 #}
<p>{{ text|lower }}</p>              {# 小写 #}
<p>{{ text|title }}</p>              {# 首字母大写 #}
<p>{{ text|truncate(20) }}</p>       {# 截断 #}
<p>{{ html_content|safe }}</p>       {# 不转义HTML #}
<p>{{ items|length }}</p>            {# 长度 #}

{# 5. 宏 (类似函数) #}
{% macro render_comment(comment) %}
    <div class="comment">
        <h4>{{ comment.author }}</h4>
        <p>{{ comment.content }}</p>
    </div>
{% endmacro %}

{{ render_comment(comment) }}
```



### 3.3 模板继承


**基础模板** `templates/base.html`:




```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{% block title %}Default Title{% endblock %}</title>
    <link rel="stylesheet" href="{{ url_for('static', filename='css/style.css') }}">
</head>
<body>
    <nav>
        <a href="{{ url_for('index') }}">Home</a>
        <a href="{{ url_for('about') }}">About</a>
        <a href="{{ url_for('contact') }}">Contact</a>
    </nav>

    <main>
        {% block content %}{% endblock %}
    </main>

    <footer>
        <p>&copy; 2025 My Website</p>
    </footer>

    {% block scripts %}{% endblock %}
</body>
</html>

```

**子模板** `templates/home.html`:


```html
{% extends "base.html" %}

{% block title %}Home Page{% endblock %}

{% block content %}
    <h1>Welcome to My Website</h1>
    <p>This is the home page.</p>
{% endblock %}

{% block scripts %}
    <script src="{{ url_for('static', filename='js/home.js') }}"></script>
{% endblock %}
```



### 3.4 包含其他模板

`templates/_navbar.html`:

```html
<nav class="navbar">
    <ul>
        <li><a href="/">Home</a></li>
        <li><a href="/about">About</a></li>
    </ul>
</nav>
```

在其他模板中使用:
```html
{% include '_navbar.html' %}
```

**Java 对比**:
```html
<!-- Thymeleaf -->
<p th:text="${user.name}"></p>

<div th:if="${user.isAdmin}">
    <p>Welcome, Admin!</p>
</div>

<ul>
    <li th:each="item : ${items}" th:text="${item}"></li>
</ul>

<!-- 模板继承类似 -->
<html xmlns:layout="http://www.ultraq.net.nz/thymeleaf/layout"
      layout:decorate="~{base}">
```


---

## 实战项目: 个人博客首页

### 项目结构
```
stage1-basics/
├── app.py
├── templates/
│   ├── base.html
│   ├── index.html
│   ├── about.html
│   ├── post.html
│   └── _post_card.html
└── static/
    ├── css/
    │   └── style.css
    └── js/
        └── main.js
```

### 代码实现


查看完整代码:
- `app.py` - 主应用
- `templates/` - 模板文件
- `static/` - 静态资源


---

## 学习检查清单

- [ ] 理解路由系统 (静态、动态、类型转换)
- [ ] 掌握 HTTP 方法 (GET, POST)
- [ ] 会使用 request 对象获取数据
- [ ] 会构造不同类型的响应
- [ ] 理解 Cookie 和 Session
- [ ] 掌握 Jinja2 基础语法
- [ ] 会使用模板继承
- [ ] 完成博客首页项目

---

## 常见问题 FAQ

**Q1: 模板找不到?**
A: 检查:
1. 文件是否在 `templates/` 目录
2. 文件名拼写是否正确
3. `Flask(__name__)` 是否正确

**Q2: 静态文件 404?**
A: 使用 `url_for('static', filename='css/style.css')`

**Q3: 模板中的变量为 None?**
A: 检查 `render_template()` 是否传递了变量

---

## 下一步

完成 Stage 1 后,进入 **Stage 2: 表单处理**

---

**参考资源**:
- helloflask-main/examples/ch2 (HTTP 和路由)
- helloflask-main/examples/ch3 (模板)
- helloflask-main/demos/http
- helloflask-main/demos/template
