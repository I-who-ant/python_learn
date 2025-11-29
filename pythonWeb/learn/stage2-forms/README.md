# Stage 2: 表单处理

> **目标**: 掌握 Flask 表单验证和文件上传
> **预计时间**: 2 天
> **核心**: Flask-WTF 扩展

---

## 核心知识点

### 1. HTML 表单基础

```python
from flask import Flask, render_template, request, redirect, url_for

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form.get('username')
        email = request.form.get('email')
        password = request.form.get('password')

        # 验证逻辑
        if not username or not email:
            return 'Username and email are required!'

        return f'User {username} registered!'

    return render_template('register.html')
```

### 2. Flask-WTF 扩展

```bash
pip install flask-wtf
```

**表单类定义** `forms.py`:
```python
from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import DataRequired, Email, Length

class RegisterForm(FlaskForm):
    username = StringField('Username',
        validators=[DataRequired(), Length(min=3, max=20)])
    email = StringField('Email',
        validators=[DataRequired(), Email()])
    password = PasswordField('Password',
        validators=[DataRequired(), Length(min=6)])
    submit = SubmitField('Register')
```

**视图函数**:
```python
from forms import RegisterForm

@app.route('/register', methods=['GET', 'POST'])
def register():
    form = RegisterForm()
    if form.validate_on_submit():  # POST 且验证通过
        username = form.username.data
        email = form.email.data
        # 处理注册逻辑...
        return redirect(url_for('success'))
    return render_template('register.html', form=form)
```

**模板** `templates/register.html`:
```html
<form method="POST">
    {{ form.hidden_tag() }}  <!-- CSRF token -->

    {{ form.username.label }}
    {{ form.username(class="form-control") }}
    {% if form.username.errors %}
        <ul class="errors">
        {% for error in form.username.errors %}
            <li>{{ error }}</li>
        {% endfor %}
        </ul>
    {% endif %}

    {{ form.email.label }}
    {{ form.email() }}

    {{ form.password.label }}
    {{ form.password() }}

    {{ form.submit() }}
</form>
```

### 3. 文件上传

```python
from werkzeug.utils import secure_filename
import os

UPLOAD_FOLDER = 'uploads'
ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif'}

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part'

    file = request.files['file']
    if file.filename == '':
        return 'No selected file'

    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        return 'File uploaded successfully'

    return 'Invalid file type'
```

---

## 实战练习

### 练习 1: 用户注册表单
- 用户名、邮箱、密码字段
- 前后端验证
- 显示错误消息

### 练习 2: 搜索表单
- 搜索框 + 提交按钮
- 支持空查询处理

### 练习 3: 头像上传
- 文件选择
- 类型验证
- 大小限制

---

**Java 对比**:
```java
// Spring MVC
@PostMapping("/register")
public String register(@Valid @ModelAttribute RegisterForm form,
                      BindingResult result) {
    if (result.hasErrors()) {
        return "register";
    }
    // 处理注册...
    return "redirect:/success";
}
```

**参考**: helloflask-main/examples/ch4, demos/form
