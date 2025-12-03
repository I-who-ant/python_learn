# WSGI 与 Servlet 架构对比详解

## 目录
- [1. 核心概念对比](#1-核心概念对比)
- [2. 完整请求流程对比](#2-完整请求流程对比)
- [3. 从前端到后端的详细流程](#3-从前端到后端的详细流程)
- [4. 组件职责详解](#4-组件职责详解)
- [5. 实际代码示例](#5-实际代码示例)
- [6. 部署架构对比](#6-部署架构对比)

---

## 1. 核心概念对比

### 1.1 规范层次对比表

| 层次 | **Python (WSGI)** | **Java (Servlet)** | **说明** |
|------|------------------|-------------------|----------|
| **接口规范** | WSGI (PEP 3333) | Servlet API (javax.servlet.*) | 定义标准接口 |
| **容器实现** | mod_wsgi, Gunicorn, uWSGI | Tomcat, Jetty, WebLogic | 运行应用的服务器 |
| **Web框架** | Flask, Django, FastAPI | Spring MVC, Struts, JSF | 开发者使用的框架 |
| **应用代码** | 路由处理函数/视图类 | Controller/Servlet 类 | 业务逻辑代码 |

### 1.2 核心接口对比

#### WSGI 接口
```python
def application(environ, start_response):
    """
    environ: 包含请求信息的字典 (类似 HttpServletRequest)
    start_response: 回调函数,用于设置响应 (类似 HttpServletResponse)
    返回: 可迭代对象,包含响应体
    """
    status = '200 OK'
    headers = [('Content-Type', 'application/json')]
    start_response(status, headers)
    return [b'{"message": "Hello"}']
```

#### Servlet 接口
```java
public class MyServlet extends HttpServlet {
    protected void doGet(
        HttpServletRequest request,   // 类似 environ
        HttpServletResponse response  // 类似 start_response
    ) throws ServletException, IOException {
        response.setStatus(200);
        response.setContentType("application/json");
        response.getWriter().write("{\"message\": \"Hello\"}");
    }
}
```

---

## 2. 完整请求流程对比

### 2.1 Python WSGI 架构

```
┌─────────────────────────────────────────────────────────────┐
│                        前端浏览器                             │
│          axios.get('http://api.example.com/users')          │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP Request
                         │ GET /users HTTP/1.1
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              Nginx (可选,反向代理 + 负载均衡)                  │
│  - SSL 终止                                                  │
│  - 静态文件服务                                               │
│  - 请求转发到后端                                             │
└────────────────────────┬────────────────────────────────────┘
                         │ proxy_pass
                         ↓
┌─────────────────────────────────────────────────────────────┐
│            Apache HTTP Server (Web 服务器)                   │
│  - 处理 HTTP 连接                                            │
│  - 负载均衡 (如果配置多个进程)                                 │
└────────────────────────┬────────────────────────────────────┘
                         │ 通过 mod_wsgi 模块
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                mod_wsgi (WSGI 容器/适配器)                   │
│  1. 解析 HTTP 请求                                           │
│  2. 构建 environ 字典:                                       │
│     {                                                       │
│       'REQUEST_METHOD': 'GET',                              │
│       'PATH_INFO': '/users',                                │
│       'QUERY_STRING': '',                                   │
│       'HTTP_HOST': 'api.example.com',                       │
│       'wsgi.input': <请求体流>,                              │
│       ...                                                   │
│     }                                                       │
│  3. 准备 start_response 回调                                 │
│  4. 调用 WSGI 应用: app(environ, start_response)             │
└────────────────────────┬────────────────────────────────────┘
                         │ 调用 WSGI 应用
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                  Flask 应用 (WSGI 应用)                      │
│                                                             │
│  from flask import Flask, jsonify                           │
│  app = Flask(__name__)                                      │
│                                                             │
│  @app.route('/users', methods=['GET'])                      │
│  def get_users():                                           │
│      # 1. Flask 解析路由: /users → get_users()              │
│      # 2. 执行业务逻辑                                       │
│      users = db.query('SELECT * FROM users')                │
│      # 3. 返回响应                                          │
│      return jsonify(users), 200                             │
│                                                             │
│  Flask 内部处理:                                             │
│  - 路由匹配 (Werkzeug)                                       │
│  - 请求上下文管理                                            │
│  - 调用视图函数                                              │
│  - 构造 Response 对象                                        │
│  - 转换为 WSGI 响应格式                                      │
└────────────────────────┬────────────────────────────────────┘
                         │ 返回 WSGI 响应
                         │ (status, headers, body_iterator)
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                     mod_wsgi                                │
│  1. 接收 WSGI 响应:                                          │
│     - status = '200 OK'                                     │
│     - headers = [('Content-Type', 'application/json'), ...]  │
│     - body = [b'{"users": [...]}']                          │
│  2. 转换为 HTTP 响应                                         │
│  3. 发送给 Apache                                           │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                   Apache HTTP Server                        │
│  - 添加 HTTP 头 (Server, Date 等)                            │
│  - 发送响应给客户端                                           │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP Response
                         │ HTTP/1.1 200 OK
                         │ Content-Type: application/json
                         │
                         │ {"users": [...]}
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                      Nginx (可选)                            │
│  - 添加缓存头                                                │
│  - Gzip 压缩                                                │
└────────────────────────┬────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                     前端浏览器                               │
│  axios 接收响应:                                             │
│  response.data = {"users": [...]}                           │
│  response.status = 200                                      │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Java Servlet 架构

```
┌─────────────────────────────────────────────────────────────┐
│                        前端浏览器                             │
│          axios.get('http://api.example.com/users')          │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP Request
                         │ GET /users HTTP/1.1
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              Nginx (可选,反向代理 + 负载均衡)                  │
│  - SSL 终止                                                  │
│  - 静态文件服务                                               │
│  - 请求转发到后端                                             │
└────────────────────────┬────────────────────────────────────┘
                         │ proxy_pass
                         ↓
┌─────────────────────────────────────────────────────────────┐
│       Apache HTTP Server (可选,仅用作反向代理)                │
│  - 通过 mod_jk 或 mod_proxy 转发到 Tomcat                     │
│  - 处理静态资源                                               │
└────────────────────────┬────────────────────────────────────┘
                         │ AJP 协议 或 HTTP 代理
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              Tomcat (Servlet 容器 + Web 服务器)              │
│                                                             │
│  ┌─────────────────────────────────────┐                   │
│  │   Connector (连接器)                 │                   │
│  │   - 监听 8080 端口                   │                   │
│  │   - 解析 HTTP 请求                   │                   │
│  └────────────┬────────────────────────┘                   │
│               │                                             │
│               ↓                                             │
│  ┌─────────────────────────────────────┐                   │
│  │   Engine (引擎)                      │                   │
│  │   - 处理请求                         │                   │
│  └────────────┬────────────────────────┘                   │
│               │                                             │
│               ↓                                             │
│  ┌─────────────────────────────────────┐                   │
│  │   Host (虚拟主机)                    │                   │
│  │   - api.example.com                 │                   │
│  └────────────┬────────────────────────┘                   │
│               │                                             │
│               ↓                                             │
│  ┌─────────────────────────────────────┐                   │
│  │   Context (应用上下文)               │                   │
│  │   - /myapp                          │                   │
│  │   - 加载 web.xml                     │                   │
│  │   - 管理 Servlet 生命周期            │                   │
│  └────────────┬────────────────────────┘                   │
│               │                                             │
│               ↓                                             │
│  ┌─────────────────────────────────────┐                   │
│  │   Servlet 容器核心                   │                   │
│  │   1. 创建 HttpServletRequest 对象    │                   │
│  │   2. 创建 HttpServletResponse 对象   │                   │
│  │   3. 根据 URL 匹配 Servlet           │                   │
│  │   4. 调用 Servlet.service() 方法     │                   │
│  └────────────┬────────────────────────┘                   │
└───────────────┼─────────────────────────────────────────────┘
                │
                ↓
┌─────────────────────────────────────────────────────────────┐
│          Spring Boot 应用 (基于 Servlet)                     │
│                                                             │
│  @RestController                                            │
│  @RequestMapping("/users")                                  │
│  public class UserController {                              │
│                                                             │
│      @GetMapping                                            │
│      public ResponseEntity<List<User>> getUsers() {         │
│          // 1. Spring 拦截请求                              │
│          // 2. DispatcherServlet 分发                       │
│          // 3. 找到对应的 Controller 方法                   │
│          List<User> users = userService.findAll();          │
│          return ResponseEntity.ok(users);                   │
│      }                                                      │
│  }                                                          │
│                                                             │
│  内部处理流程:                                               │
│  ┌──────────────────────────────────┐                      │
│  │  DispatcherServlet (核心 Servlet) │                      │
│  │  1. 处理器映射 (HandlerMapping)   │                      │
│  │  2. 调用 Controller               │                      │
│  │  3. 视图解析 / JSON 序列化        │                      │
│  │  4. 返回响应                      │                      │
│  └──────────────────────────────────┘                      │
└────────────────────────┬────────────────────────────────────┘
                         │ 返回 HttpServletResponse
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                   Tomcat Servlet 容器                        │
│  1. 接收 Controller 返回值                                   │
│  2. HttpServletResponse 写入:                                │
│     - response.setStatus(200)                               │
│     - response.setContentType("application/json")           │
│     - response.getWriter().write("{\"users\": [...]}")       │
│  3. 构建 HTTP 响应                                           │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP Response
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                Apache HTTP Server (可选)                     │
│  - 添加额外的响应头                                           │
└────────────────────────┬────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                      Nginx (可选)                            │
│  - 缓存处理                                                  │
│  - 压缩响应                                                  │
└────────────────────────┬────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                     前端浏览器                               │
│  axios 接收响应:                                             │
│  response.data = {"users": [...]}                           │
│  response.status = 200                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. 从前端到后端的详细流程

### 3.1 前端发起请求 (相同)

#### 使用 Axios 发送 RESTful 请求

```javascript
// 前端代码 (React/Vue 等)
import axios from 'axios';

// GET 请求
axios.get('http://api.example.com/users', {
    headers: {
        'Authorization': 'Bearer token123',
        'Accept': 'application/json'
    },
    params: {
        page: 1,
        limit: 10
    }
})
.then(response => {
    console.log(response.data);  // 后端返回的数据
    console.log(response.status); // 200
})
.catch(error => {
    console.error(error.response.data);
});

// POST 请求
axios.post('http://api.example.com/users', {
    name: 'John',
    email: 'john@example.com'
}, {
    headers: {
        'Content-Type': 'application/json'
    }
})
.then(response => {
    console.log('Created:', response.data);
});
```

#### 生成的 HTTP 请求

```http
GET /users?page=1&limit=10 HTTP/1.1
Host: api.example.com
Authorization: Bearer token123
Accept: application/json
User-Agent: Mozilla/5.0
Connection: keep-alive
```

---

### 3.2 Python (Flask) 后端处理

#### 完整的 Flask 应用示例

```python
# app.py
from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://user:pass@localhost/mydb'
db = SQLAlchemy(app)

# 数据模型
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)

# RESTful API 端点
@app.route('/users', methods=['GET'])
def get_users():
    """
    处理 GET /users 请求

    Flask 内部流程:
    1. Werkzeug 路由匹配: /users → get_users()
    2. 创建 Request 对象 (从 WSGI environ 构建)
    3. 执行视图函数
    """
    # 获取查询参数
    page = request.args.get('page', 1, type=int)
    limit = request.args.get('limit', 10, type=int)

    # 验证 Token
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'error': 'Unauthorized'}), 401

    # 查询数据库
    users = User.query.paginate(page=page, per_page=limit)

    # 序列化数据
    result = {
        'users': [
            {'id': u.id, 'name': u.name, 'email': u.email}
            for u in users.items
        ],
        'total': users.total,
        'page': page
    }

    # 返回 JSON 响应
    response = jsonify(result)
    response.status_code = 200
    response.headers['X-Total-Count'] = str(users.total)

    return response
    # Flask 将 Response 对象转换为 WSGI 格式:
    # ('200 OK', [('Content-Type', 'application/json'), ...], [b'{"users": [...]}'])

@app.route('/users', methods=['POST'])
def create_user():
    """处理 POST /users 请求"""
    # 解析 JSON 请求体
    data = request.get_json()

    # 验证数据
    if not data or 'name' not in data or 'email' not in data:
        return jsonify({'error': 'Invalid data'}), 400

    # 创建用户
    user = User(name=data['name'], email=data['email'])
    db.session.add(user)
    db.session.commit()

    return jsonify({
        'id': user.id,
        'name': user.name,
        'email': user.email
    }), 201

# WSGI 应用入口
if __name__ == '__main__':
    # 开发环境使用内置服务器
    app.run(debug=True)

# 生产环境通过 mod_wsgi 运行:
# Apache 配置中指定: WSGIScriptAlias / /path/to/app.py
# mod_wsgi 会导入这个模块并调用 app(environ, start_response)
```

#### mod_wsgi 如何调用 Flask

```python
# mod_wsgi 内部流程 (简化版)

# 1. mod_wsgi 加载 app.py 模块
import sys
sys.path.insert(0, '/path/to/your/project')
from app import app as application  # WSGI 应用

# 2. 对每个 HTTP 请求,mod_wsgi 执行:
def handle_request(apache_request):
    # 2.1 构建 environ 字典
    environ = {
        'REQUEST_METHOD': 'GET',
        'PATH_INFO': '/users',
        'QUERY_STRING': 'page=1&limit=10',
        'HTTP_AUTHORIZATION': 'Bearer token123',
        'wsgi.input': apache_request.body_stream,
        'wsgi.url_scheme': 'http',
        # ... 更多环境变量
    }

    # 2.2 准备 start_response 回调
    response_status = None
    response_headers = None

    def start_response(status, headers):
        nonlocal response_status, response_headers
        response_status = status
        response_headers = headers

    # 2.3 调用 Flask 应用
    response_body = application(environ, start_response)
    # Flask 内部会:
    # - 解析路由
    # - 创建 Request 对象
    # - 调用 get_users()
    # - 返回 Response 对象
    # - 转换为 WSGI 格式

    # 2.4 发送响应给客户端
    apache_request.status = int(response_status.split()[0])
    for header_name, header_value in response_headers:
        apache_request.headers_out[header_name] = header_value

    for data in response_body:
        apache_request.write(data)
```

---

### 3.3 Java (Spring Boot) 后端处理

#### 完整的 Spring Boot 应用示例

```java
// UserController.java
package com.example.demo.controller;

import com.example.demo.model.User;
import com.example.demo.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/users")
public class UserController {

    @Autowired
    private UserRepository userRepository;

    /**
     * 处理 GET /users 请求
     *
     * Spring 内部流程:
     * 1. DispatcherServlet 拦截请求
     * 2. HandlerMapping 找到 getUsers() 方法
     * 3. HandlerAdapter 调用方法
     * 4. 序列化返回值为 JSON
     */
    @GetMapping
    public ResponseEntity<Map<String, Object>> getUsers(
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int limit,
            HttpServletRequest request  // Servlet API 对象
    ) {
        // 验证 Token
        String authHeader = request.getHeader("Authorization");
        if (authHeader == null || !authHeader.startsWith("Bearer ")) {
            Map<String, String> error = new HashMap<>();
            error.put("error", "Unauthorized");
            return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(error);
        }

        // 查询数据库
        PageRequest pageRequest = PageRequest.of(page - 1, limit);
        Page<User> usersPage = userRepository.findAll(pageRequest);

        // 构建响应
        Map<String, Object> result = new HashMap<>();
        result.put("users", usersPage.getContent());
        result.put("total", usersPage.getTotalElements());
        result.put("page", page);

        // 添加自定义响应头
        return ResponseEntity.ok()
                .header("X-Total-Count", String.valueOf(usersPage.getTotalElements()))
                .body(result);

        // Spring 会自动:
        // 1. 将 Map 序列化为 JSON (通过 Jackson)
        // 2. 设置 Content-Type: application/json
        // 3. 调用 HttpServletResponse.getWriter().write(json)
    }

    @PostMapping
    public ResponseEntity<User> createUser(@RequestBody User user) {
        // @RequestBody 自动解析 JSON 请求体

        // 验证数据
        if (user.getName() == null || user.getEmail() == null) {
            return ResponseEntity.badRequest().build();
        }

        // 保存用户
        User savedUser = userRepository.save(user);

        return ResponseEntity.status(HttpStatus.CREATED).body(savedUser);
    }
}

// User.java (实体类)
@Entity
@Table(name = "users")
public class User {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private String name;
    private String email;

    // Getters and Setters
}

// UserRepository.java
public interface UserRepository extends JpaRepository<User, Long> {
}

// Application.java (启动类)
@SpringBootApplication
public class Application {
    public static void main(String[] args) {
        SpringApplication.run(Application.class, args);
        // 内嵌 Tomcat 启动,监听 8080 端口
    }
}
```

#### Tomcat 如何调用 Spring

```java
// Tomcat Servlet 容器内部流程 (简化版)

// 1. Tomcat 启动时加载 DispatcherServlet
public class DispatcherServlet extends HttpServlet {

    @Override
    public void init() {
        // 初始化 Spring 容器
        // 扫描 @Controller 注解
        // 构建 URL → 方法 的映射表
    }

    @Override
    protected void service(HttpServletRequest request, HttpServletResponse response) {
        // 2. 对每个 HTTP 请求,Tomcat 调用 service()

        String path = request.getRequestURI();  // "/users"
        String method = request.getMethod();     // "GET"

        // 3. 查找处理器
        HandlerMethod handler = handlerMapping.getHandler(path, method);
        // 找到: UserController.getUsers()

        // 4. 提取参数
        int page = Integer.parseInt(request.getParameter("page"));
        int limit = Integer.parseInt(request.getParameter("limit"));

        // 5. 调用 Controller 方法
        ResponseEntity<?> result = handler.invoke(page, limit, request);

        // 6. 处理返回值
        response.setStatus(result.getStatusCodeValue());  // 200
        result.getHeaders().forEach((name, values) -> {
            response.setHeader(name, values.get(0));
        });

        // 7. 序列化响应体
        Object body = result.getBody();
        String json = objectMapper.writeValueAsString(body);

        // 8. 写入响应
        response.setContentType("application/json");
        response.getWriter().write(json);
    }
}
```

---

## 4. 组件职责详解

### 4.1 Python 技术栈组件职责

| 组件 | 职责 | 类比 Java 组件 |
|------|------|---------------|
| **Nginx** | - 反向代理<br>- SSL 终止<br>- 负载均衡<br>- 静态文件服务 | Nginx / Apache HTTP Server |
| **Apache + mod_wsgi** | - HTTP 服务器<br>- WSGI 容器<br>- 将 HTTP 请求转换为 WSGI 调用 | **Apache + mod_jk** 或 **Tomcat** |
| **Flask** | - 路由管理<br>- 请求/响应处理<br>- 模板渲染<br>- 实现 WSGI 接口 | **Spring MVC** |
| **SQLAlchemy** | ORM 数据库访问 | **Hibernate / JPA** |
| **Werkzeug** | - WSGI 工具库<br>- 请求/响应对象<br>- 路由系统 | **Servlet API** 的部分功能 |

### 4.2 Java 技术栈组件职责

| 组件 | 职责 | 类比 Python 组件 |
|------|------|----------------|
| **Nginx** | - 反向代理<br>- SSL 终止<br>- 负载均衡 | Nginx |
| **Tomcat** | - HTTP 服务器<br>- Servlet 容器<br>- 管理 Servlet 生命周期 | **Apache + mod_wsgi** |
| **Spring MVC** | - 路由管理 (@RequestMapping)<br>- 依赖注入<br>- AOP | **Flask** |
| **Hibernate/JPA** | ORM 数据库访问 | **SQLAlchemy** |
| **Servlet API** | - HttpServletRequest<br>- HttpServletResponse<br>- 定义容器和应用的接口 | **WSGI 规范** |

---

## 5. 实际代码示例

### 5.1 处理 POST 请求并返回 JSON

#### Python (Flask)

```python
from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

@app.route('/api/orders', methods=['POST'])
def create_order():
    # 1. 解析 JSON 请求体
    data = request.get_json()
    # data = {'product_id': 123, 'quantity': 2}

    # 2. 访问请求头
    user_agent = request.headers.get('User-Agent')
    auth_token = request.headers.get('Authorization')

    # 3. 业务逻辑
    order = {
        'id': 456,
        'product_id': data.get('product_id'),
        'quantity': data.get('quantity'),
        'created_at': datetime.now().isoformat(),
        'user_agent': user_agent
    }

    # 4. 返回 JSON 响应
    response = jsonify(order)
    response.status_code = 201
    response.headers['Location'] = f'/api/orders/{order["id"]}'

    return response

# WSGI 应用对象
# mod_wsgi 会调用: app(environ, start_response)
```

#### Java (Spring Boot)

```java
@RestController
@RequestMapping("/api/orders")
public class OrderController {

    @PostMapping
    public ResponseEntity<Order> createOrder(
            @RequestBody OrderRequest request,
            @RequestHeader("User-Agent") String userAgent,
            @RequestHeader(value = "Authorization", required = false) String authToken
    ) {
        // 1. 请求体自动解析为 OrderRequest 对象
        // request.getProductId(), request.getQuantity()

        // 2. 业务逻辑
        Order order = new Order();
        order.setId(456L);
        order.setProductId(request.getProductId());
        order.setQuantity(request.getQuantity());
        order.setCreatedAt(LocalDateTime.now());
        order.setUserAgent(userAgent);

        // 3. 返回响应
        return ResponseEntity
                .status(HttpStatus.CREATED)
                .header("Location", "/api/orders/" + order.getId())
                .body(order);
        // Spring 自动序列化 Order 为 JSON
    }
}

class OrderRequest {
    private Long productId;
    private Integer quantity;
    // Getters and Setters
}

class Order {
    private Long id;
    private Long productId;
    private Integer quantity;
    private LocalDateTime createdAt;
    private String userAgent;
    // Getters and Setters
}
```

---

### 5.2 environ vs HttpServletRequest 对比

#### Python - environ 字典

```python
def application(environ, start_response):
    # 请求方法
    method = environ['REQUEST_METHOD']  # 'POST'

    # URL 路径
    path = environ['PATH_INFO']  # '/api/orders'

    # 查询参数
    query = environ['QUERY_STRING']  # 'page=1&limit=10'

    # 请求头 (以 HTTP_ 开头)
    content_type = environ.get('CONTENT_TYPE')  # 'application/json'
    user_agent = environ.get('HTTP_USER_AGENT')
    auth = environ.get('HTTP_AUTHORIZATION')

    # 请求体
    content_length = int(environ.get('CONTENT_LENGTH', 0))
    body = environ['wsgi.input'].read(content_length)

    # 服务器信息
    server_name = environ['SERVER_NAME']  # 'api.example.com'
    server_port = environ['SERVER_PORT']  # '80'

    # WSGI 特定变量
    scheme = environ['wsgi.url_scheme']  # 'http' 或 'https'

    return [b'Response body']
```

#### Java - HttpServletRequest

```java
protected void doPost(HttpServletRequest request, HttpServletResponse response) {
    // 请求方法
    String method = request.getMethod();  // "POST"

    // URL 路径
    String path = request.getRequestURI();  // "/api/orders"

    // 查询参数
    String query = request.getQueryString();  // "page=1&limit=10"
    String page = request.getParameter("page");  // "1"

    // 请求头
    String contentType = request.getContentType();  // "application/json"
    String userAgent = request.getHeader("User-Agent");
    String auth = request.getHeader("Authorization");

    // 请求体
    BufferedReader reader = request.getReader();
    String body = reader.lines().collect(Collectors.joining());

    // 服务器信息
    String serverName = request.getServerName();  // "api.example.com"
    int serverPort = request.getServerPort();  // 80

    // 协议
    String scheme = request.getScheme();  // "http" 或 "https"
}
```

---

## 6. 部署架构对比

### 6.1 Python 生产环境部署

#### 架构图
```
Internet
    ↓
[Nginx :443] (SSL 终止, 静态文件)
    ↓
[Apache + mod_wsgi :8000] (WSGI 服务器, 多进程)
    ↓
[Flask 应用] (Python 进程池)
    ↓
[MySQL/PostgreSQL] (数据库)
```

#### Apache 配置

```apache
# /etc/httpd/conf.d/myapp.conf

<VirtualHost *:8000>
    ServerName api.example.com

    # 加载 mod_wsgi 模块
    LoadModule wsgi_module modules/mod_wsgi.so

    # 指定 WSGI 应用
    WSGIScriptAlias / /var/www/myapp/app.py

    # 配置进程组 (多进程模式)
    WSGIDaemonProcess myapp \
        processes=4 \
        threads=1 \
        python-path=/var/www/myapp:/var/www/myapp/venv/lib/python3.9/site-packages

    WSGIProcessGroup myapp

    # 应用目录权限
    <Directory /var/www/myapp>
        Require all granted
    </Directory>

    # 日志
    ErrorLog /var/log/httpd/myapp_error.log
    CustomLog /var/log/httpd/myapp_access.log combined
</VirtualHost>
```

#### Nginx 前置代理配置

```nginx
# /etc/nginx/conf.d/myapp.conf

upstream backend {
    server 127.0.0.1:8000;
}

server {
    listen 443 ssl http2;
    server_name api.example.com;

    ssl_certificate /etc/ssl/certs/api.example.com.crt;
    ssl_certificate_key /etc/ssl/private/api.example.com.key;

    # 静态文件直接服务
    location /static/ {
        alias /var/www/myapp/static/;
        expires 30d;
    }

    # API 请求转发到 Apache + mod_wsgi
    location / {
        proxy_pass http://backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

---

### 6.2 Java 生产环境部署

#### 架构图
```
Internet
    ↓
[Nginx :443] (SSL 终止, 静态文件)
    ↓
[Tomcat :8080] (Servlet 容器, 多线程)
    ↓
[Spring Boot 应用] (WAR / 内嵌 Tomcat)
    ↓
[MySQL/PostgreSQL] (数据库)
```

#### Tomcat server.xml 配置

```xml
<!-- /opt/tomcat/conf/server.xml -->

<Server port="8005" shutdown="SHUTDOWN">
    <Service name="Catalina">

        <!-- HTTP 连接器 -->
        <Connector port="8080"
                   protocol="HTTP/1.1"
                   maxThreads="200"
                   minSpareThreads="25"
                   connectionTimeout="20000"
                   redirectPort="8443" />

        <!-- Engine -->
        <Engine name="Catalina" defaultHost="localhost">

            <!-- 虚拟主机 -->
            <Host name="api.example.com"
                  appBase="/opt/webapps"
                  unpackWARs="true"
                  autoDeploy="true">

                <!-- 应用上下文 -->
                <Context path="" docBase="myapp" reloadable="false">
                    <!-- 数据源配置 -->
                    <Resource name="jdbc/MyDB"
                              auth="Container"
                              type="javax.sql.DataSource"
                              driverClassName="com.mysql.jdbc.Driver"
                              url="jdbc:mysql://localhost:3306/mydb"
                              username="user"
                              password="pass"
                              maxTotal="20" />
                </Context>

            </Host>
        </Engine>
    </Service>
</Server>
```

#### Nginx 配置

```nginx
# /etc/nginx/conf.d/myapp.conf

upstream tomcat {
    server 127.0.0.1:8080;
}

server {
    listen 443 ssl http2;
    server_name api.example.com;

    ssl_certificate /etc/ssl/certs/api.example.com.crt;
    ssl_certificate_key /etc/ssl/private/api.example.com.key;

    # 静态文件
    location /static/ {
        alias /opt/webapps/myapp/static/;
        expires 30d;
    }

    # 转发到 Tomcat
    location / {
        proxy_pass http://tomcat;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

---

### 6.3 部署差异总结

| 维度 | Python (WSGI) | Java (Servlet) |
|------|--------------|---------------|
| **应用服务器依赖** | 需要 Apache + mod_wsgi (或 Gunicorn) | Tomcat 是独立的 |
| **并发模型** | 多进程 (mod_wsgi daemon mode) | 多线程 (Tomcat 默认) |
| **内存占用** | 每个进程独立内存空间,占用较大 | 线程共享内存,占用较小 |
| **隔离性** | 进程隔离,一个崩溃不影响其他 | 线程崩溃可能导致整个 JVM 崩溃 |
| **启动时间** | 快 (Python 解释器) | 慢 (JVM 启动) |
| **热部署** | 需要重启 Apache 进程组 | Tomcat 支持应用热部署 |

---

## 7. 关键概念总结

### 7.1 WSGI vs Servlet 本质

| 对比项 | WSGI | Servlet |
|--------|------|---------|
| **定义** | Python Web 应用接口规范 | Java Web 应用接口规范 |
| **标准文档** | PEP 3333 | JSR 340 (Servlet 3.1) |
| **接口形式** | 函数签名: `app(environ, start_response)` | 类接口: `HttpServlet.service(request, response)` |
| **请求对象** | `environ` 字典 | `HttpServletRequest` 对象 |
| **响应方式** | 回调函数 + 返回迭代器 | 直接操作 `HttpServletResponse` |
| **中间件** | 装饰器模式 | Filter 链 |

### 7.2 容器对比

| 容器 | mod_wsgi | Tomcat |
|------|----------|--------|
| **类型** | Apache 模块 | 独立服务器 |
| **独立性** | 依赖 Apache | 完全独立 |
| **语言** | C + Python | Java |
| **配置** | Apache httpd.conf | server.xml + web.xml |
| **进程模型** | 多进程 (daemon) 或嵌入式 | 多线程 |

---

## 8. 学习建议

1. **Python 开发者学习路径**:
   ```
   HTTP 基础 → WSGI 规范 → Flask/Django 框架 → 部署 (mod_wsgi/Gunicorn)
   ```

2. **Java 开发者学习路径**:
   ```
   HTTP 基础 → Servlet API → Spring MVC → Tomcat 容器
   ```

3. **对比学习要点**:
   - 理解接口规范的相似性 (都是容器和应用的契约)
   - 掌握请求/响应对象的使用
   - 学习中间件/过滤器模式
   - 了解部署架构的差异

---

## 参考资源

- [PEP 3333 - WSGI 规范](https://peps.python.org/pep-3333/)
- [Servlet 3.1 规范](https://javaee.github.io/servlet-spec/)
- [Flask 官方文档](https://flask.palletsprojects.com/)
- [Spring Boot 官方文档](https://spring.io/projects/spring-boot)
- [mod_wsgi 文档](https://modwsgi.readthedocs.io/)
- [Apache Tomcat 文档](https://tomcat.apache.org/)
