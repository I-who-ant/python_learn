# Maven 与 npm 对比

## 核心问题

**Maven 和 npm 是一样的东西吗?**

**简短回答**:相似但不同。

- **相同点**:都管理依赖
- **不同点**:Maven 是"一站式"构建工具,npm 只管理包

---

## 快速对比

| 特性           | Maven         | npm |
|--------------|---------------|-----|
| **诞生时间**     | 2004年         | 2010年 |
| **语言**       | Java          | JavaScript |
| **配置文件**     | pom.xml       | package.json |
| **锁文件**      | ❌ 无           | package-lock.json |
| **依赖管理**     | ✅ 是           | ✅ 是 |
| **编译**       | ✅ 是 (javac)   | ❌ 否 |
| **打包**       | ✅ 是 (jar/war) | ❌ 否 (需要 Webpack) |
| **测试**       | ✅ 是           | ✅ 是 (但需要工具) |
| **脚本运行**     | ✅ 有           | ✅ 有 (更常用) |
| **中央仓库**     | Maven Central | npm Registry |

---

## npm 详解

### npm 是什么?

**npm** = **Node** **Package** **Manager**

**定位**:JavaScript 的包管理器

**职责**:
1. 安装/卸载包
2. 管理依赖版本
3. 运行脚本
4. 发布包

**不负责**:
- 编译 (JavaScript 不需要编译)
- 打包 (交给 Webpack/Vite)
- 代码转换 (交给 Babel)

### package.json


```json
{
  "name": "my-vue-blog",
  "version": "1.0.0",
  "description": "My awesome blog",

  // 脚本命令
  "scripts": {
    "dev": "vite",                  // npm run dev
    "build": "vite build",          // npm run build
    "test": "jest",                 // npm run test
    "lint": "eslint src/"           // npm run lint
  },

  // 生产依赖
  "dependencies": {
    "vue": "^3.5.18",
    "vue-router": "^4.2.5",
    "pinia": "^2.1.7"
  },

  // 开发依赖
  "devDependencies": {
    "vite": "^7.0.6",
    "@vitejs/plugin-vue": "^5.0.0",
    "typescript": "^5.6.3"
  },

  // 引擎要求
  "engines": {
    "node": ">=22.0.0"
  }
}
```

**关键特点**:
- JSON 格式 (易读易写)
- 区分生产/开发依赖
- scripts 灵活强大
- 轻量级元数据

### npm 工作流程

#### 安装依赖

```bash
# 安装所有依赖
npm install
# 或
npm i

# 安装特定包
npm install vue
npm install -D vite  # 开发依赖

# 全局安装
npm install -g @vue/cli
```



**过程**:

```
1. 读取 package.json
2. 解析依赖树
3. 检查 package-lock.json (如果存在)
4. 下载包到 node_modules/
5. 更新 package-lock.json
```



#### 运行脚本


```bash
npm run dev    # 运行 vite
npm run build  # 运行 vite build
npm test       # 运行测试 (特殊,可以省略 run)
npm start      # 运行 start (特殊,可以省略 run)
```




#### 发布包


```bash
# 登录
npm login

# 发布
npm publish

# 更新版本并发布
npm version patch  # 1.0.0 → 1.0.1
npm publish
```





### npm 的局限

**不包含编译**:
```bash
# TypeScript 需要额外工具
npm install typescript
npx tsc  # 需要手动运行编译器

# 打包需要额外工具
npm install webpack webpack-cli
npx webpack  # 需要手动运行打包工具
```

**这就是为什么 JavaScript 项目通常需要多个工具**:
- npm:依赖管理
- Webpack/Vite:打包
- Babel:转译
- ESLint:代码检查
- Jest:测试

---

## Maven 详解

### Maven 是什么?

**Maven** = 项目管理和构建自动化工具

**定位**:Java 的"一站式"构建工具

**职责**:
1. 依赖管理
2. 编译 Java 代码
3. 运行测试
4. 打包成 JAR/WAR
5. 生成文档
6. 部署到仓库

**全包了!**

### pom.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project xmlns="http://maven.apache.org/POM/4.0.0">
  <modelVersion>4.0.0</modelVersion>

  <!-- 项目坐标 -->
  <groupId>com.example</groupId>
  <artifactId>my-spring-app</artifactId>
  <version>1.0.0-SNAPSHOT</version>
  <packaging>jar</packaging>

  <name>My Spring App</name>
  <description>My awesome Spring Boot application</description>

  <!-- 属性 -->
  <properties>
    <java.version>17</java.version>
    <spring.version>3.2.0</spring.version>
  </properties>

  <!-- 依赖 -->
  <dependencies>
    <!-- Spring Boot -->
    <dependency>
      <groupId>org.springframework.boot</groupId>
      <artifactId>spring-boot-starter-web</artifactId>
      <version>${spring.version}</version>
    </dependency>

    <!-- 数据库 -->
    <dependency>
      <groupId>org.postgresql</groupId>
      <artifactId>postgresql</artifactId>
      <version>42.6.0</version>
    </dependency>

    <!-- 测试 (scope=test 表示仅测试时使用) -->
    <dependency>
      <groupId>org.springframework.boot</groupId>
      <artifactId>spring-boot-starter-test</artifactId>
      <version>${spring.version}</version>
      <scope>test</scope>
    </dependency>
  </dependencies>

  <!-- 构建配置 -->
  <build>
    <plugins>
      <!-- 编译插件 -->
      <plugin>
        <groupId>org.apache.maven.plugins</groupId>
        <artifactId>maven-compiler-plugin</artifactId>
        <version>3.11.0</version>
        <configuration>
          <source>17</source>
          <target>17</target>
        </configuration>
      </plugin>

      <!-- Spring Boot 打包插件 -->
      <plugin>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-maven-plugin</artifactId>
      </plugin>
    </plugins>
  </build>
</project>
```

**关键特点**:
- XML 格式 (冗长但严格)
- 约定优于配置
- 插件系统强大
- 完整的项目元数据

### Maven 工作流程

#### 标准目录结构

Maven 强制使用标准结构 (约定优于配置):


```
my-spring-app/
├── pom.xml
├── src/
│   ├── main/
│   │   ├── java/          # Java 源代码
│   │   │   └── com/example/
│   │   │       └── Application.java
│   │   ├── resources/     # 配置文件
│   │   │   └── application.properties
│   │   └── webapp/        # Web 资源 (仅 WAR 项目)
│   └── test/
│       ├── java/          # 测试代码
│       └── resources/     # 测试资源
└── target/                # 编译输出 (自动生成)
    ├── classes/
    ├── test-classes/
    └── my-spring-app-1.0.0.jar
```




#### 构建生命周期

Maven 有三个内置生命周期:

**1. Clean (清理)**:
```bash
mvn clean  # 删除 target/ 目录
```


**2. Default (构建)**:
```bash
mvn validate   # 验证项目正确性
mvn compile    # 编译源代码 → target/classes/
mvn test       # 运行单元测试
mvn package    # 打包成 JAR/WAR → target/my-app.jar
mvn verify     # 运行集成测试
mvn install    # 安装到本地仓库 (~/.m2/repository/)
mvn deploy     # 部署到远程仓库
```

**3. Site (文档)**:
```bash
mvn site       # 生成项目文档网站
```

**完整构建**:
```bash
mvn clean package

# 执行了:
# 1. clean:     删除 target/
# 2. validate:  验证项目
# 3. compile:   编译 Java 代码
# 4. test:      运行测试
# 5. package:   打包成 JAR
```

#### 依赖管理


```bash
# Maven 自动下载依赖到 ~/.m2/repository/

# 例如:
# <dependency>
#   <groupId>org.springframework.boot</groupId>
#   <artifactId>spring-boot-starter-web</artifactId>
#   <version>3.2.0</version>
# </dependency>

# Maven 下载到:
# ~/.m2/repository/org/springframework/boot/spring-boot-starter-web/3.2.0/
#   ├── spring-boot-starter-web-3.2.0.jar
#   ├── spring-boot-starter-web-3.2.0.pom
#   └── ...
```






**传递依赖**:
```
你的项目依赖:
└── spring-boot-starter-web:3.2.0
    ├── spring-web:6.1.0
    ├── spring-webmvc:6.1.0
    ├── tomcat-embed-core:10.1.15
    └── ... (数十个传递依赖)

Maven 自动解析并下载所有依赖
```



### Maven 的优势

**一站式解决方案**:
```bash
mvn clean package

# 一条命令完成:
# ✅ 清理旧文件
# ✅ 编译 Java 代码
# ✅ 运行测试
# ✅ 打包成 JAR
# ✅ 所有依赖自动管理
```


**对比 npm**:
```bash
npm run build

# 只是运行一个脚本,实际工作由其他工具完成:
# - TypeScript → tsc 编译
# - 打包 → Vite/Webpack
# - 测试 → Jest (需要单独运行)
```


## 详细对比

### 1. 依赖声明

#### npm (package.json)

```json
{
  "dependencies": {
    "vue": "^3.5.18",           // ^ 表示兼容版本
    "axios": "~1.6.0",          // ~ 表示补丁版本
    "lodash": "4.17.21"         // 精确版本
  }
}
```

**语义化版本**:
- `^3.5.18`:允许 3.x.x (不升级大版本)
- `~1.6.0`:允许 1.6.x (只升级补丁版本)
- `4.17.21`:精确版本



#### Maven (pom.xml)

```xml
<dependencies>
  <dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
    <version>3.2.0</version>
  </dependency>

  <!-- 范围版本 (不推荐) -->
  <dependency>
    <groupId>junit</groupId>
    <artifactId>junit</artifactId>
    <version>[4.12,5.0)</version>  <!-- 4.12 ≤ version < 5.0 -->
  </dependency>
</dependencies>
```



**Maven 通常使用精确版本**,避免"依赖地狱"

### 2. 依赖安装

#### npm

```bash
npm install

# 安装位置:
./node_modules/
├── vue/
├── axios/
└── lodash/

# 每个项目独立的 node_modules
```

**特点**:
- 项目本地安装
- 每个项目有自己的副本
- 占用磁盘空间大

#### Maven

```bash
mvn package

# 安装位置:
~/.m2/repository/
├── org/springframework/boot/spring-boot-starter-web/3.2.0/
├── com/google/guava/guava/32.1.0/
└── ...

# 全局共享仓库
```

**特点**:
- 全局共享仓库
- 多个项目复用同一个 JAR
- 节省磁盘空间

### 3. 版本锁定

#### npm (package-lock.json)

```json
{
  "name": "my-app",
  "version": "1.0.0",
  "lockfileVersion": 3,
  "packages": {
    "node_modules/vue": {
      "version": "3.5.18",
      "resolved": "https://registry.npmjs.org/vue/-/vue-3.5.18.tgz",
      "integrity": "sha512-xxx",
      "dependencies": {
        "@vue/compiler-dom": "3.5.18",
        "@vue/runtime-dom": "3.5.18"
      }
    }
  }
}
```

**作用**:
- 锁定所有依赖的精确版本
- 保证不同环境安装相同版本
- 自动生成

#### Maven

**Maven 没有 lock 文件!**

**问题**:
```
开发环境: spring-boot:3.2.0 → 传递依赖 spring-web:6.1.0
生产环境: spring-boot:3.2.0 → 传递依赖 spring-web:6.1.1 (如果仓库更新了)

可能导致不一致!
```

**解决方案**:
- 使用精确版本
- 使用依赖管理 (dependencyManagement)
- 使用 Maven Wrapper (锁定 Maven 版本)

### 4. 脚本运行

#### npm

```json
{
  "scripts": {
    "dev": "vite",
    "build": "vite build && echo 'Build complete!'",
    "test": "jest --coverage",
    "lint": "eslint . && prettier --check .",
    "deploy": "npm run build && scp dist/* server:/var/www/"
  }
}
```

**特点**:
- 非常灵活
- 可以串联多个命令
- 支持钩子 (pretest, posttest)
- 常用于日常开发

#### Maven

```xml
<build>
  <plugins>
    <plugin>
      <groupId>org.codehaus.mojo</groupId>
      <artifactId>exec-maven-plugin</artifactId>
      <executions>
        <execution>
          <id>run-script</id>
          <phase>package</phase>
          <goals>
            <goal>exec</goal>
          </goals>
          <configuration>
            <executable>bash</executable>
            <arguments>
              <argument>deploy.sh</argument>
            </arguments>
          </configuration>
        </execution>
      </executions>
    </plugin>
  </plugins>
</build>
```

**特点**:
- XML 配置冗长
- 绑定到生命周期阶段
- 主要用于构建流程
- 很少用于日常脚本

### 5. 多项目管理

#### npm (Monorepo: workspaces)

```json
{
  "name": "my-monorepo",
  "workspaces": [
    "packages/*"
  ]
}
```

```
my-monorepo/
├── package.json
└── packages/
    ├── web-app/
    │   └── package.json
    ├── mobile-app/
    │   └── package.json
    └── shared-lib/
        └── package.json
```

```bash
npm install  # 一次性安装所有子项目依赖
```

#### Maven (Multi-module)

```xml
<!-- 父 pom.xml -->
<project>
  <groupId>com.example</groupId>
  <artifactId>parent</artifactId>
  <version>1.0.0</version>
  <packaging>pom</packaging>

  <modules>
    <module>web-app</module>
    <module>mobile-app</module>
    <module>shared-lib</module>
  </modules>
</project>
```

```
my-project/
├── pom.xml (父)
├── web-app/
│   └── pom.xml (子)
├── mobile-app/
│   └── pom.xml (子)
└── shared-lib/
    └── pom.xml (子)
```

```bash
mvn clean package  # 一次性构建所有模块
```

---

## 生态系统对比

### npm 生态

**中央仓库**:https://www.npmjs.com

**统计** (2024):
- 包数量:300 万+
- 每周下载:500 亿+
- 活跃开发者:数百万

**特点**:
- 发布门槛低 (任何人都可以发布)
- 包数量巨大
- 质量参差不齐
- 微包现象严重

**流行包**:
- react (前端框架)
- express (Web 框架)
- lodash (工具库)
- axios (HTTP 客户端)

### Maven 生态

**中央仓库**:https://repo.maven.apache.org/maven2/

**统计** (2024):
- 包数量:50 万+
- 每月下载:1000 亿+
- 企业用户为主

**特点**:
- 发布门槛高 (需要 GPG 签名等)
- 包质量较高
- 企业级库为主
- 依赖数量少

**流行包**:
- spring-boot (Web 框架)
- junit (测试框架)
- guava (Google 工具库)
- jackson (JSON 处理)

---

## 实际案例对比

### 案例1:添加 HTTP 客户端

#### npm (JavaScript)

```bash
# 安装
npm install axios

# 使用
```
```javascript
import axios from 'axios'

const response = await axios.get('https://api.example.com/data')
console.log(response.data)
```

#### Maven (Java)


```xml
<!-- 添加到 pom.xml -->
<dependency>
  <groupId>com.squareup.okhttp3</groupId>
  <artifactId>okhttp</artifactId>
  <version>4.12.0</version>
</dependency>
```


```java
// 使用
import okhttp3.*;

OkHttpClient client = new OkHttpClient();
Request request = new Request.Builder()
    .url("https://api.example.com/data")
    .build();

try (Response response = client.newCall(request).execute()) {
    System.out.println(response.body().string());
}
```

### 案例2:完整构建流程

#### JavaScript 项目


```bash
# 1. 安装依赖
npm install

# 2. 开发
npm run dev  # 启动 Vite 开发服务器

# 3. 测试
npm test     # 运行 Jest

# 4. 构建
npm run build  # Vite 打包

# 5. 部署
scp -r dist/* server:/var/www/
```



**涉及的工具**:
- npm (依赖管理)
- Vite (开发服务器 + 打包)
- Jest (测试)
- ESLint (代码检查,可选)

#### Java 项目

```bash
# 1. 构建 + 测试 + 打包 (一条命令)
mvn clean package

# 2. 运行
java -jar target/my-app-1.0.0.jar

# 3. 部署
scp target/my-app-1.0.0.jar server:/opt/app/
```


**只需一个工具:Maven**

---


## Gradle:Maven 的现代替代

### Gradle 简介

**Gradle** = Maven 的改进版 (2012年)

**优点**:
- 使用 Groovy/Kotlin DSL (比 XML 简洁)
- 增量构建 (更快)
- 灵活性更高

**build.gradle (Groovy)**:
```groovy
plugins {
    id 'java'
    id 'org.springframework.boot' version '3.2.0'
}

group = 'com.example'
version = '1.0.0'

dependencies {
    implementation 'org.springframework.boot:spring-boot-starter-web'
    testImplementation 'org.springframework.boot:spring-boot-starter-test'
}
```

**对比 Maven pom.xml**:

```
Maven: 50 行 XML
Gradle: 10 行 Groovy
功能相同!
```


---

## 总结

### 核心差异

| 维度 | npm | Maven |
|------|-----|-------|
| **定位** | 包管理器 | 构建工具 + 包管理器 |
| **职责** | 依赖管理 | 依赖 + 编译 + 测试 + 打包 |
| **配置复杂度** | ⭐⭐ (简单) | ⭐⭐⭐⭐ (复杂) |
| **灵活性** | ⭐⭐⭐⭐⭐ (非常灵活) | ⭐⭐⭐ (约定多) |
| **学习曲线** | 平缓 | 陡峭 |
| **生态规模** | 300万+ 包 | 50万+ 包 |
| **包质量** | 参差不齐 | 较高 |

### 为什么不同?

**JavaScript 的特点**:
- 不需要编译 (直接运行)
- 前端需要打包工具 (Webpack/Vite)
- 工具链分离 (npm + Vite + Jest)

**Java 的特点**:
- 必须编译 (javac)
- 企业级项目复杂
- "一站式"工具更方便 (Maven)

### 现代趋势

**JavaScript**:
```
趋势:工具链整合
- Vite (开发服务器 + 打包)
- Bun (包管理 + 运行时 + 打包)
```

**Java**:
```
趋势:从 Maven 迁移到 Gradle
- 配置更简洁
- 构建更快
- 但 Maven 仍是主流
```

### 如果你是...

**前端开发者**:
- 主要使用:npm + Vite
- Maven 了解即可

**Java 开发者**:
- 主要使用:Maven 或 Gradle
- npm 了解即可

**全栈开发者**:
- 前端:npm + Vite
- 后端:Maven (Java) 或 无需构建工具 (Python/Node)

---

**下一步**:阅读 [05-构建工具演进史.md](./05-构建工具演进史.md) 了解构建工具的历史演变


