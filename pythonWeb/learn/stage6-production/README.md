# Stage 6: 性能优化与部署

> **目标**: 优化应用性能,部署到生产环境
> **预计时间**: 2-3 天
> **核心**: 缓存、静态资源、部署

---

## 第一部分: 缓存优化

### 1.1 安装 Flask-Caching

```bash
pip install Flask-Caching redis
```

### 1.2 配置缓存

```python
from flask_caching import Cache

app = Flask(__name__)

# 缓存配置
app.config['CACHE_TYPE'] = 'SimpleCache'  # 开发环境
# app.config['CACHE_TYPE'] = 'RedisCache'  # 生产环境
# app.config['CACHE_REDIS_URL'] = 'redis://localhost:6379/0'

cache = Cache(app)
```

### 1.3 使用缓存

```python
# 1. 缓存视图函数
@app.route('/expensive')
@cache.cached(timeout=300)  # 缓存 5 分钟
def expensive_operation():
    # 耗时操作
    result = complex_calculation()
    return render_template('result.html', result=result)

# 2. 缓存特定参数
@app.route('/user/<username>')
@cache.cached(timeout=600, key_prefix='user_view')
def user_profile(username):
    user = User.query.filter_by(username=username).first_or_404()
    return render_template('profile.html', user=user)

# 3. 手动缓存
@app.route('/posts')
def posts():
    posts = cache.get('all_posts')
    if posts is None:
        posts = Post.query.all()
        cache.set('all_posts', posts, timeout=300)
    return render_template('posts.html', posts=posts)

# 4. 清除缓存
@app.route('/admin/clear-cache')
def clear_cache():
    cache.clear()
    return 'Cache cleared'
```

### 1.4 缓存模板片段

```python
from flask import render_template_string

@app.route('/')
def index():
    # 缓存模板渲染结果
    content = cache.get('homepage_content')
    if content is None:
        content = render_template('index.html')
        cache.set('homepage_content', content, timeout=600)
    return content
```

---

## 第二部分: 静态资源管理

### 2.1 Flask-Assets

```bash
pip install Flask-Assets cssmin jsmin
```

### 2.2 配置资源管理

```python
from flask_assets import Environment, Bundle

assets = Environment(app)

# CSS Bundle
css = Bundle(
    'css/bootstrap.css',
    'css/style.css',
    filters='cssmin',
    output='gen/packed.css'
)
assets.register('css_all', css)

# JS Bundle
js = Bundle(
    'js/jquery.js',
    'js/bootstrap.js',
    'js/main.js',
    filters='jsmin',
    output='gen/packed.js'
)
assets.register('js_all', js)
```

### 2.3 模板中使用

```html
{% assets "css_all" %}
    <link rel="stylesheet" href="{{ ASSET_URL }}">
{% endassets %}

{% assets "js_all" %}
    <script src="{{ ASSET_URL }}"></script>
{% endassets %}
```

### 2.4 CDN 集成

```python
app.config['FLASK_ASSETS_USE_CDN'] = True
app.config['CDN_DOMAIN'] = 'https://cdn.example.com'
```

---

## 第三部分: 部署

### 3.1 生产环境配置

**config.py**:
```python
import os

class Config:
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'hard-to-guess-string'
    SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL')
    SQLALCHEMY_TRACK_MODIFICATIONS = False

class DevelopmentConfig(Config):
    DEBUG = True

class ProductionConfig(Config):
    DEBUG = False

config = {
    'development': DevelopmentConfig,
    'production': ProductionConfig,
    'default': DevelopmentConfig
}
```

### 3.2 使用 Gunicorn

```bash
pip install gunicorn
```

**启动应用**:
```bash
gunicorn -w 4 -b 0.0.0.0:8000 'app:create_app()'
```

**gunicorn.conf.py**:
```python
workers = 4
worker_class = 'sync'
bind = '0.0.0.0:8000'
timeout = 120
accesslog = '-'
errorlog = '-'
loglevel = 'info'
```

### 3.3 Nginx 配置

**/etc/nginx/sites-available/myapp**:
```nginx
server {
    listen 80;
    server_name example.com;

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }

    location /static {
        alias /path/to/app/static;
        expires 30d;
    }
}
```

### 3.4 Docker 部署

**Dockerfile**:
```dockerfile
FROM python:3.11-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 8000

CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:8000", "app:create_app()"]
```

**docker-compose.yml**:
```yaml
version: '3.8'

services:
  web:
    build: .
    ports:
      - "8000:8000"
    environment:
      - DATABASE_URL=postgresql://user:pass@db:5432/myapp
      - SECRET_KEY=your-secret-key
    depends_on:
      - db
      - redis

  db:
    image: postgres:15
    environment:
      - POSTGRES_USER=user
      - POSTGRES_PASSWORD=pass
      - POSTGRES_DB=myapp
    volumes:
      - postgres_data:/var/lib/postgresql/data

  redis:
    image: redis:7
    ports:
      - "6379:6379"

  nginx:
    image: nginx:latest
    ports:
      - "80:80"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
    depends_on:
      - web

volumes:
  postgres_data:
```

### 3.5 环境变量管理

**.env** (不提交到 Git):
```
SECRET_KEY=your-secret-key-here
DATABASE_URL=postgresql://localhost/myapp
REDIS_URL=redis://localhost:6379/0
FLASK_ENV=production
```

**加载环境变量**:
```python
from dotenv import load_dotenv
import os

load_dotenv()

app.config['SECRET_KEY'] = os.getenv('SECRET_KEY')
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL')
```

---

## 第四部分: 性能监控

### 4.1 日志配置

```python
import logging
from logging.handlers import RotatingFileHandler

if not app.debug:
    handler = RotatingFileHandler(
        'logs/app.log',
        maxBytes=10240000,  # 10MB
        backupCount=10
    )
    handler.setFormatter(logging.Formatter(
        '%(asctime)s %(levelname)s: %(message)s '
        '[in %(pathname)s:%(lineno)d]'
    ))
    handler.setLevel(logging.INFO)
    app.logger.addHandler(handler)
    app.logger.setLevel(logging.INFO)
    app.logger.info('Application startup')
```

### 4.2 性能分析

```python
from flask import g
import time

@app.before_request
def before_request():
    g.start_time = time.time()

@app.after_request
def after_request(response):
    if hasattr(g, 'start_time'):
        elapsed = time.time() - g.start_time
        app.logger.info(f'Request took {elapsed:.3f}s')
    return response
```

---

## 实战练习

### 练习 1: 添加缓存
- 为耗时查询添加缓存
- 实现缓存失效策略

### 练习 2: 静态资源优化
- 压缩 CSS/JS
- 配置 CDN

### 练习 3: Docker 部署
- 编写 Dockerfile
- 使用 docker-compose 部署

---

**Java 对比**:

**缓存**:
```java
@Cacheable(value = "posts", key = "#id")
public Post getPost(Long id) {
    return postRepository.findById(id);
}

@CacheEvict(value = "posts", allEntries = true)
public void clearCache() {
}
```

**部署**:
```bash
# Spring Boot JAR
java -jar myapp.jar

# Docker
FROM openjdk:17
COPY target/myapp.jar app.jar
ENTRYPOINT ["java", "-jar", "app.jar"]
```

**参考**: helloflask-main/examples/cache, examples/assets
