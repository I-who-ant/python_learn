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
