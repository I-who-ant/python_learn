# Stage 4: 用户认证

> **目标**: 实现完整的用户登录注册系统
> **预计时间**: 2-3 天
> **核心**: Flask-Login, 密码哈希

---

## 核心知识点

### 1. 安装依赖

```bash
pip install flask-login werkzeug
```

### 2. 用户模型

```python
from flask_login import UserMixin
from werkzeug.security import generate_password_hash, check_password_hash

class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(128))

    def set_password(self, password):
        """设置密码哈希"""
        self.password_hash = generate_password_hash(password)

    def check_password(self, password):
        """验证密码"""
        return check_password_hash(self.password_hash, password)
```

### 3. 配置 Flask-Login

```python
from flask_login import LoginManager

login_manager = LoginManager(app)
login_manager.login_view = 'login'  # 未登录时跳转的页面
login_manager.login_message = '请先登录'

@login_manager.user_loader
def load_user(user_id):
    """加载用户回调函数"""
    return User.query.get(int(user_id))
```

### 4. 注册功能

```python
from flask_login import login_user

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form.get('username')
        email = request.form.get('email')
        password = request.form.get('password')

        # 检查用户是否已存在
        if User.query.filter_by(username=username).first():
            return '用户名已存在'

        # 创建新用户
        user = User(username=username, email=email)
        user.set_password(password)
        db.session.add(user)
        db.session.commit()

        # 自动登录
        login_user(user)
        return redirect(url_for('index'))

    return render_template('register.html')
```

### 5. 登录功能

```python
from flask_login import login_user, current_user

@app.route('/login', methods=['GET', 'POST'])
def login():
    if current_user.is_authenticated:
        return redirect(url_for('index'))

    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        remember = request.form.get('remember', False)

        user = User.query.filter_by(username=username).first()

        if user and user.check_password(password):
            login_user(user, remember=remember)
            next_page = request.args.get('next')
            return redirect(next_page or url_for('index'))

        return '用户名或密码错误'

    return render_template('login.html')
```

### 6. 登出功能

```python
from flask_login import logout_user

@app.route('/logout')
def logout():
    logout_user()
    return redirect(url_for('index'))
```

### 7. 权限控制

```python
from flask_login import login_required, current_user

@app.route('/profile')
@login_required  # 需要登录才能访问
def profile():
    return render_template('profile.html', user=current_user)

@app.route('/admin')
@login_required
def admin():
    if not current_user.is_admin:
        return '需要管理员权限', 403
    return render_template('admin.html')
```

### 8. 模板中使用

```html
{% if current_user.is_authenticated %}
    <p>欢迎, {{ current_user.username }}!</p>
    <a href="{{ url_for('logout') }}">登出</a>
{% else %}
    <a href="{{ url_for('login') }}">登录</a>
    <a href="{{ url_for('register') }}">注册</a>
{% endif %}
```

---

## 安全最佳实践

### 1. CSRF 保护

```python
from flask_wtf.csrf import CSRFProtect

csrf = CSRFProtect(app)
```

### 2. 密码强度验证

```python
from wtforms.validators import ValidationError
import re

def validate_password(form, field):
    password = field.data
    if len(password) < 8:
        raise ValidationError('密码至少8位')
    if not re.search(r'[A-Z]', password):
        raise ValidationError('密码必须包含大写字母')
    if not re.search(r'[a-z]', password):
        raise ValidationError('密码必须包含小写字母')
    if not re.search(r'[0-9]', password):
        raise ValidationError('密码必须包含数字')
```

### 3. 防止 SQL 注入

```python
# ✅ 正确 - 使用 ORM
user = User.query.filter_by(username=username).first()

# ❌ 错误 - 直接拼接 SQL
# query = f"SELECT * FROM user WHERE username='{username}'"
```

---

## 实战练习

### 练习 1: 完整的用户系统
- 注册、登录、登出
- 记住我功能
- 个人资料页面

### 练习 2: 密码重置
- 发送重置邮件
- 验证 token
- 重置密码

### 练习 3: 角色权限
- 普通用户、VIP、管理员
- 不同角色不同权限

---

**Java 对比**:
```java
// Spring Security
@Service
public class UserDetailsServiceImpl implements UserDetailsService {
    @Override
    public UserDetails loadUserByUsername(String username) {
        User user = userRepository.findByUsername(username);
        // 返回 UserDetails
    }
}

// 密码编码
BCryptPasswordEncoder encoder = new BCryptPasswordEncoder();
String hashedPassword = encoder.encode(password);

// 权限控制
@PreAuthorize("hasRole('ADMIN')")
@GetMapping("/admin")
public String admin() {
    return "admin";
}
```

**参考**: Flask-Login 官方文档
