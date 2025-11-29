# WSGI 工作原理详解

## 1. WSGI 是什么?

WSGI (Web Server Gateway Interface) 是 Python Web 应用程序与 Web 服务器之间的标准接口规范。它定义在 **PEP 3333** 中。

### 核心思想
```
客户端请求 → Web服务器(Apache/Nginx) → WSGI服务器(mod_wsgi) → WSGI应用(Flask/Django)
```

WSGI 的目的是**解耦 Web 服务器和 Web 应用框架**,让它们可以独立开发和选择。

## 2. WSGI 规范的核心要素

### 2.1 WSGI 应用的标准签名

一个 WSGI 应用必须是一个可调用对象(函数、方法、类),接受两个参数:

```python
def application(environ, start_response):
    """
    environ: 包含请求信息的字典
    start_response: 用于开始HTTP响应的回调函数

    返回: 可迭代对象,包含响应体的字节串
    """
    pass
```

### 2.2 environ 字典

environ 是一个包含所有请求信息的字典,包含:

#### CGI 环境变量
```python
environ = {
    'REQUEST_METHOD': 'GET',           # HTTP方法
    'SCRIPT_NAME': '',                 # URL路径的初始部分
    'PATH_INFO': '/hello',             # URL路径的剩余部分
    'QUERY_STRING': 'name=world',      # 查询字符串
    'CONTENT_TYPE': 'text/plain',      # 请求体的内容类型
    'CONTENT_LENGTH': '123',           # 请求体长度
    'SERVER_NAME': 'localhost',        # 服务器主机名
    'SERVER_PORT': '8000',             # 服务器端口
    'SERVER_PROTOCOL': 'HTTP/1.1',     # HTTP协议版本
    'HTTP_HOST': 'localhost:8000',     # Host请求头
    'HTTP_USER_AGENT': 'Mozilla/5.0',  # User-Agent请求头
    # 其他HTTP_*变量代表HTTP请求头
}
```

#### WSGI 特定变量
```python
environ.update({
    'wsgi.version': (1, 0),            # WSGI版本
    'wsgi.url_scheme': 'http',         # URL方案(http/https)
    'wsgi.input': <输入流对象>,         # 请求体的输入流
    'wsgi.errors': <错误流对象>,        # 错误日志流
    'wsgi.multithread': True,          # 应用是否多线程运行
    'wsgi.multiprocess': True,         # 应用是否多进程运行
    'wsgi.run_once': False,            # 应用是否只运行一次
})
```

### 2.3 start_response 函数

用于开始 HTTP 响应的回调函数:

```python
def start_response(status, response_headers, exc_info=None):
    """
    status: HTTP状态码字符串,如 '200 OK', '404 Not Found'
    response_headers: 响应头列表 [('Header-Name', 'value'), ...]
    exc_info: 可选的异常信息(用于错误处理)

    返回: write()函数(不推荐使用)
    """
    pass
```

示例:
```python
status = '200 OK'
headers = [
    ('Content-Type', 'text/html; charset=utf-8'),
    ('Content-Length', '13')
]
start_response(status, headers)
```

### 2.4 响应体(可迭代对象)

应用必须返回一个可迭代对象,产生字节串:

```python
# 方式1: 返回列表
return [b'Hello World!']

# 方式2: 返回生成器
def generate():
    yield b'Hello '
    yield b'World!'
return generate()

# 方式3: 返回文件对象
return open('response.html', 'rb')
```

## 3. 完整的 WSGI 应用示例

### 示例1: 最简单的应用
```python
def simple_app(environ, start_response):
    status = '200 OK'
    headers = [('Content-Type', 'text/plain')]
    start_response(status, headers)
    return [b'Hello WSGI!']
```

### 示例2: 处理不同路径
```python
def routing_app(environ, start_response):
    path = environ.get('PATH_INFO', '/')

    if path == '/':
        status = '200 OK'
        body = b'Home Page'
    elif path == '/about':
        status = '200 OK'
        body = b'About Page'
    else:
        status = '404 Not Found'
        body = b'Page Not Found'

    headers = [
        ('Content-Type', 'text/plain'),
        ('Content-Length', str(len(body)))
    ]
    start_response(status, headers)
    return [body]
```

### 示例3: 读取请求体
```python
def echo_app(environ, start_response):
    # 读取请求体
    try:
        content_length = int(environ.get('CONTENT_LENGTH', 0))
    except ValueError:
        content_length = 0

    body = environ['wsgi.input'].read(content_length)

    status = '200 OK'
    headers = [('Content-Type', 'text/plain')]
    start_response(status, headers)

    return [b'You sent: ', body]
```

### 示例4: 处理查询参数
```python
from urllib.parse import parse_qs

def query_app(environ, start_response):
    # 解析查询字符串
    query = parse_qs(environ.get('QUERY_STRING', ''))
    name = query.get('name', ['World'])[0]

    status = '200 OK'
    headers = [('Content-Type', 'text/plain')]
    start_response(status, headers)

    message = f'Hello, {name}!'.encode('utf-8')
    return [message]
```

## 4. mod_wsgi 如何调用 WSGI 应用

### 4.1 调用流程

```
1. Apache 接收 HTTP 请求
        ↓
2. mod_wsgi 拦截请求(根据配置)
        ↓
3. mod_wsgi 构建 environ 字典
   - 从 Apache 请求对象中提取信息
   - 添加 WSGI 特定变量
        ↓
4. mod_wsgi 准备 start_response 回调函数
        ↓
5. mod_wsgi 调用 WSGI 应用
   result = application(environ, start_response)
        ↓
6. WSGI 应用调用 start_response()
   - 设置 HTTP 状态码
   - 设置响应头
        ↓
7. WSGI 应用返回可迭代对象
        ↓
8. mod_wsgi 迭代响应体
   for data in result:
       # 发送数据到客户端
        ↓
9. mod_wsgi 关闭迭代器(如果有 close() 方法)
   if hasattr(result, 'close'):
       result.close()
        ↓
10. 响应完成,连接关闭
```

### 4.2 environ.py 示例解析

在 `mod_wsgi-develop/src/server/environ.py` 中,有一个完整的演示应用:

```python
def application(environ, start_response):
    # 1. 准备响应头
    headers = []
    headers.append(('Content-Type', 'text/plain; charset="UTF-8"'))

    # 2. 调用 start_response
    write = start_response('200 OK', headers)

    # 3. 收集要输出的信息
    output = StringIO()

    # 打印进程信息
    print('PID: %s' % os.getpid(), file=output)
    print('UID: %s' % os.getuid(), file=output)

    # 打印 Python 信息
    print('python.version: %r' % (sys.version,), file=output)
    print('python.path: %r' % (sys.path,), file=output)

    # 打印 mod_wsgi 信息
    print('mod_wsgi.version: %r' % (mod_wsgi.version,), file=output)
    print('mod_wsgi.process_group: %s' % mod_wsgi.process_group, file=output)

    # 打印所有 environ 变量
    keys = sorted(environ.keys())
    for key in keys:
        print('%s: %s' % (key, repr(environ[key])), file=output)

    # 4. 获取结果并编码
    result = output.getvalue()
    if not isinstance(result, bytes):
        result = result.encode('UTF-8')

    # 5. 返回响应体(生成器)
    yield result

    # 6. 可选:回显请求体
    input_stream = environ['wsgi.input']
    data = input_stream.read(8192)
    while data:
        yield data
        data = input_stream.read(8192)
```

## 5. WSGI 中间件

中间件是位于服务器和应用之间的组件,可以:
- 修改 environ
- 修改响应
- 实现通用功能(日志、认证、压缩等)

### 中间件示例

```python
class LoggingMiddleware:
    def __init__(self, app):
        self.app = app

    def __call__(self, environ, start_response):
        # 在调用应用之前
        print(f"Request: {environ['REQUEST_METHOD']} {environ['PATH_INFO']}")

        # 包装 start_response 以记录响应
        def logging_start_response(status, headers, exc_info=None):
            print(f"Response: {status}")
            return start_response(status, headers, exc_info)

        # 调用实际应用
        return self.app(environ, logging_start_response)

# 使用中间件
application = LoggingMiddleware(original_app)
```

### Flask 中的中间件栈

```
请求
  ↓
中间件1(认证)
  ↓
中间件2(日志)
  ↓
Flask 应用
  ↓
中间件2(处理响应)
  ↓
中间件1(处理响应)
  ↓
响应
```

## 6. WSGI 的优缺点

### 优点
1. **标准化**: 所有 Python Web 框架都遵循相同接口
2. **可移植性**: 应用可以在不同的 WSGI 服务器上运行
3. **可组合性**: 通过中间件实现功能复用
4. **简单**: 接口设计简洁明了

### 缺点
1. **同步模型**: 基于传统的同步 I/O
2. **HTTP/1.x 限制**: 不支持 HTTP/2、WebSocket 等现代协议
3. **性能开销**: 每个请求都要构建 environ 字典

### ASGI (异步替代方案)
```python
# ASGI 应用签名(异步)
async def application(scope, receive, send):
    """
    scope: 连接信息字典
    receive: 接收消息的协程
    send: 发送消息的协程
    """
    await send({
        'type': 'http.response.start',
        'status': 200,
        'headers': [[b'content-type', b'text/plain']],
    })
    await send({
        'type': 'http.response.body',
        'body': b'Hello ASGI!',
    })
```

## 7. 实践练习

### 练习1: 创建一个简单的 WSGI 应用
创建 `myapp.py`:
```python
def application(environ, start_response):
    path = environ.get('PATH_INFO', '/')
    method = environ.get('REQUEST_METHOD', 'GET')

    status = '200 OK'
    headers = [('Content-Type', 'text/html; charset=utf-8')]
    start_response(status, headers)

    html = f"""
    <html>
    <head><title>My WSGI App</title></head>
    <body>
        <h1>Hello WSGI!</h1>
        <p>Path: {path}</p>
        <p>Method: {method}</p>
    </body>
    </html>
    """.encode('utf-8')

    return [html]
```

运行:
```bash
mod_wsgi-express start-server myapp.py --port 8000
```

### 练习2: 实现一个计数器中间件
```python
class CounterMiddleware:
    def __init__(self, app):
        self.app = app
        self.count = 0

    def __call__(self, environ, start_response):
        self.count += 1
        environ['counter.value'] = self.count
        return self.app(environ, start_response)

def application(environ, start_response):
    count = environ.get('counter.value', 0)

    status = '200 OK'
    headers = [('Content-Type', 'text/plain')]
    start_response(status, headers)

    return [f'Request #{count}'.encode('utf-8')]

# 应用中间件
application = CounterMiddleware(application)
```

## 8. 调试技巧

### 8.1 打印 environ 内容
```python
def debug_app(environ, start_response):
    import pprint

    status = '200 OK'
    headers = [('Content-Type', 'text/plain')]
    start_response(status, headers)

    output = pprint.pformat(dict(environ))
    return [output.encode('utf-8')]
```

### 8.2 捕获异常
```python
def safe_app(environ, start_response):
    try:
        # 你的应用逻辑
        return actual_app(environ, start_response)
    except Exception as e:
        status = '500 Internal Server Error'
        headers = [('Content-Type', 'text/plain')]
        start_response(status, headers)
        return [f'Error: {str(e)}'.encode('utf-8')]
```

## 9. 总结

WSGI 规范的关键点:
1. **简单的接口**: `application(environ, start_response)`
2. **environ 字典**: 包含所有请求信息
3. **start_response 回调**: 设置响应状态和头
4. **可迭代响应体**: 返回字节串的迭代器
5. **中间件模式**: 实现功能复用

理解了 WSGI,就理解了 Flask 等框架的底层工作原理!

## 下一步

继续阅读 **03-与Flask的关系.md**,了解 Flask 如何实现和使用 WSGI 接口。
