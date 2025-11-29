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
