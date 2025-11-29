# SQLAlchemy 与 MyBatis 系列对比理解指南

> **目标读者**: 熟悉 MyBatis / MyBatis-Plus / MyBatis-Flex 的 Java 开发者
> **目的**: 通过 MyBatis 系列框架快速理解 SQLAlchemy ORM

---

## 目录

1. [ORM 框架定位对比](#1-orm-框架定位对比)
2. [核心概念映射](#2-核心概念映射)
3. [实体类与模型定义对比](#3-实体类与模型定义对比)
4. [基础 CRUD 操作对比](#4-基础-crud-操作对比)
5. [查询方法对比](#5-查询方法对比)
6. [条件构造器对比](#6-条件构造器对比)
7. [关系映射对比](#7-关系映射对比)
8. [分页查询对比](#8-分页查询对比)
9. [事务管理对比](#9-事务管理对比)
10. [数据库迁移对比](#10-数据库迁移对比)
11. [高级特性对比](#11-高级特性对比)
12. [完整示例对比](#12-完整示例对比)

---

## 1. ORM 框架定位对比

### MyBatis 系列框架对比

```
MyBatis (原生)
    ↓ 半自动 ORM
    ↓ 需要手写 SQL
    ↓ 灵活但繁琐

MyBatis-Plus
    ↓ 在 MyBatis 基础上增强
    ↓ 提供 BaseMapper (单表 CRUD 无需写 SQL)
    ↓ 条件构造器 (QueryWrapper)
    ↓ 代码生成器

MyBatis-Flex
    ↓ 新一代增强框架
    ↓ 更优雅的 API
    ↓ 更好的性能
    ↓ Row 映射、多表查询
```

---

### SQLAlchemy 定位

```
SQLAlchemy
    ↓ 全自动 ORM (类似 JPA/Hibernate)
    ↓ 几乎不需要写 SQL
    ↓ 强大的查询 API
    ↓ 支持原生 SQL (如需要)
    ↓ 类似于 MyBatis-Plus 的增强模式
```

**关键区别**:

| 特性 | MyBatis | MyBatis-Plus | MyBatis-Flex | SQLAlchemy |
|------|---------|--------------|--------------|------------|
| SQL 编写 | 手写 XML/注解 | 单表自动生成 | 单表自动生成 | 自动生成 |
| CRUD | 手动实现 | BaseMapper 提供 | BaseMapper 提供 | 自动提供 |
| 条件构造 | 手写 SQL | QueryWrapper | QueryWrapper | Query API |
| 关系映射 | 手动处理 | 手动处理 | 改进支持 | 原生支持 |
| 类比 | - | **最接近 SQLAlchemy** | **最接近 SQLAlchemy** | - |

**结论**: SQLAlchemy ≈ MyBatis-Plus/Flex (都是增强后的 ORM)

---

## 2. 核心概念映射

### MyBatis-Plus 核心概念

```java
@Entity                          // 实体类
@TableName("users")              // 表名
public class User {
    @TableId(type = IdType.AUTO) // 主键
    private Long id;

    @TableField("user_name")     // 字段映射
    private String username;
}

// Mapper 接口
public interface UserMapper extends BaseMapper<User> {
    // 继承 BaseMapper,获得基础 CRUD
}

// Service 层
public interface UserService extends IService<User> {}

@Service
public class UserServiceImpl
    extends ServiceImpl<UserMapper, User>
    implements UserService {}
```

---

### SQLAlchemy 核心概念 (完全对应)

```python
from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy(app)  # ORM 引擎 (类似 MyBatis 配置)

# 模型类 (对应 @Entity)
class User(db.Model):
    __tablename__ = 'users'  # 对应 @TableName

    # 对应 @TableId
    id = db.Column(db.Integer, primary_key=True)

    # 对应 @TableField
    username = db.Column(db.String(80), nullable=False)
    email = db.Column(db.String(120), unique=True)

# 查询操作 (对应 BaseMapper)
# SQLAlchemy 自动提供,无需定义 Mapper 接口
User.query.all()           # 类似 userMapper.selectList(null)
User.query.get(1)          # 类似 userMapper.selectById(1)
```

**概念映射表**:

| MyBatis-Plus | SQLAlchemy | 说明 |
|--------------|------------|------|
| `@Entity` | `db.Model` | 实体基类 |
| `@TableName("users")` | `__tablename__ = 'users'` | 表名 |
| `@TableId` | `primary_key=True` | 主键 |
| `@TableField` | `db.Column()` | 字段定义 |
| `BaseMapper<User>` | `User.query` | 查询入口 |
| `QueryWrapper` | `filter()` / `filter_by()` | 条件构造 |
| `IService` | 无需 (直接用 Model) | Service 层 |

---

## 3. 实体类与模型定义对比

### MyBatis-Plus 实体类

```java
@Data
@TableName("users")
public class User {
    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("user_name")
    private String username;

    @TableField("email")
    private String email;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createdAt;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updatedAt;

    // 非数据库字段
    @TableField(exist = false)
    private List<Post> posts;
}
```

---

### SQLAlchemy 模型类 (对应实现)

```python
from datetime import datetime
from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class User(db.Model):
    __tablename__ = 'users'  # 对应 @TableName

    # 对应 @TableId(type = IdType.AUTO)
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)

    # 对应 @TableField
    username = db.Column('user_name', db.String(80), nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)

    # 对应 @TableField(fill = FieldFill.INSERT)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    # 对应 @TableField(fill = FieldFill.INSERT_UPDATE)
    updated_at = db.Column(
        db.DateTime,
        default=datetime.utcnow,
        onupdate=datetime.utcnow
    )

    # 对应 @TableField(exist = false) - 关系字段
    posts = db.relationship('Post', backref='author', lazy=True)

    def __repr__(self):
        return f'<User {self.username}>'

    # 类似 Lombok 的 toDict (用于序列化)
    def to_dict(self):
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }
```

---

### MyBatis-Flex 实体类 (更优雅)

```java
@Table("users")
public class User {
    @Id(keyType = KeyType.Auto)
    private Long id;

    @Column("user_name")
    private String username;

    private String email;

    @Column(onInsertValue = "now()")
    private LocalDateTime createdAt;

    @Column(onInsertValue = "now()", onUpdateValue = "now()")
    private LocalDateTime updatedAt;
}
```

**SQLAlchemy 对应** (同上,SQLAlchemy 本身就很优雅)

---

### 字段类型对比

| Java 类型 | MyBatis-Plus | SQLAlchemy 类型 | Python 类型 |
|----------|--------------|-----------------|-------------|
| `Long` | `@TableId` | `db.Integer` | `int` |
| `String` | `@TableField` | `db.String(n)` | `str` |
| `String` (大文本) | `@TableField` | `db.Text` | `str` |
| `Boolean` | `@TableField` | `db.Boolean` | `bool` |
| `LocalDateTime` | `@TableField` | `db.DateTime` | `datetime` |
| `LocalDate` | `@TableField` | `db.Date` | `date` |
| `BigDecimal` | `@TableField` | `db.Numeric` | `Decimal` |
| `byte[]` | `@TableField` | `db.LargeBinary` | `bytes` |

---

## 4. 基础 CRUD 操作对比

### MyBatis-Plus CRUD

```java
@Service
public class UserService {
    @Autowired
    private UserMapper userMapper;

    // Create - 插入
    public void create() {
        User user = new User();
        user.setUsername("alice");
        user.setEmail("alice@example.com");
        userMapper.insert(user);  // 返回影响行数
        System.out.println("新用户ID: " + user.getId());
    }

    // Read - 查询
    public void read() {
        // 根据 ID 查询
        User user = userMapper.selectById(1L);

        // 查询所有
        List<User> users = userMapper.selectList(null);

        // 条件查询
        User user2 = userMapper.selectOne(
            new QueryWrapper<User>()
                .eq("username", "alice")
        );
    }

    // Update - 更新
    public void update() {
        User user = userMapper.selectById(1L);
        user.setEmail("newemail@example.com");
        userMapper.updateById(user);

        // 或使用 UpdateWrapper
        userMapper.update(null,
            new UpdateWrapper<User>()
                .set("email", "new@example.com")
                .eq("id", 1)
        );
    }

    // Delete - 删除
    public void delete() {
        userMapper.deleteById(1L);

        // 条件删除
        userMapper.delete(
            new QueryWrapper<User>()
                .eq("username", "alice")
        );
    }
}
```

---

### SQLAlchemy CRUD (对应实现)

```python
from flask import Flask
from models import db, User

app = Flask(__name__)

# Create - 插入
def create():
    user = User(
        username='alice',
        email='alice@example.com'
    )
    db.session.add(user)      # 对应 userMapper.insert()
    db.session.commit()
    print(f"新用户ID: {user.id}")

# Read - 查询
def read():
    # 根据 ID 查询 - 对应 selectById
    user = User.query.get(1)
    # 或
    user = User.query.filter_by(id=1).first()

    # 查询所有 - 对应 selectList(null)
    users = User.query.all()

    # 条件查询 - 对应 selectOne
    user2 = User.query.filter_by(username='alice').first()

# Update - 更新
def update():
    # 方式1: 先查询再更新 - 对应 updateById
    user = User.query.get(1)
    user.email = 'newemail@example.com'
    db.session.commit()

    # 方式2: 直接更新 - 对应 UpdateWrapper
    User.query.filter_by(id=1).update({
        'email': 'new@example.com'
    })
    db.session.commit()

# Delete - 删除
def delete():
    # 方式1: 根据 ID 删除 - 对应 deleteById
    user = User.query.get(1)
    db.session.delete(user)
    db.session.commit()

    # 方式2: 条件删除 - 对应 delete(QueryWrapper)
    User.query.filter_by(username='alice').delete()
    db.session.commit()
```

---

### MyBatis-Flex CRUD (更简洁)

```java
// Create
User user = new User();
user.setUsername("alice");
userMapper.insert(user);

// Read
User user = userMapper.selectOneById(1);
List<User> users = userMapper.selectAll();

// Update
UpdateChain.of(User.class)
    .set(User::getEmail, "new@example.com")
    .where(User::getId).eq(1)
    .update();

// Delete
userMapper.deleteById(1);
```

**SQLAlchemy 对应** (同上,API 已经很简洁)

---

### CRUD 对比总结

| 操作 | MyBatis-Plus | SQLAlchemy |
|------|--------------|------------|
| 插入 | `mapper.insert(user)` | `db.session.add(user)` + `commit()` |
| 根据ID查询 | `mapper.selectById(1)` | `User.query.get(1)` |
| 查询所有 | `mapper.selectList(null)` | `User.query.all()` |
| 条件查询 | `mapper.selectOne(wrapper)` | `User.query.filter_by(...).first()` |
| 更新 | `mapper.updateById(user)` | `user.attr = value` + `commit()` |
| 删除 | `mapper.deleteById(1)` | `db.session.delete(user)` + `commit()` |

**关键区别**:
- **MyBatis-Plus**: 需要 `@Autowired UserMapper`,通过 mapper 操作
- **SQLAlchemy**: 直接通过 `User.query` 操作,更简洁
- **事务**: MyBatis-Plus 用 `@Transactional`,SQLAlchemy 用 `db.session.commit()`

---

## 5. 查询方法对比

### MyBatis-Plus 查询方法

```java
// 1. 基础查询
User user = userMapper.selectById(1L);
List<User> users = userMapper.selectList(null);
User user = userMapper.selectOne(new QueryWrapper<User>().eq("username", "alice"));

// 2. 条件查询
List<User> users = userMapper.selectList(
    new QueryWrapper<User>()
        .eq("username", "alice")           // username = 'alice'
        .ne("email", "test@example.com")   // email != 'test@example.com'
        .gt("age", 18)                     // age > 18
        .ge("age", 18)                     // age >= 18
        .lt("age", 60)                     // age < 60
        .le("age", 60)                     // age <= 60
        .like("username", "ali")           // username LIKE '%ali%'
        .likeRight("username", "ali")      // username LIKE 'ali%'
        .in("id", Arrays.asList(1, 2, 3))  // id IN (1, 2, 3)
        .isNull("deleted_at")              // deleted_at IS NULL
        .isNotNull("email")                // email IS NOT NULL
        .between("age", 18, 60)            // age BETWEEN 18 AND 60
);

// 3. 排序
List<User> users = userMapper.selectList(
    new QueryWrapper<User>()
        .orderByDesc("created_at")         // ORDER BY created_at DESC
        .orderByAsc("username")            // ORDER BY username ASC
);

// 4. 分页
Page<User> page = new Page<>(1, 10);  // 第1页,每页10条
IPage<User> result = userMapper.selectPage(page, null);
List<User> users = result.getRecords();
long total = result.getTotal();

// 5. 统计
Long count = userMapper.selectCount(
    new QueryWrapper<User>().gt("age", 18)
);
```

---

### SQLAlchemy 查询方法 (对应实现)

```python
# 1. 基础查询 - 完全对应
user = User.query.get(1)                    # selectById
users = User.query.all()                    # selectList(null)
user = User.query.filter_by(username='alice').first()  # selectOne

# 2. 条件查询 - 完全对应
users = User.query.filter(
    User.username == 'alice',               # eq("username", "alice")
    User.email != 'test@example.com',       # ne("email", "test@example.com")
    User.age > 18,                          # gt("age", 18)
    User.age >= 18,                         # ge("age", 18)
    User.age < 60,                          # lt("age", 60)
    User.age <= 60,                         # le("age", 60)
).all()

# LIKE 查询 - 对应 like
users = User.query.filter(User.username.like('%ali%')).all()     # like
users = User.query.filter(User.username.like('ali%')).all()      # likeRight

# IN 查询 - 对应 in
users = User.query.filter(User.id.in_([1, 2, 3])).all()

# NULL 查询 - 对应 isNull/isNotNull
users = User.query.filter(User.deleted_at.is_(None)).all()       # isNull
users = User.query.filter(User.email.isnot(None)).all()          # isNotNull

# BETWEEN 查询 - 对应 between
users = User.query.filter(User.age.between(18, 60)).all()

# 3. 排序 - 对应 orderByDesc/orderByAsc
users = User.query.order_by(User.created_at.desc()).all()        # orderByDesc
users = User.query.order_by(User.username.asc()).all()           # orderByAsc

# 组合排序
users = User.query.order_by(
    User.created_at.desc(),
    User.username.asc()
).all()

# 4. 分页 - 对应 Page
page = 1
per_page = 10
pagination = User.query.paginate(page=page, per_page=per_page, error_out=False)
users = pagination.items       # 对应 result.getRecords()
total = pagination.total       # 对应 result.getTotal()

# 5. 统计 - 对应 selectCount
count = User.query.filter(User.age > 18).count()
```

---

### MyBatis-Flex 查询 (更优雅的 API)

```java
// 使用 Lambda 表达式
List<User> users = QueryChain.of(User.class)
    .where(User::getUsername).eq("alice")
    .and(User::getAge).gt(18)
    .orderBy(User::getCreatedAt, false)  // DESC
    .list();

// 分页
Page<User> page = QueryChain.of(User.class)
    .where(User::getAge).gt(18)
    .page(new Page<>(1, 10));
```

**SQLAlchemy 对应** (已经很接近):

```python
users = User.query.filter(
    User.username == 'alice',
    User.age > 18
).order_by(User.created_at.desc()).all()
```

---

### 查询方法对比总结

| 操作 | MyBatis-Plus | SQLAlchemy |
|------|--------------|------------|
| 等于 | `.eq("field", value)` | `Model.field == value` |
| 不等于 | `.ne("field", value)` | `Model.field != value` |
| 大于 | `.gt("field", value)` | `Model.field > value` |
| LIKE | `.like("field", "val")` | `Model.field.like('%val%')` |
| IN | `.in("field", list)` | `Model.field.in_(list)` |
| IS NULL | `.isNull("field")` | `Model.field.is_(None)` |
| BETWEEN | `.between("field", a, b)` | `Model.field.between(a, b)` |
| 排序 | `.orderByDesc("field")` | `.order_by(Model.field.desc())` |
| 分页 | `selectPage(page, wrapper)` | `.paginate(page, per_page)` |
| 统计 | `selectCount(wrapper)` | `.count()` |

**关键优势**:
- **SQLAlchemy**: 使用 Python 原生运算符 (`==`, `>`, `<`),更直观
- **MyBatis-Plus**: 使用方法链 (`.eq()`, `.gt()`),更显式

---

## 6. 条件构造器对比

### MyBatis-Plus 条件构造器

```java
// 1. QueryWrapper - 基础条件构造
QueryWrapper<User> wrapper = new QueryWrapper<>();
wrapper.eq("username", "alice")
       .ne("status", 0)
       .gt("age", 18)
       .like("email", "@gmail.com")
       .orderByDesc("created_at");

List<User> users = userMapper.selectList(wrapper);

// 2. LambdaQueryWrapper - 类型安全
LambdaQueryWrapper<User> wrapper = new LambdaQueryWrapper<>();
wrapper.eq(User::getUsername, "alice")
       .gt(User::getAge, 18)
       .like(User::getEmail, "@gmail.com")
       .orderByDesc(User::getCreatedAt);

// 3. 动态条件
QueryWrapper<User> wrapper = new QueryWrapper<>();
wrapper.eq(StringUtils.isNotEmpty(username), "username", username)
       .ge(minAge != null, "age", minAge)
       .le(maxAge != null, "age", maxAge);

// 4. OR 条件
wrapper.eq("username", "alice")
       .or()
       .eq("email", "alice@example.com");
// WHERE username = 'alice' OR email = 'alice@example.com'

// 5. 嵌套条件
wrapper.eq("status", 1)
       .and(w -> w.eq("username", "alice").or().eq("username", "bob"));
// WHERE status = 1 AND (username = 'alice' OR username = 'bob')

// 6. UpdateWrapper - 更新条件
UpdateWrapper<User> updateWrapper = new UpdateWrapper<>();
updateWrapper.set("status", 1)
             .set("updated_at", LocalDateTime.now())
             .eq("username", "alice");
userMapper.update(null, updateWrapper);
```

---

### SQLAlchemy 条件构造 (对应实现)

```python
from sqlalchemy import and_, or_, not_

# 1. 基础条件构造 - 对应 QueryWrapper
query = User.query.filter(
    User.username == 'alice',
    User.status != 0,
    User.age > 18,
    User.email.like('%@gmail.com%')
).order_by(User.created_at.desc())

users = query.all()

# 2. 类型安全 - SQLAlchemy 原生就是类型安全
# User.username 会自动检查字段是否存在

# 3. 动态条件 - 对应动态 QueryWrapper
filters = []
if username:
    filters.append(User.username == username)
if min_age is not None:
    filters.append(User.age >= min_age)
if max_age is not None:
    filters.append(User.age <= max_age)

users = User.query.filter(*filters).all()

# 或使用方法链
query = User.query
if username:
    query = query.filter(User.username == username)
if min_age is not None:
    query = query.filter(User.age >= min_age)
if max_age is not None:
    query = query.filter(User.age <= max_age)
users = query.all()

# 4. OR 条件 - 对应 wrapper.or()
users = User.query.filter(
    or_(
        User.username == 'alice',
        User.email == 'alice@example.com'
    )
).all()

# 5. 嵌套条件 - 对应 and(w -> ...)
users = User.query.filter(
    User.status == 1,
    or_(
        User.username == 'alice',
        User.username == 'bob'
    )
).all()
# WHERE status = 1 AND (username = 'alice' OR username = 'bob')

# 复杂嵌套
users = User.query.filter(
    and_(
        User.status == 1,
        or_(
            User.age > 18,
            User.vip == True
        ),
        not_(User.deleted == True)
    )
).all()

# 6. 更新条件 - 对应 UpdateWrapper
User.query.filter(User.username == 'alice').update({
    'status': 1,
    'updated_at': datetime.utcnow()
})
db.session.commit()
```

---

### MyBatis-Flex 条件构造 (更优雅)

```java
// 使用 QueryWrapper (静态导入)
import static com.mybatisflex.core.query.QueryMethods.*;

QueryWrapper wrapper = QueryWrapper.create()
    .select()
    .from(USER)
    .where(USER.USERNAME.eq("alice"))
    .and(USER.AGE.gt(18))
    .orderBy(USER.CREATED_AT.desc());

// 或使用 QueryChain
List<User> users = QueryChain.of(User.class)
    .where(User::getUsername).eq("alice")
    .and(User::getAge).gt(18)
    .list();
```

**SQLAlchemy 对应** (已经很简洁):

```python
users = User.query.filter(
    User.username == 'alice',
    User.age > 18
).order_by(User.created_at.desc()).all()
```

---

### 条件构造器对比总结

| 特性 | MyBatis-Plus | SQLAlchemy |
|------|--------------|------------|
| 基础条件 | `QueryWrapper` | `filter()` |
| 类型安全 | `LambdaQueryWrapper` | 原生支持 |
| AND 连接 | `.eq().gt()` (默认) | 多个参数或 `and_()` |
| OR 连接 | `.or()` | `or_()` |
| NOT 条件 | - | `not_()` |
| 动态条件 | `.eq(condition, field, val)` | `if` 语句 + `append` |
| 嵌套条件 | `.and(w -> ...)` | `and_()` / `or_()` 嵌套 |
| 更新 | `UpdateWrapper` | `.update({})` |

---

## 7. 关系映射对比

### MyBatis-Plus 关系处理 (需手动)

```java
// 1. 一对多 - 需要手动查询
@Data
@TableName("users")
public class User {
    @TableId
    private Long id;
    private String username;

    // 不是数据库字段
    @TableField(exist = false)
    private List<Post> posts;
}

@Data
@TableName("posts")
public class Post {
    @TableId
    private Long id;
    private String title;
    private Long userId;  // 外键

    @TableField(exist = false)
    private User author;
}

// Service 中手动处理关系
@Service
public class UserService {
    @Autowired
    private UserMapper userMapper;

    @Autowired
    private PostMapper postMapper;

    public User getUserWithPosts(Long userId) {
        // 1. 查询用户
        User user = userMapper.selectById(userId);

        // 2. 手动查询文章
        List<Post> posts = postMapper.selectList(
            new QueryWrapper<Post>().eq("user_id", userId)
        );

        user.setPosts(posts);
        return user;
    }
}
```

---

### SQLAlchemy 关系映射 (原生支持)

```python
# 1. 一对多 - 原生支持,自动处理
class User(db.Model):
    __tablename__ = 'users'

    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80))

    # 定义关系 - 对应 @TableField(exist = false)
    # 但 SQLAlchemy 会自动管理
    posts = db.relationship('Post', backref='author', lazy=True)

class Post(db.Model):
    __tablename__ = 'posts'

    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200))
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'))

    # backref 自动创建 author 属性

# 使用 - 无需手动查询
user = User.query.get(1)
posts = user.posts  # 自动查询关联的文章!
# SQLAlchemy 自动执行: SELECT * FROM posts WHERE user_id = 1

post = Post.query.get(1)
author = post.author  # 自动查询作者!
# SQLAlchemy 自动执行: SELECT * FROM users WHERE id = post.user_id
```

---

### MyBatis-Flex 关系映射 (改进)

```java
// MyBatis-Flex 提供了更好的关系支持
@Table("users")
public class User {
    @Id
    private Long id;
    private String username;

    // 使用 @RelationOneToMany
    @RelationOneToMany(
        selfField = "id",
        targetField = "userId"
    )
    private List<Post> posts;
}

// 查询时可以自动加载
User user = userMapper.selectOneWithRelationById(1);
List<Post> posts = user.getPosts();  // 已自动加载
```

**SQLAlchemy 对应** (同上,更简洁)

---

### 关系类型对比

#### 一对多 (One-to-Many)

**MyBatis-Plus** (手动):
```java
// 用户 → 文章
List<Post> posts = postMapper.selectList(
    new QueryWrapper<Post>().eq("user_id", userId)
);
user.setPosts(posts);
```

**SQLAlchemy** (自动):
```python
# 定义关系
posts = db.relationship('Post', backref='author', lazy=True)

# 使用
user = User.query.get(1)
posts = user.posts  # 自动查询
```

---

#### 多对一 (Many-to-One)

**MyBatis-Plus** (手动):
```java
// 文章 → 用户
Post post = postMapper.selectById(1);
User author = userMapper.selectById(post.getUserId());
post.setAuthor(author);
```

**SQLAlchemy** (自动):
```python
post = Post.query.get(1)
author = post.author  # 自动查询 (通过 backref)
```

---

#### 多对多 (Many-to-Many)

**MyBatis-Plus** (手动,需要中间表):
```java
// 学生 ↔ 课程
@TableName("students")
public class Student {
    @TableId
    private Long id;

    @TableField(exist = false)
    private List<Course> courses;
}

@TableName("student_courses")  // 中间表
public class StudentCourse {
    private Long studentId;
    private Long courseId;
}

// 手动查询
List<StudentCourse> relations = studentCourseMapper.selectList(
    new QueryWrapper<StudentCourse>().eq("student_id", studentId)
);
List<Long> courseIds = relations.stream()
    .map(StudentCourse::getCourseId)
    .collect(Collectors.toList());
List<Course> courses = courseMapper.selectBatchIds(courseIds);
```

**SQLAlchemy** (自动,使用 `secondary`):
```python
# 定义中间表
student_courses = db.Table('student_courses',
    db.Column('student_id', db.Integer, db.ForeignKey('students.id')),
    db.Column('course_id', db.Integer, db.ForeignKey('courses.id'))
)

class Student(db.Model):
    __tablename__ = 'students'
    id = db.Column(db.Integer, primary_key=True)

    # 多对多关系
    courses = db.relationship('Course', secondary=student_courses,
                              backref='students')

class Course(db.Model):
    __tablename__ = 'courses'
    id = db.Column(db.Integer, primary_key=True)

# 使用
student = Student.query.get(1)
courses = student.courses  # 自动查询所有课程

course = Course.query.get(1)
students = course.students  # 自动查询所有学生
```

---

### 关系加载策略

**MyBatis-Plus**: 默认不加载关系,需手动查询

**SQLAlchemy**: 提供多种加载策略

```python
# lazy=True (默认) - 延迟加载
posts = db.relationship('Post', lazy=True)
# 访问 user.posts 时才查询

# lazy='dynamic' - 返回查询对象
posts = db.relationship('Post', lazy='dynamic')
# user.posts 返回 Query 对象,可继续过滤
user.posts.filter(Post.status == 1).all()

# lazy='joined' - 立即加载 (使用 JOIN)
posts = db.relationship('Post', lazy='joined')
# 查询 user 时自动 JOIN posts

# lazy='subquery' - 立即加载 (使用子查询)
posts = db.relationship('Post', lazy='subquery')
```

---

### 关系映射对比总结

| 特性 | MyBatis-Plus | MyBatis-Flex | SQLAlchemy |
|------|--------------|--------------|------------|
| 一对多 | 手动查询 | `@RelationOneToMany` | `db.relationship()` |
| 多对一 | 手动查询 | `@RelationManyToOne` | `backref` 自动 |
| 多对多 | 手动查询中间表 | 改进支持 | `secondary` 参数 |
| 加载策略 | 无 | 有限支持 | 丰富 (lazy, joined, etc.) |
| 自动化程度 | 低 | 中 | 高 |

**结论**: SQLAlchemy 在关系映射方面远超 MyBatis 系列,更接近 JPA/Hibernate

---

## 8. 分页查询对比

### MyBatis-Plus 分页

```java
// 1. 配置分页插件
@Configuration
public class MybatisPlusConfig {
    @Bean
    public MybatisPlusInterceptor mybatisPlusInterceptor() {
        MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();
        interceptor.addInnerInterceptor(
            new PaginationInnerInterceptor(DbType.MYSQL)
        );
        return interceptor;
    }
}

// 2. 使用分页
@Service
public class UserService {
    @Autowired
    private UserMapper userMapper;

    public IPage<User> getUsers(int pageNum, int pageSize) {
        // 创建分页对象
        Page<User> page = new Page<>(pageNum, pageSize);

        // 执行分页查询
        IPage<User> result = userMapper.selectPage(page,
            new QueryWrapper<User>().gt("age", 18)
        );

        // 获取结果
        List<User> users = result.getRecords();  // 当前页数据
        long total = result.getTotal();          // 总记录数
        long pages = result.getPages();          // 总页数

        return result;
    }
}

// 3. Controller 中使用
@GetMapping("/users")
public IPage<User> list(
    @RequestParam(defaultValue = "1") int page,
    @RequestParam(defaultValue = "10") int size
) {
    return userService.getUsers(page, size);
}
```

---

### SQLAlchemy 分页 (对应实现)

```python
# 无需配置插件,SQLAlchemy 原生支持分页

from flask import Flask, request, jsonify

# 使用分页
def get_users(page_num, page_size):
    # 执行分页查询 - 对应 selectPage
    pagination = User.query.filter(User.age > 18).paginate(
        page=page_num,
        per_page=page_size,
        error_out=False  # 页码超出范围时不报错
    )

    # 获取结果 - 对应 IPage
    users = pagination.items     # 对应 result.getRecords()
    total = pagination.total     # 对应 result.getTotal()
    pages = pagination.pages     # 对应 result.getPages()
    has_next = pagination.has_next      # 是否有下一页
    has_prev = pagination.has_prev      # 是否有上一页

    return pagination

# Flask 路由中使用
@app.route('/api/users')
def list_users():
    page = request.args.get('page', 1, type=int)
    size = request.args.get('size', 10, type=int)

    pagination = get_users(page, size)

    return jsonify({
        'items': [user.to_dict() for user in pagination.items],
        'total': pagination.total,
        'pages': pagination.pages,
        'current_page': pagination.page,
        'has_next': pagination.has_next,
        'has_prev': pagination.has_prev
    })
```

---

### 分页结果对比

| 属性 | MyBatis-Plus (IPage) | SQLAlchemy (Pagination) |
|------|----------------------|-------------------------|
| 当前页数据 | `getRecords()` | `.items` |
| 总记录数 | `getTotal()` | `.total` |
| 总页数 | `getPages()` | `.pages` |
| 当前页码 | `getCurrent()` | `.page` |
| 每页大小 | `getSize()` | `.per_page` |
| 是否有下一页 | `hasNext()` | `.has_next` |
| 是否有上一页 | `hasPrevious()` | `.has_prev` |

---

### 前端分页组件集成

**MyBatis-Plus 返回格式**:
```json
{
  "records": [...],
  "total": 100,
  "pages": 10,
  "current": 1,
  "size": 10
}
```

**SQLAlchemy 返回格式** (自定义):
```python
def paginate_response(pagination):
    return {
        'items': [item.to_dict() for item in pagination.items],
        'total': pagination.total,
        'pages': pagination.pages,
        'current': pagination.page,
        'size': pagination.per_page
    }

# 使用
@app.route('/api/users')
def list_users():
    pagination = User.query.paginate(page=page, per_page=size)
    return jsonify(paginate_response(pagination))
```

**两者返回格式可以保持一致,前端无需修改!**

---

## 9. 事务管理对比

### MyBatis-Plus 事务

```java
// 1. 声明式事务 (@Transactional)
@Service
public class UserService {
    @Autowired
    private UserMapper userMapper;

    @Autowired
    private PostMapper postMapper;

    @Transactional(rollbackFor = Exception.class)
    public void createUserWithPost(User user, Post post) {
        // 插入用户
        userMapper.insert(user);

        // 插入文章
        post.setUserId(user.getId());
        postMapper.insert(post);

        // 如果抛出异常,两者都会回滚
        if (someCondition) {
            throw new RuntimeException("回滚事务");
        }
    }

    // 2. 手动事务管理
    @Autowired
    private DataSourceTransactionManager transactionManager;

    public void manualTransaction() {
        TransactionStatus status = transactionManager.getTransaction(
            new DefaultTransactionDefinition()
        );

        try {
            userMapper.insert(user);
            postMapper.insert(post);

            transactionManager.commit(status);
        } catch (Exception e) {
            transactionManager.rollback(status);
            throw e;
        }
    }
}
```

---

### SQLAlchemy 事务 (对应实现)

```python
# 1. 自动事务管理 (推荐) - 类似 @Transactional
def create_user_with_post(user_data, post_data):
    try:
        # 插入用户
        user = User(username=user_data['username'])
        db.session.add(user)
        db.session.flush()  # 刷新,获取 user.id (但不提交)

        # 插入文章
        post = Post(title=post_data['title'], user_id=user.id)
        db.session.add(post)

        # 如果抛出异常,两者都会回滚
        if some_condition:
            raise Exception("回滚事务")

        # 提交事务 - 成功则两者都插入
        db.session.commit()

    except Exception as e:
        # 回滚事务
        db.session.rollback()
        raise e

# 2. 上下文管理器 (更优雅)
from contextlib import contextmanager

@contextmanager
def transaction():
    try:
        yield db.session
        db.session.commit()
    except Exception:
        db.session.rollback()
        raise

# 使用
def create_user_with_post_v2(user_data, post_data):
    with transaction():
        user = User(username=user_data['username'])
        db.session.add(user)
        db.session.flush()

        post = Post(title=post_data['title'], user_id=user.id)
        db.session.add(post)
        # 自动提交或回滚

# 3. 装饰器方式 (类似 @Transactional)
from functools import wraps

def transactional(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            result = func(*args, **kwargs)
            db.session.commit()
            return result
        except Exception as e:
            db.session.rollback()
            raise e
    return wrapper

@transactional
def create_user_with_post_v3(user_data, post_data):
    user = User(username=user_data['username'])
    db.session.add(user)
    db.session.flush()

    post = Post(title=post_data['title'], user_id=user.id)
    db.session.add(post)
    # 自动提交或回滚
```

---

### 事务隔离级别

**MyBatis-Plus**:
```java
@Transactional(isolation = Isolation.READ_COMMITTED)
public void someMethod() {
    // ...
}
```

**SQLAlchemy**:
```python
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

engine = create_engine(
    'mysql://user:pass@localhost/db',
    isolation_level='READ COMMITTED'
)
Session = sessionmaker(bind=engine)
```

---

### 事务对比总结

| 特性 | MyBatis-Plus | SQLAlchemy |
|------|--------------|------------|
| 声明式事务 | `@Transactional` | 装饰器 (自定义) |
| 手动管理 | `TransactionManager` | `try/except` + `commit/rollback` |
| 自动回滚 | 异常时自动 | 异常时手动 (或装饰器) |
| 上下文管理 | - | `with transaction()` |
| 隔离级别 | `isolation` 参数 | engine 配置 |

---

## 10. 数据库迁移对比

### MyBatis-Plus 数据库迁移

MyBatis-Plus 本身不提供迁移工具,通常使用:

**Flyway** (推荐):
```xml
<!-- pom.xml -->
<dependency>
    <groupId>org.flywaydb</groupId>
    <artifactId>flyway-core</artifactId>
</dependency>
```

```sql
-- src/main/resources/db/migration/V1__create_users_table.sql
CREATE TABLE users (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(80) NOT NULL UNIQUE,
    email VARCHAR(120) NOT NULL UNIQUE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- V2__create_posts_table.sql
CREATE TABLE posts (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    content TEXT,
    user_id BIGINT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

**Liquibase**:
```xml
<!-- changelog.xml -->
<changeSet id="1" author="dev">
    <createTable tableName="users">
        <column name="id" type="BIGINT" autoIncrement="true">
            <constraints primaryKey="true"/>
        </column>
        <column name="username" type="VARCHAR(80)">
            <constraints unique="true" nullable="false"/>
        </column>
    </createTable>
</changeSet>
```

---

### SQLAlchemy 数据库迁移 (Flask-Migrate)

```bash
# 1. 安装
pip install flask-migrate
```

```python
# 2. 配置
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate

app = Flask(__name__)
db = SQLAlchemy(app)
migrate = Migrate(app, db)  # 配置迁移

# 3. 定义模型
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
```

```bash
# 4. 初始化迁移仓库 (只需一次)
flask db init

# 5. 生成迁移脚本 (自动检测模型变化)
flask db migrate -m "创建 users 表"

# 6. 应用迁移
flask db upgrade

# 7. 回退迁移
flask db downgrade
```

**生成的迁移文件** (`migrations/versions/xxx_创建_users_表.py`):
```python
"""创建 users 表

Revision ID: abc123
Revises:
Create Date: 2025-01-15 10:00:00
"""
from alembic import op
import sqlalchemy as sa

def upgrade():
    # 创建表
    op.create_table('users',
        sa.Column('id', sa.Integer(), nullable=False),
        sa.Column('username', sa.String(length=80), nullable=False),
        sa.Column('email', sa.String(length=120), nullable=False),
        sa.PrimaryKeyConstraint('id'),
        sa.UniqueConstraint('username'),
        sa.UniqueConstraint('email')
    )

def downgrade():
    # 回退时删除表
    op.drop_table('users')
```

---

### 修改表结构示例

**Flyway** (手写 SQL):
```sql
-- V3__add_age_to_users.sql
ALTER TABLE users ADD COLUMN age INT;
```

**Flask-Migrate** (自动生成):
```python
# 1. 修改模型
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80))
    email = db.Column(db.String(120))
    age = db.Column(db.Integer)  # 新增字段

# 2. 生成迁移 (自动检测变化)
flask db migrate -m "添加年龄字段"

# 3. 应用迁移
flask db upgrade
```

自动生成的迁移文件:
```python
def upgrade():
    op.add_column('users', sa.Column('age', sa.Integer(), nullable=True))

def downgrade():
    op.drop_column('users', 'age')
```

---

### 数据库迁移对比总结

| 特性 | Flyway/Liquibase | Flask-Migrate (Alembic) |
|------|------------------|-------------------------|
| SQL 编写 | 手写 SQL | 自动生成 (可手动修改) |
| 版本控制 | 基于文件版本号 | 基于 Alembic 版本链 |
| 自动检测 | ❌ | ✅ (检测模型变化) |
| 回退 | 手写回退 SQL | 自动生成 downgrade |
| 学习曲线 | 低 (SQL) | 中 (Python API) |

**优势**:
- **Flyway**: 更透明,完全控制 SQL
- **Flask-Migrate**: 更自动化,减少手写 SQL

---

## 11. 高级特性对比

### 代码生成器

**MyBatis-Plus 代码生成器**:
```java
public class CodeGenerator {
    public static void main(String[] args) {
        AutoGenerator mpg = new AutoGenerator();

        // 全局配置
        GlobalConfig gc = new GlobalConfig();
        gc.setOutputDir(System.getProperty("user.dir") + "/src/main/java");
        gc.setAuthor("dev");
        mpg.setGlobalConfig(gc);

        // 数据源配置
        DataSourceConfig dsc = new DataSourceConfig();
        dsc.setUrl("jdbc:mysql://localhost:3306/mydb");
        dsc.setUsername("root");
        dsc.setPassword("password");
        mpg.setDataSource(dsc);

        // 策略配置
        StrategyConfig strategy = new StrategyConfig();
        strategy.setInclude("users", "posts");  // 要生成的表
        mpg.setStrategy(strategy);

        mpg.execute();
    }
}
```

生成:
- Entity 类
- Mapper 接口
- Mapper XML
- Service 接口和实现
- Controller (可选)

---

**SQLAlchemy 代码生成** (Flask-Sqlacodegen):
```bash
# 安装
pip install flask-sqlacodegen

# 从现有数据库生成模型
flask-sqlacodegen mysql://user:pass@localhost/mydb > models.py
```

生成的模型:
```python
class User(db.Model):
    __tablename__ = 'users'

    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120))
```

**对比**: MyBatis-Plus 生成更全面,SQLAlchemy 只生成模型

---

### 逻辑删除

**MyBatis-Plus**:
```java
@TableName("users")
public class User {
    @TableId
    private Long id;

    @TableLogic  // 逻辑删除字段
    private Integer deleted;  // 0=未删除, 1=已删除
}

// 删除操作 (实际执行 UPDATE)
userMapper.deleteById(1);
// SQL: UPDATE users SET deleted=1 WHERE id=1

// 查询时自动过滤
userMapper.selectList(null);
// SQL: SELECT * FROM users WHERE deleted=0
```

**SQLAlchemy** (手动实现):
```python
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80))
    deleted = db.Column(db.Boolean, default=False)

    # 方式1: 手动过滤
    @staticmethod
    def get_active_users():
        return User.query.filter_by(deleted=False).all()

    # 方式2: 使用查询属性
    @classmethod
    def active(cls):
        return cls.query.filter_by(deleted=False)

# 软删除
user = User.query.get(1)
user.deleted = True
db.session.commit()

# 查询
users = User.active().all()  # 只查询未删除的
```

---

### 自动填充

**MyBatis-Plus** (自动填充 created_at, updated_at):
```java
public class User {
    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createdAt;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updatedAt;
}

// 配置处理器
@Component
public class MyMetaObjectHandler implements MetaObjectHandler {
    @Override
    public void insertFill(MetaObject metaObject) {
        this.strictInsertFill(metaObject, "createdAt",
                              LocalDateTime.class, LocalDateTime.now());
    }

    @Override
    public void updateFill(MetaObject metaObject) {
        this.strictUpdateFill(metaObject, "updatedAt",
                              LocalDateTime.class, LocalDateTime.now());
    }
}
```

**SQLAlchemy** (自动填充):
```python
from datetime import datetime

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80))

    # 插入时自动填充
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    # 插入和更新时自动填充
    updated_at = db.Column(db.DateTime,
                           default=datetime.utcnow,
                           onupdate=datetime.utcnow)

# 使用 - 无需手动设置时间
user = User(username='alice')
db.session.add(user)
db.session.commit()
# created_at 和 updated_at 自动设置
```

---

### 乐观锁

**MyBatis-Plus**:
```java
public class User {
    @Version  // 乐观锁版本号
    private Integer version;
}

// 更新时自动检查版本号
User user = userMapper.selectById(1);  // version = 1
user.setUsername("new name");
userMapper.updateById(user);
// SQL: UPDATE users SET username='new name', version=2
//      WHERE id=1 AND version=1
```

**SQLAlchemy** (手动实现):
```python
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80))
    version = db.Column(db.Integer, default=1)

# 更新时检查版本
user = User.query.get(1)  # version = 1
user.username = 'new name'
user.version += 1

rows = db.session.query(User).filter(
    User.id == user.id,
    User.version == user.version - 1  # 检查旧版本
).update({
    'username': user.username,
    'version': user.version
})

if rows == 0:
    raise Exception("数据已被修改,请刷新后重试")

db.session.commit()
```

---

## 12. 完整示例对比

### 需求: 用户管理系统

功能:
- 创建用户
- 查询用户列表 (分页、搜索)
- 更新用户
- 删除用户 (逻辑删除)

---

### MyBatis-Plus 完整实现

```java
// 1. 实体类
@Data
@TableName("users")
public class User {
    @TableId(type = IdType.AUTO)
    private Long id;

    private String username;
    private String email;
    private Integer age;

    @TableLogic
    private Integer deleted;

    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createdAt;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private LocalDateTime updatedAt;
}

// 2. Mapper
@Mapper
public interface UserMapper extends BaseMapper<User> {}

// 3. Service
@Service
public class UserService extends ServiceImpl<UserMapper, User> {

    // 创建用户
    public User createUser(UserDTO dto) {
        User user = new User();
        BeanUtils.copyProperties(dto, user);
        save(user);
        return user;
    }

    // 分页查询
    public IPage<User> searchUsers(int page, int size, String keyword) {
        Page<User> pageObj = new Page<>(page, size);

        LambdaQueryWrapper<User> wrapper = new LambdaQueryWrapper<>();
        if (StringUtils.isNotEmpty(keyword)) {
            wrapper.like(User::getUsername, keyword)
                   .or()
                   .like(User::getEmail, keyword);
        }
        wrapper.orderByDesc(User::getCreatedAt);

        return page(pageObj, wrapper);
    }

    // 更新用户
    public User updateUser(Long id, UserDTO dto) {
        User user = getById(id);
        if (user == null) {
            throw new NotFoundException("用户不存在");
        }

        BeanUtils.copyProperties(dto, user, "id", "createdAt");
        updateById(user);
        return user;
    }

    // 删除用户 (逻辑删除)
    public void deleteUser(Long id) {
        removeById(id);  // 自动逻辑删除
    }
}

// 4. Controller
@RestController
@RequestMapping("/api/users")
public class UserController {
    @Autowired
    private UserService userService;

    @PostMapping
    public User create(@RequestBody UserDTO dto) {
        return userService.createUser(dto);
    }

    @GetMapping
    public IPage<User> list(
        @RequestParam(defaultValue = "1") int page,
        @RequestParam(defaultValue = "10") int size,
        @RequestParam(required = false) String keyword
    ) {
        return userService.searchUsers(page, size, keyword);
    }

    @PutMapping("/{id}")
    public User update(@PathVariable Long id, @RequestBody UserDTO dto) {
        return userService.updateUser(id, dto);
    }

    @DeleteMapping("/{id}")
    public void delete(@PathVariable Long id) {
        userService.deleteUser(id);
    }
}
```

---

### SQLAlchemy 完整实现 (对应)

```python
# 1. 模型定义
from datetime import datetime
from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class User(db.Model):
    __tablename__ = 'users'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    username = db.Column(db.String(80), nullable=False)
    email = db.Column(db.String(120), nullable=False)
    age = db.Column(db.Integer)

    # 逻辑删除
    deleted = db.Column(db.Boolean, default=False)

    # 自动填充
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime,
                           default=datetime.utcnow,
                           onupdate=datetime.utcnow)

    def to_dict(self):
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'age': self.age,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

# 2. Service 层 (可选,这里直接在路由中实现)

# 3. 路由 (对应 Controller)
from flask import Blueprint, request, jsonify

api_bp = Blueprint('api', __name__, url_prefix='/api')

# 创建用户
@api_bp.route('/users', methods=['POST'])
def create_user():
    data = request.get_json()

    user = User(
        username=data['username'],
        email=data['email'],
        age=data.get('age')
    )
    db.session.add(user)
    db.session.commit()

    return jsonify(user.to_dict()), 201

# 分页查询
@api_bp.route('/users', methods=['GET'])
def list_users():
    page = request.args.get('page', 1, type=int)
    size = request.args.get('size', 10, type=int)
    keyword = request.args.get('keyword', '')

    # 构建查询
    query = User.query.filter_by(deleted=False)

    # 搜索条件
    if keyword:
        query = query.filter(
            db.or_(
                User.username.like(f'%{keyword}%'),
                User.email.like(f'%{keyword}%')
            )
        )

    # 排序
    query = query.order_by(User.created_at.desc())

    # 分页
    pagination = query.paginate(page=page, per_page=size, error_out=False)

    return jsonify({
        'items': [user.to_dict() for user in pagination.items],
        'total': pagination.total,
        'pages': pagination.pages,
        'current': pagination.page,
        'size': pagination.per_page
    })

# 更新用户
@api_bp.route('/users/<int:user_id>', methods=['PUT'])
def update_user(user_id):
    user = User.query.get_or_404(user_id)

    data = request.get_json()
    user.username = data.get('username', user.username)
    user.email = data.get('email', user.email)
    user.age = data.get('age', user.age)

    db.session.commit()

    return jsonify(user.to_dict())

# 删除用户 (逻辑删除)
@api_bp.route('/users/<int:user_id>', methods=['DELETE'])
def delete_user(user_id):
    user = User.query.get_or_404(user_id)
    user.deleted = True
    db.session.commit()

    return '', 204

# 4. 应用工厂
def create_app():
    app = Flask(__name__)
    app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///users.db'

    db.init_app(app)
    app.register_blueprint(api_bp)

    return app
```

---

### 代码量对比

| 部分 | MyBatis-Plus | SQLAlchemy | 说明 |
|------|--------------|------------|------|
| 实体/模型 | ~30 行 | ~25 行 | SQLAlchemy 更简洁 |
| Mapper | 1 行 (接口) | 0 行 (无需) | SQLAlchemy 无需 Mapper |
| Service | ~50 行 | 0 行 (可选) | SQLAlchemy 可直接在路由操作 |
| Controller/路由 | ~40 行 | ~60 行 | 相近 |
| **总计** | ~120 行 | ~85 行 | SQLAlchemy 减少 30% 代码 |

---

## 总结对比表

### 核心对比

| 特性 | MyBatis | MyBatis-Plus | MyBatis-Flex | SQLAlchemy |
|------|---------|--------------|--------------|------------|
| **ORM 类型** | 半自动 | 增强 ORM | 增强 ORM | 全自动 ORM |
| **SQL 编写** | 必须手写 | 单表免写 | 单表免写 | 几乎不需要 |
| **CRUD** | 手动 | BaseMapper | BaseMapper | 自动 |
| **条件构造** | XML/注解 | QueryWrapper | QueryWrapper | filter() |
| **关系映射** | 手动 | 手动 | 改进 | 原生支持 |
| **分页** | 插件 | 插件 | 内置 | 内置 |
| **迁移** | Flyway/Liquibase | 同左 | 同左 | Flask-Migrate |
| **学习曲线** | 高 | 中 | 中 | 中低 |
| **代码量** | 多 | 中 | 中 | 少 |

---

### 使用建议

**选择 MyBatis**:
- 复杂 SQL 优化需求
- 需要完全控制 SQL
- 遗留系统维护

**选择 MyBatis-Plus/Flex**:
- 中大型 Spring Boot 项目
- 需要快速开发
- 单表操作为主

**选择 SQLAlchemy**:
- Flask/Python 生态
- 快速原型开发
- 关系映射复杂
- 数据分析/机器学习应用

---

### 关键区别

1. **代码量**: SQLAlchemy < MyBatis-Plus < MyBatis
2. **自动化**: SQLAlchemy > MyBatis-Plus > MyBatis
3. **灵活性**: MyBatis > MyBatis-Plus ≈ SQLAlchemy
4. **关系处理**: SQLAlchemy >> MyBatis-Plus > MyBatis

---

### 对于 Java 开发者

**如果你熟悉**:
- **MyBatis** → SQLAlchemy 会感觉更自动化,减少 XML 配置
- **MyBatis-Plus** → SQLAlchemy 非常相似,API 对应关系清晰
- **JPA/Hibernate** → SQLAlchemy 就是 Python 版的 JPA!

**学习建议**:
1. 把 SQLAlchemy 当作 "Python 版 MyBatis-Plus"
2. 重点理解 `db.session` (类似事务管理)
3. 掌握 `filter()` 和 `filter_by()` (对应 QueryWrapper)
4. 学习关系映射 (SQLAlchemy 的强项)

---

希望这份对比文档能帮助你快速理解 SQLAlchemy! 🚀
