
# Jinja2 模板引擎详解

## 1. Jinja2 是什么?

Jinja2 是 **Python 最流行的模板引擎**,Flask 框架默认使用它来渲染 HTML 页面。

### 名字来源
- Jinja = 日本"神社"(Shrine)的意思
- 2 = 第二个版本

### 基本使用

```python
from jinja2 import Template

# 创建模板
template = Template("Hello, {{ name }}!")

# 渲染模板
output = template.render(name="张三")
print(output)  # Hello, 张三!
```

## 2. Jinja2 语法基础

### 2.1 三种分隔符

```html
1. {{ ... }}  - 输出变量或表达式的值
2. {% ... %}  - 控制语句(循环、条件等)
3. {# ... #}  - 注释(不会出现在输出中)
```

### 2.2 变量输出

```html
<!-- 简单变量 -->
<h1>{{ title }}</h1>
<p>{{ content }}</p>

<!-- 对象属性 -->
<p>{{ user.name }}</p>
<p>{{ user.email }}</p>

<!-- 字典访问 -->
<p>{{ user['name'] }}</p>

<!-- 列表访问 -->
<p>{{ items[0] }}</p>

<!-- 方法调用 -->
<p>{{ user.get_full_name() }}</p>
```

```python
# Python 代码
from flask import render_template_string

template = """
<h1>{{ title }}</h1>
<p>{{ user.name }} - {{ user.email }}</p>
"""

output = render_template_string(
    template,
    title="用户信息",
    user={'name': '张三', 'email': 'zhang@example.com'}
)
```

### 2.3 自动转义 (安全特性)

Jinja2 **自动转义** HTML 特殊字符,防止 XSS 攻击:


```python

template = Template("<p>{{ content }}</p>")

# 用户输入
malicious_input = "<script>alert('XSS')</script>"

# Jinja2 会自动转义
output = template.render(content=malicious_input)
print(output)
# <p>&lt;script&gt;alert('XSS')&lt;/script&gt;</p>

```



如果确实需要输出原始 HTML:
```html
<!-- 方式1: 使用 safe 过滤器 -->
{{ html_content | safe }}

<!-- 方式2: 使用 Markup 对象 -->
{{ markup_content }}
```



```python
from markupsafe import Markup

template = Template("{{ content | safe }}")
output = template.render(content="<b>粗体</b>")
print(output)  # <b>粗体</b>
```



## 3. 控制结构

### 3.1 条件语句

```html
<!-- if/elif/else -->
{% if user.is_admin %}
  <button>管理面板</button>
{% elif user.is_moderator %}
  <button>审核面板</button>
{% else %}
  <p>普通用户</p>
{% endif %}

<!-- 单行 if -->
<p>{{ "管理员" if user.is_admin else "普通用户" }}</p>
```

### 3.2 循环语句

```html
<!-- 基本循环 -->
<ul>
{% for item in items %}
  <li>{{ item }}</li>
{% endfor %}
</ul>

<!-- 带索引的循环 -->
<ul>
{% for index, item in enumerate(items) %}
  <li>{{ index }}: {{ item }}</li>
{% endfor %}
</ul>

<!-- 循环字典 -->
<dl>
{% for key, value in user.items() %}
  <dt>{{ key }}</dt>
  <dd>{{ value }}</dd>
{% endfor %}
</dl>

<!-- 循环特殊变量 -->
<ul>
{% for item in items %}
  <li>
    {{ loop.index }}.  {# 从 1 开始的索引 #}
    {{ item }}
    {% if loop.first %}(第一个){% endif %}
    {% if loop.last %}(最后一个){% endif %}
  </li>
{% endfor %}
</ul>

<!-- 空列表处理 -->
<ul>
{% for item in items %}
  <li>{{ item }}</li>
{% else %}
  <li>没有数据</li>
{% endfor %}
</ul>
```

**loop 对象的属性:**

| 属性 | 说明 |
|------|------|
| `loop.index` | 当前循环索引(从 1 开始) |
| `loop.index0` | 当前循环索引(从 0 开始) |
| `loop.first` | 是否是第一次循环 |
| `loop.last` | 是否是最后一次循环 |
| `loop.length` | 序列长度 |
| `loop.cycle` | 在多个值之间循环 |

## 4. 过滤器 (Filters)

过滤器用于修改变量的显示方式:

```html
{{ variable | filter }}
{{ variable | filter(arg1, arg2) }}
{{ variable | filter1 | filter2 }}  <!-- 链式调用 -->
```

### 常用过滤器

```html
<!-- 字符串处理 -->
{{ "hello" | upper }}           <!-- HELLO -->
{{ "WORLD" | lower }}           <!-- world -->
{{ "hello world" | title }}     <!-- Hello World -->
{{ "hello world" | capitalize }}<!-- Hello world -->

<!-- 去除空白 -->
{{ "  hello  " | trim }}        <!-- hello -->

<!-- 默认值 -->
{{ user.name | default("访客") }} <!-- 如果 name 为空,显示"访客" -->

<!-- 长度 -->
{{ items | length }}             <!-- 列表长度 -->

<!-- 截断 -->
{{ long_text | truncate(20) }}   <!-- 只显示前 20 个字符 -->

<!-- 安全输出 HTML -->
{{ html_content | safe }}

<!-- 转义 -->
{{ user_input | escape }}

<!-- 日期格式化 -->
{{ date | datetimeformat('%Y-%m-%d') }}

<!-- 数字格式化 -->
{{ 1234.56 | round(1) }}         <!-- 1234.6 -->
{{ 42 | float }}                 <!-- 42.0 -->

<!-- 列表操作 -->
{{ [1, 2, 3] | join(', ') }}     <!-- 1, 2, 3 -->
{{ [1, 2, 3] | first }}          <!-- 1 -->
{{ [1, 2, 3] | last }}           <!-- 3 -->
{{ items | sort }}               <!-- 排序 -->
{{ items | reverse }}            <!-- 反转 -->
{{ items | unique }}             <!-- 去重 -->

<!-- 字典操作 -->
{{ items | dictsort }}           <!-- 按键排序 -->
```

### 自定义过滤器

```python
from flask import Flask
import re

app = Flask(__name__)

# 定义自定义过滤器
@app.template_filter('remove_html')
def remove_html_tags(text):
    """移除 HTML 标签"""
    return re.sub(r'<[^>]+>', '', text)


# 或使用装饰器
def reverse_string(s):
    return s[::-1]

app.jinja_env.filters['reverse'] = reverse_string
```

```html
<!-- 使用自定义过滤器 -->
{{ content | remove_html }}
{{ "hello" | reverse }}  <!-- olleh -->
```

## 5. 模板继承 (重要!)

模板继承是 Jinja2 最强大的功能之一。

### 基础模板 (base.html)

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>{% block title %}默认标题{% endblock %}</title>
    {% block head %}
    <link rel="stylesheet" href="/static/style.css">
    {% endblock %}
</head>
<body>
    <nav>
        <ul>
            <li><a href="/">首页</a></li>
            <li><a href="/about">关于</a></li>
        </ul>
    </nav>

    <main>
        {% block content %}
        <!-- 子模板会替换这里的内容 -->
        {% endblock %}
    </main>

    <footer>
        <p>&copy; 2025 我的网站</p>
    </footer>

    {% block scripts %}
    <script src="/static/main.js"></script>
    {% endblock %}
</body>
</html>
```

### 子模板 (home.html)

```html
{% extends "base.html" %}

{% block title %}首页{% endblock %}

{% block content %}
    <h1>欢迎来到首页</h1>
    <p>这是首页的内容</p>
{% endblock %}

{% block scripts %}
    {{ super() }}  <!-- 保留父模板的脚本 -->
    <script src="/static/home.js"></script>
{% endblock %}
```

### 多层继承

```html
<!-- base.html (基础) -->
<!DOCTYPE html>
<html>
<head><title>{% block title %}{% endblock %}</title></head>
<body>{% block content %}{% endblock %}</body>
</html>

<!-- layout.html (中间层,继承 base.html) -->
{% extends "base.html" %}
{% block content %}
  <div class="container">
    {% block page_content %}{% endblock %}
  </div>
{% endblock %}

<!-- home.html (最终页面,继承 layout.html) -->
{% extends "layout.html" %}
{% block title %}首页{% endblock %}
{% block page_content %}
  <h1>首页内容</h1>
{% endblock %}
```


## 6. 包含其他模板

```html
<!-- header.html -->
<header>
  <h1>{{ site_name }}</h1>
  <nav>...</nav>
</header>

<!-- footer.html -->
<footer>
  <p>&copy; {{ year }} {{ company }}</p>
</footer>

<!-- main.html -->
<!DOCTYPE html>
<html>
<body>
  {% include 'header.html' %}

  <main>
    <!-- 主要内容 -->
  </main>

  {% include 'footer.html' %}
</body>
</html>
```





## 7. 宏 (Macros)

宏类似于函数,可以重用模板片段:

```html
<!-- macros.html -->
{% macro input(name, type='text', placeholder='') %}
  <div class="form-group">
    <input type="{{ type }}"
           name="{{ name }}"
           placeholder="{{ placeholder }}"
           class="form-control">
  </div>
{% endmacro %}

{% macro render_user(user) %}
  <div class="user-card">
    <h3>{{ user.name }}</h3>
    <p>{{ user.email }}</p>
  </div>
{% endmacro %}
```

```html
<!-- 使用宏 -->
{% from 'macros.html' import input, render_user %}

<form>
  {{ input('username', placeholder='请输入用户名') }}
  {{ input('password', type='password', placeholder='请输入密码') }}
  {{ input('email', type='email', placeholder='请输入邮箱') }}
</form>

<!-- 循环使用宏 -->
{% for user in users %}
  {{ render_user(user) }}
{% endfor %}
```

## 8. Flask 中使用 Jinja2

### 基本用法

```python
from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html',
                          title='首页',
                          users=['张三', '李四'])

@app.route('/user/<username>')
def user_profile(username):
    user = {
        'name': username,
        'email': f'{username}@example.com',
        'is_admin': False
    }
    return render_template('profile.html', user=user)
```

### 全局变量和函数

```python
from datetime import datetime

app = Flask(__name__)

# 全局上下文处理器
@app.context_processor
def inject_globals():
    return {
        'current_year': datetime.now().year,
        'site_name': '我的网站'
    }

# 自定义过滤器
@app.template_filter('currency')
def currency_filter(value):
    return f'¥{value:.2f}'
```

```html
<!-- 在任何模板中都可以使用 -->
<p>&copy; {{ current_year }} {{ site_name }}</p>
<p>价格: {{ price | currency }}</p>
```



## 9. 实际示例

### 完整的博客文章列表

```python
# app.py
from flask import Flask, render_template

app = Flask(__name__)

@app.route('/blog')
def blog():
    posts = [
        {
            'id': 1,
            'title': 'Python 入门教程',
            'author': '张三',
            'date': '2025-01-15',
            'content': 'Python 是一门优秀的编程语言...',
            'tags': ['Python', '教程', '入门']
        },
        {
            'id': 2,
            'title': 'Flask 框架介绍',
            'author': '李四',
            'date': '2025-01-20',
            'content': 'Flask 是一个轻量级的 Web 框架...',
            'tags': ['Flask', 'Web', 'Python']
        }
    ]
    return render_template('blog.html', posts=posts)
```

```html
<!-- templates/base.html -->
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>{% block title %}我的博客{% endblock %}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; }
        .post { border-bottom: 1px solid #ccc; padding: 20px 0; }
        .tag { background: #eee; padding: 2px 8px; border-radius: 3px; margin-right: 5px; }
    </style>
</head>
<body>
    <header>
        <h1>我的博客</h1>
        <nav>
            <a href="/">首页</a> |
            <a href="/blog">文章</a> |
            <a href="/about">关于</a>
        </nav>
    </header>

    <main>
        {% block content %}{% endblock %}
    </main>

    <footer>
        <p>&copy; 2025 我的博客</p>
    </footer>
</body>
</html>
```

```html
<!-- templates/blog.html -->
{% extends "base.html" %}

{% block title %}文章列表 - 我的博客{% endblock %}

{% block content %}
    <h2>文章列表</h2>

    {% for post in posts %}
    <article class="post">
        <h3>{{ post.title }}</h3>
        <p class="meta">
            作者: {{ post.author }} |
            日期: {{ post.date }}
        </p>
        <p>{{ post.content | truncate(100) }}</p>

        <div class="tags">
            {% for tag in post.tags %}
                <span class="tag">{{ tag }}</span>
            {% endfor %}
        </div>

        <a href="/blog/{{ post.id }}">阅读更多 &raquo;</a>
    </article>
    {% else %}
    <p>暂无文章</p>
    {% endfor %}
{% endblock %}
```


## 10. 性能优化


### 编译缓存


Jinja2 会自动编译和缓存模板:

```python
from flask import Flask

app = Flask(__name__)

# 开发环境:自动重载模板
app.config['TEMPLATES_AUTO_RELOAD'] = True

# 生产环境:禁用自动重载以提高性能
app.config['TEMPLATES_AUTO_RELOAD'] = False
```

### 预编译模板

```python
from jinja2 import Environment, FileSystemLoader

env = Environment(loader=FileSystemLoader('templates'))

# 编译所有模板
env.compile_templates('compiled_templates')
```

## 11. 总结

### Jinja2 的核心特性

1. **变量输出**: `{{ variable }}`
2. **控制结构**: `{% if %}`, `{% for %}`
3. **过滤器**: `{{ variable | filter }}`
4. **模板继承**: `{% extends %}`, `{% block %}`
5. **包含模板**: `{% include %}`
6. **宏**: `{% macro %}`
7. **自动转义**: 防止 XSS 攻击

### 为什么选择 Jinja2?

- ✅ 语法简洁优雅
- ✅ 功能强大(继承、宏等)
- ✅ 安全(自动转义)
- ✅ 性能好(编译缓存)
- ✅ Flask 默认集成
- ✅ 文档完善,社区活跃

### 下一步

- 阅读 **02-模板引擎vs前端框架.md** 了解与 Vue/React 的区别
- 实践: 创建一个完整的 Flask 博客系统

---

**Jinja2 是 Flask 不可分割的一部分,掌握它是成为 Flask 开发者的必经之路!**
