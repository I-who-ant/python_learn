# Stage 5: 进阶特性

> **目标**: 掌握蓝图、RESTful API、WebSocket
> **预计时间**: 3-4 天
> **核心**: 应用模块化、API 设计

---

## 第一部分: 蓝图 (Blueprints)

### 1.1 什么是蓝图?

蓝图是 Flask 的模块化机制,类似 Java 的模块化或 Spring 的多模块项目。

### 1.2 创建蓝图

**项目结构**:
```
myapp/
├── app/
│   ├── __init__.py          # 应用工厂
│   ├── models.py
│   ├── auth/                # 认证蓝图
│   │   ├── __init__.py
│   │   ├── forms.py
│   │   └── views.py
│   ├── blog/                # 博客蓝图
│   │   ├── __init__.py
│   │   └── views.py
│   └── api/                 # API 蓝图
│       ├── __init__.py
│       └── views.py
└── run.py
```

**定义蓝图** `app/auth/__init__.py`:
```python
from flask import Blueprint

auth_bp = Blueprint('auth', __name__, url_prefix='/auth')

from . import views
```

**蓝图视图** `app/auth/views.py`:
```python
from . import auth_bp
from flask import render_template

@auth_bp.route('/login')
def login():
    return render_template('auth/login.html')

@auth_bp.route('/register')
def register():
    return render_template('auth/register.html')
```

**注册蓝图** `app/__init__.py`:
```python
from flask import Flask

def create_app():
    app = Flask(__name__)

    # 注册蓝图
    from .auth import auth_bp
    app.register_blueprint(auth_bp)

    from .blog import blog_bp
    app.register_blueprint(blog_bp)

    return app
```

**启动应用** `run.py`:
```python
from app import create_app

app = create_app()

if __name__ == '__main__':
    app.run(debug=True)
```

### 1.3 蓝图模板

```
app/
└── templates/
    ├── base.html
    ├── auth/
    │   ├── login.html
    │   └── register.html
    └── blog/
        ├── index.html
        └── post.html
```

---

## 第二部分: RESTful API

### 2.1 RESTful 设计原则

| HTTP 方法 | URL | 操作 |
|----------|-----|------|
| GET | /api/posts | 获取所有文章 |
| GET | /api/posts/1 | 获取单篇文章 |
| POST | /api/posts | 创建文章 |
| PUT | /api/posts/1 | 更新文章 |
| DELETE | /api/posts/1 | 删除文章 |

### 2.2 实现 RESTful API

```python
from flask import Blueprint, jsonify, request

api_bp = Blueprint('api', __name__, url_prefix='/api')

# GET /api/posts - 获取所有文章
@api_bp.route('/posts', methods=['GET'])
def get_posts():
    posts = Post.query.all()
    return jsonify({
        'posts': [post.to_dict() for post in posts]
    })

# GET /api/posts/<id> - 获取单篇文章
@api_bp.route('/posts/<int:post_id>', methods=['GET'])
def get_post(post_id):
    post = Post.query.get_or_404(post_id)
    return jsonify(post.to_dict())

# POST /api/posts - 创建文章
@api_bp.route('/posts', methods=['POST'])
def create_post():
    data = request.get_json()
    post = Post(
        title=data.get('title'),
        content=data.get('content')
    )
    db.session.add(post)
    db.session.commit()
    return jsonify(post.to_dict()), 201

# PUT /api/posts/<id> - 更新文章
@api_bp.route('/posts/<int:post_id>', methods=['PUT'])
def update_post(post_id):
    post = Post.query.get_or_404(post_id)
    data = request.get_json()
    post.title = data.get('title', post.title)
    post.content = data.get('content', post.content)
    db.session.commit()
    return jsonify(post.to_dict())

# DELETE /api/posts/<id> - 删除文章
@api_bp.route('/posts/<int:post_id>', methods=['DELETE'])
def delete_post(post_id):
    post = Post.query.get_or_404(post_id)
    db.session.delete(post)
    db.session.commit()
    return '', 204
```

### 2.3 错误处理

```python
@api_bp.errorhandler(404)
def not_found(error):
    return jsonify({'error': 'Not found'}), 404

@api_bp.errorhandler(400)
def bad_request(error):
    return jsonify({'error': 'Bad request'}), 400

@api_bp.errorhandler(500)
def internal_error(error):
    db.session.rollback()
    return jsonify({'error': 'Internal server error'}), 500
```

### 2.4 API 认证

```python
from functools import wraps

def token_required(f):
    @wraps(f)
    def decorated(*args, **kwargs):
        token = request.headers.get('Authorization')
        if not token:
            return jsonify({'error': 'Token is missing'}), 401

        # 验证 token
        try:
            # 解析 token,获取用户
            user = verify_token(token)
        except:
            return jsonify({'error': 'Invalid token'}), 401

        return f(user, *args, **kwargs)
    return decorated

@api_bp.route('/protected')
@token_required
def protected(user):
    return jsonify({'message': f'Hello {user.username}'})
```

---

## 第三部分: WebSocket (实时通信)

### 3.1 安装 Flask-SocketIO

```bash
pip install flask-socketio python-socketio
```

### 3.2 配置 SocketIO

```python
from flask import Flask
from flask_socketio import SocketIO

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

if __name__ == '__main__':
    socketio.run(app, debug=True)
```

### 3.3 实现聊天室

**服务器端**:
```python
from flask_socketio import emit, join_room, leave_room

@socketio.on('connect')
def handle_connect():
    print('Client connected')
    emit('message', {'data': 'Connected to server'})

@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')

@socketio.on('join')
def on_join(data):
    username = data['username']
    room = data['room']
    join_room(room)
    emit('message',
         {'data': f'{username} has joined the room'},
         room=room)

@socketio.on('leave')
def on_leave(data):
    username = data['username']
    room = data['room']
    leave_room(room)
    emit('message',
         {'data': f'{username} has left the room'},
         room=room)

@socketio.on('send_message')
def handle_message(data):
    room = data['room']
    message = data['message']
    emit('message',
         {'data': message},
         room=room)
```

**客户端** (JavaScript):
```html
<script src="https://cdn.socket.io/4.5.4/socket.io.min.js"></script>
<script>
    const socket = io();

    socket.on('connect', function() {
        console.log('Connected to server');
        socket.emit('join', {
            username: 'Alice',
            room: 'general'
        });
    });

    socket.on('message', function(data) {
        console.log('Message:', data.data);
        // 显示消息
    });

    function sendMessage(message) {
        socket.emit('send_message', {
            room: 'general',
            message: message
        });
    }
</script>
```

---

## 实战项目选择

### 项目 1: 模块化博客 (蓝图)
- auth 模块: 用户认证
- blog 模块: 文章管理
- admin 模块: 后台管理

### 项目 2: Todo API (RESTful)
- 完整的 CRUD API
- JWT 认证
- API 文档

### 项目 3: 简单聊天室 (WebSocket)
- 多房间支持
- 实时消息
- 在线用户列表

---

**Java 对比**:

**模块化**:
```java
// Spring Boot 多模块
@SpringBootApplication
@ComponentScan(basePackages = {"com.example.auth", "com.example.blog"})
public class Application {
    // ...
}
```

**RESTful API**:
```java
@RestController
@RequestMapping("/api/posts")
public class PostController {
    @GetMapping
    public List<Post> getPosts() { }

    @PostMapping
    public Post createPost(@RequestBody Post post) { }

    @PutMapping("/{id}")
    public Post updatePost(@PathVariable Long id, @RequestBody Post post) { }

    @DeleteMapping("/{id}")
    public void deletePost(@PathVariable Long id) { }
}
```

**WebSocket**:
```java
@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {
    // ...
}
```

**参考**: helloflask-main/examples/ch7, examples/longtalk
