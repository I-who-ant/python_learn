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
