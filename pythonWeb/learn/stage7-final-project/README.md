# Stage 7: 综合实战项目

> **目标**: 独立完成一个完整的 Web 应用
> **预计时间**: 1-2 周
> **成果**: 可部署的生产级应用

---

## 项目选择 (三选一)

### 方案 A: 个人博客系统 ⭐ 推荐

**功能需求**:
1. **用户系统**
   - 注册、登录、登出
   - 个人资料管理
   - 头像上传

2. **文章管理**
   - 创建、编辑、删除文章
   - Markdown 编辑器
   - 文章分类和标签
   - 草稿和发布

3. **评论系统**
   - 发表评论
   - 评论审核
   - 评论回复

4. **搜索功能**
   - 全文搜索
   - 按分类筛选
   - 按标签筛选

5. **管理后台**
   - 用户管理
   - 文章管理
   - 评论管理
   - 统计数据

**技术栈**:
- Flask + SQLAlchemy
- Bootstrap 5
- Markdown-it
- Flask-Login
- Flask-WTF

**数据库设计**:
```python
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True)
    email = db.Column(db.String(120), unique=True)
    password_hash = db.Column(db.String(128))
    avatar = db.Column(db.String(200))
    is_admin = db.Column(db.Boolean, default=False)
    posts = db.relationship('Post', backref='author', lazy='dynamic')
    comments = db.relationship('Comment', backref='author', lazy='dynamic')

class Post(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200))
    content = db.Column(db.Text)
    summary = db.Column(db.String(500))
    is_published = db.Column(db.Boolean, default=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, onupdate=datetime.utcnow)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    category_id = db.Column(db.Integer, db.ForeignKey('category.id'))
    tags = db.relationship('Tag', secondary=post_tags, backref='posts')
    comments = db.relationship('Comment', backref='post', lazy='dynamic')

class Category(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(50), unique=True)
    posts = db.relationship('Post', backref='category', lazy='dynamic')

class Tag(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(50), unique=True)

class Comment(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    is_approved = db.Column(db.Boolean, default=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    post_id = db.Column(db.Integer, db.ForeignKey('post.id'))
```

**参考项目**: helloflask-main Bluelog

---

### 方案 B: 图片社交应用

**功能需求**:
1. **用户系统**
   - 注册、登录
   - 个人主页
   - 关注/取消关注

2. **图片功能**
   - 上传图片
   - 图片展示墙
   - 图片详情页

3. **社交功能**
   - 点赞/取消点赞
   - 评论
   - 收藏

4. **动态流**
   - 关注用户动态
   - 热门图片
   - 最新图片

**技术栈**:
- Flask + SQLAlchemy
- PIL (图片处理)
- Flask-Uploads
- AJAX 交互

**数据库设计**:
```python
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True)
    bio = db.Column(db.String(500))
    photos = db.relationship('Photo', backref='author', lazy='dynamic')

    # 关注关系
    following = db.relationship(
        'User', secondary=followers,
        primaryjoin=(followers.c.follower_id == id),
        secondaryjoin=(followers.c.followed_id == id),
        backref=db.backref('followers', lazy='dynamic'),
        lazy='dynamic'
    )

class Photo(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    filename = db.Column(db.String(200))
    description = db.Column(db.String(500))
    upload_at = db.Column(db.DateTime, default=datetime.utcnow)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))

    likes = db.relationship('Like', backref='photo', lazy='dynamic')
    comments = db.relationship('Comment', backref='photo', lazy='dynamic')

class Like(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    photo_id = db.Column(db.Integer, db.ForeignKey('photo.id'))
```

**参考项目**: helloflask-main Albumy, Moments

---

### 方案 C: 待办事项应用

**功能需求**:
1. **任务管理**
   - 创建、编辑、删除任务
   - 任务状态 (未完成/已完成)
   - 任务优先级

2. **分类管理**
   - 创建分类
   - 任务分类

3. **API 接口**
   - RESTful API
   - JWT 认证
   - API 文档

4. **前端应用**
   - Vue.js / React
   - 单页应用 (SPA)

**技术栈**:
- Flask + SQLAlchemy
- Flask-RESTful
- JWT 认证
- Vue.js (前端)

**数据库设计**:
```python
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True)
    tasks = db.relationship('Task', backref='owner', lazy='dynamic')
    categories = db.relationship('Category', backref='owner', lazy='dynamic')

class Task(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200))
    description = db.Column(db.Text)
    is_completed = db.Column(db.Boolean, default=False)
    priority = db.Column(db.String(20))  # high, medium, low
    due_date = db.Column(db.DateTime)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    category_id = db.Column(db.Integer, db.ForeignKey('category.id'))

class Category(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(50))
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    tasks = db.relationship('Task', backref='category', lazy='dynamic')
```

**参考项目**: helloflask-main Todoism

---

## 开发流程

### 第一周: 核心功能

**Day 1-2: 项目初始化**
- [ ] 创建项目结构
- [ ] 配置数据库
- [ ] 设计数据模型
- [ ] 创建迁移脚本

**Day 3-4: 用户系统**
- [ ] 实现注册功能
- [ ] 实现登录功能
- [ ] 个人资料页面

**Day 5-7: 核心功能**
- [ ] 实现主要业务逻辑
- [ ] 前端页面设计
- [ ] CRUD 操作

### 第二周: 完善与优化

**Day 8-10: 功能完善**
- [ ] 搜索功能
- [ ] 分页功能
- [ ] 权限控制

**Day 11-12: 优化**
- [ ] 添加缓存
- [ ] 性能优化
- [ ] 错误处理

**Day 13-14: 部署**
- [ ] 编写文档
- [ ] Docker 部署
- [ ] 测试上线

---

## 项目检查清单

### 功能完整性
- [ ] 所有核心功能已实现
- [ ] 用户认证系统完善
- [ ] 权限控制正确
- [ ] 错误处理完善

### 代码质量
- [ ] 遵循 PEP 8 规范
- [ ] 适当的注释
- [ ] 代码模块化
- [ ] 无安全漏洞

### 用户体验
- [ ] 响应式设计
- [ ] 加载速度快
- [ ] 操作流畅
- [ ] 错误提示友好

### 部署准备
- [ ] 环境变量配置
- [ ] 数据库迁移脚本
- [ ] Docker 配置
- [ ] README 文档

---

## 推荐开发工具

### 前端
- Bootstrap 5 / Tailwind CSS
- Font Awesome (图标)
- Markdown 编辑器
- 图片上传插件

### 后端
- Flask-DebugToolbar (调试)
- Flask-Migrate (数据库迁移)
- pytest (测试)

### 部署
- Docker + Docker Compose
- Nginx
- PostgreSQL / MySQL

---

## 项目展示

完成项目后,确保包含:

1. **README.md**
   - 项目简介
   - 功能列表
   - 技术栈
   - 安装步骤
   - 运行方法

2. **截图**
   - 首页
   - 主要功能页面
   - 管理后台

3. **在线演示** (可选)
   - 部署到云服务器
   - 提供演示账号

---

## 下一步学习方向

完成综合项目后,你可以:

1. **深入 Flask**
   - 学习 Flask 扩展开发
   - 研究 Flask 源码

2. **前端框架**
   - Vue.js / React
   - 前后端分离

3. **微服务**
   - Flask + Docker + K8s
   - 服务拆分

4. **其他 Python Web 框架**
   - Django
   - FastAPI
   - Quart (异步)

---

**恭喜你完成 Flask 学习之旅! 🎉**

现在你已经掌握了:
- ✅ Flask 核心概念
- ✅ 数据库操作
- ✅ 用户认证
- ✅ RESTful API
- ✅ 部署上线

继续学习,不断实践,成为更优秀的 Web 开发者!
