# Stage 3: 数据库集成

> **目标**: 使用 SQLAlchemy ORM 操作数据库
> **预计时间**: 3-4 天
> **核心**: Flask-SQLAlchemy

---

## 核心知识点

### 1. 安装依赖

```bash
pip install flask-sqlalchemy flask-migrate
```

### 2. 配置数据库



```python
from flask import Flask
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///blog.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)
```



### 3. 定义模型

```python
from datetime import datetime

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    # 关系
    posts = db.relationship('Post', backref='author', lazy=True)

    def __repr__(self):
        return f'<User {self.username}>'

class Post(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200), nullable=False)
    content = db.Column(db.Text, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

    def __repr__(self):
        return f'<Post {self.title}>'
```
    
### 4. CRUD 操作

```python
# Create
@app.route('/user/add')
def add_user():
    user = User(username='alice', email='alice@example.com')
    db.session.add(user)
    db.session.commit()
    return 'User created'

# Read
@app.route('/users')
def list_users():
    users = User.query.all()
    return render_template('users.html', users=users)

@app.route('/user/<int:user_id>')
def show_user(user_id):
    user = User.query.get_or_404(user_id)
    return render_template('user.html', user=user)

# Update
@app.route('/user/<int:user_id>/edit', methods=['POST'])
def edit_user(user_id):
    user = User.query.get_or_404(user_id)
    user.username = request.form.get('username')
    db.session.commit()
    return redirect(url_for('show_user', user_id=user.id))

# Delete
@app.route('/user/<int:user_id>/delete', methods=['POST'])
def delete_user(user_id):
    user = User.query.get_or_404(user_id)
    db.session.delete(user)
    db.session.commit()
    return redirect(url_for('list_users'))
```

### 5. 查询操作

```python
# 基础查询
users = User.query.all()
user = User.query.first()
user = User.query.get(1)
user = User.query.filter_by(username='alice').first()

# 条件查询
users = User.query.filter(User.username.like('%alice%')).all()
users = User.query.filter(User.id > 5).all()

# 排序
users = User.query.order_by(User.created_at.desc()).all()

# 分页
page = request.args.get('page', 1, type=int)
pagination = User.query.paginate(page=page, per_page=10)
users = pagination.items
```

### 6. 关系查询

```python
# 一对多
user = User.query.get(1)
posts = user.posts  # 获取用户的所有文章

post = Post.query.get(1)
author = post.author  # 获取文章作者

# 关联查询
posts = Post.query.join(User).filter(User.username == 'alice').all()
```

### 7. 数据库迁移

```bash
# 初始化
flask db init

# 创建迁移
flask db migrate -m "Initial migration"

# 应用迁移
flask db upgrade

# 回退
flask db downgrade
```

---

## 实战项目: Notebook 应用

**功能**:
- 创建笔记
- 查看笔记列表
- 编辑笔记
- 删除笔记
- 搜索笔记

**模型设计**:
```python
class Note(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200))
    content = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, onupdate=datetime.utcnow)
```

---

**Java 对比**:
```java
// JPA Entity
@Entity
public class User {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(unique = true, nullable = false)
    private String username;

    @OneToMany(mappedBy = "author")
    private List<Post> posts;
}

// Repository
@Repository
public interface UserRepository extends JpaRepository<User, Long> {
    User findByUsername(String username);
    List<User> findByUsernameContaining(String keyword);
}
```

**参考**: helloflask-main/examples/ch5, examples/notebook
