# 图形 API 详解:OpenGL vs Vulkan vs DirectX

## 🎨 什么是图形 API?

**图形 API** = 程序员与 GPU(显卡)通信的接口

```
你的程序
    ↓ 调用图形API
OpenGL/Vulkan/DirectX
    ↓ 驱动程序
   GPU(显卡)
    ↓ 渲染
  屏幕显示
```

**类比理解**:
```
图形API = 翻译官

程序员说:"画一个三角形"
  ↓ (通过图形API)
GPU听到:"在这些坐标放这些像素"
```

---

## 🌍 三大主流图形 API

### 对比总览

| API | 平台 | 年代 | 难度 | 性能 |
|-----|------|------|------|------|
| **OpenGL** | 跨平台 | 1992 | ⭐⭐ | 中 |
| **Vulkan** | 跨平台 | 2016 | ⭐⭐⭐⭐⭐ | 高 |
| **DirectX** | Windows | 1995 | ⭐⭐⭐ | 高 |
| **Metal** | macOS/iOS | 2014 | ⭐⭐⭐⭐ | 高 |

---

## 📐 OpenGL - 经典图形 API

### 简介

**OpenGL**(Open Graphics Library) 是最古老、最通用的跨平台图形API。

### 优势

✅ **跨平台**
```
Windows ✓
Linux ✓
macOS ✓ (已弃用,但仍可用)
Web(WebGL) ✓
Android(OpenGL ES) ✓
```

✅ **易学**
```c
// 画三角形(简化版)
glBegin(GL_TRIANGLES);
  glVertex2f(0.0, 0.5);
  glVertex2f(-0.5, -0.5);
  glVertex2f(0.5, -0.5);
glEnd();
```

✅ **生态成熟**
- 大量教程(LearnOpenGL.com)
- 工具链完善(GLFW, GLAD)
- 社区活跃

### 劣势

❌ **性能较低**
- 驱动开销大
- 单线程渲染
- 自动状态管理(有隐藏成本)

❌ **macOS已弃用**
- Apple推Metal,不再更新OpenGL

### OpenGL 版本

| 版本 | 年份 | 特性 |
|------|------|------|
| **2.0** | 2004 | GLSL着色器 |
| **3.3** | 2010 | 核心模式(Core Profile) |
| **4.6** | 2017 | 最新特性 |

**推荐学习**: OpenGL 3.3+ (Core Profile)

### 代码示例

```c
// 现代OpenGL(3.3+)示例

// 1. 创建顶点缓冲区
GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 2. 编译着色器
GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertex_shader, 1, &vertex_source, NULL);
glCompileShader(vertex_shader);

// 3. 绘制
glUseProgram(shader_program);
glBindVertexArray(vao);
glDrawArrays(GL_TRIANGLES, 0, 3);
```

---

## 🔥 Vulkan - 现代低级 API

### 简介

**Vulkan** 是现代、低级、高性能的跨平台图形API,OpenGL的接班人。

### 优势

✅ **极致性能**
```
性能对比(相同场景):
OpenGL: 1000 draw calls → 30 FPS
Vulkan:  1000 draw calls → 60 FPS
```

✅ **多线程友好**
```c
// 多个线程同时提交命令
Thread1: record_commands(cmd_buffer_1);
Thread2: record_commands(cmd_buffer_2);
Thread3: record_commands(cmd_buffer_3);

// 一次性提交
submit_all_buffers([cmd_buffer_1, cmd_buffer_2, cmd_buffer_3]);
```

✅ **明确控制**
- 显式内存管理
- 同步由程序员控制
- 无隐藏成本

### 劣势

❌ **极其复杂**
```
OpenGL绘制三角形: ~50行代码
Vulkan绘制三角形: ~1000行代码
```

❌ **学习曲线陡峭**
- 需要理解GPU架构
- 手动管理内存
- 同步原语复杂

❌ **调试困难**
- 错误信息少
- 需要Validation Layers

### 何时使用 Vulkan?

✅ **适合**:
- 3A大作(需要极致性能)
- 引擎开发(Unity/Unreal已用Vulkan)
- 对GPU深度理解

❌ **不适合**:
- 初学者
- 简单项目
- 快速原型

### Vulkan vs OpenGL 代码量对比

**OpenGL**(绘制三角形):
```c
// ~50行代码
glClear(GL_COLOR_BUFFER_BIT);
glUseProgram(program);
glDrawArrays(GL_TRIANGLES, 0, 3);
```

**Vulkan**(绘制三角形):
```c
// ~1000行代码(简化列表)
1. 创建Instance
2. 选择PhysicalDevice
3. 创建LogicalDevice
4. 创建Queue
5. 创建Swapchain
6. 创建ImageView
7. 创建RenderPass
8. 创建Framebuffer
9. 创建CommandPool
10. 分配CommandBuffer
11. 创建Semaphore/Fence
12. 录制命令
13. 提交命令
14. Present
// ... 还有更多
```

---

## 🪟 DirectX - Windows 专用 API

### 简介

**DirectX** 是微软开发的Windows平台图形API(和其他多媒体API的集合)。

### DirectX 家族

| 模块 | 用途 |
|------|------|
| **Direct3D** | 3D图形 |
| **Direct2D** | 2D图形 |
| **DirectSound** | 音频 |
| **DirectInput** | 输入设备 |
| **DirectWrite** | 文字渲染 |

**注意**: 通常说的"DirectX"指的是**Direct3D**。

### DirectX 版本

| 版本 | 年份 | 特性 |
|------|------|------|
| **DirectX 9** | 2002 | Shader Model 3.0 |
| **DirectX 11** | 2009 | Compute Shader, Tessellation |
| **DirectX 12** | 2015 | 低级API,类似Vulkan |

### 优势

✅ **Windows性能最优**
- 驱动优化好
- Xbox兼容

✅ **开发工具完善**
- Visual Studio集成
- PIX调试器
- DirectX SDK

### 劣势

❌ **仅Windows**
- 无法跨平台
- macOS/Linux不支持

❌ **DirectX 12复杂**
- 和Vulkan一样难学
- 需要手动管理资源

### DirectX 11 vs DirectX 12

| 特性 | DX11 | DX12 |
|------|------|------|
| **难度** | 中等 | 困难 |
| **性能** | 好 | 最优 |
| **多线程** | 有限 | 完全支持 |
| **代码量** | 适中 | 大 |

**建议**: 学习时用DX11,产品用DX12。

---

## 🍎 Metal - Apple 专用 API

### 简介

**Metal** 是Apple为macOS/iOS开发的现代图形API。

### 特点

✅ **Apple平台最优**
- iPhone/iPad/Mac专用优化
- 与硬件深度集成

✅ **相对简单**(比Vulkan/DX12)
- API设计现代但不过度复杂

❌ **仅Apple平台**
- Windows/Android不支持

---

## 🛠️ 图形 API 抽象层

### 为什么需要抽象层?

**问题**: 每个平台用不同API太麻烦!

```
Windows → DirectX
macOS → Metal
Linux → OpenGL/Vulkan
Web → WebGL
```

**解决**: 用抽象层统一接口!

### 主流抽象层

| 库名 | 特点 | Soluna使用 |
|------|------|-----------|
| **Sokol** | 轻量,单头文件 | ✅ 使用 |
| **BGFX** | 功能全,稍重 | ❌ 未用 |
| **SDL2** | 全功能多媒体库 | ❌ 未用 |

### Sokol 示例

```c
// Sokol自动选择后端
#if defined(_WIN32)
  // 使用D3D11
#elif defined(__APPLE__)
  // 使用Metal
#elif defined(__linux__)
  // 使用OpenGL
#elif defined(__EMSCRIPTEN__)
  // 使用WebGL
#endif

// 你的代码:统一接口
sg_draw(0, 3, 1);  // 在所有平台工作!
```

---

## 📊 选择图形 API 的决策树

```
你的项目需求?

├─ 跨平台
│  ├─ 简单项目 → OpenGL 3.3+
│  ├─ 高性能 → Vulkan
│  └─ 快速开发 → Sokol/BGFX(抽象层)
│
├─ 仅Windows
│  ├─ 中等项目 → DirectX 11
│  └─ 3A大作 → DirectX 12
│
├─ 仅Apple
│  └─ Metal
│
└─ Web游戏
   └─ WebGL(基于OpenGL ES)
```

---

## 🎓 学习建议

### 初学者路径

**第1步: 学OpenGL**(1-2个月)
```
推荐教程: LearnOpenGL (https://learnopengl.com/)

学习内容:
1. Hello Triangle
2. 纹理和变换
3. 光照基础
4. 模型加载
```

**第2步: 用抽象层**(实践)
```
学习Sokol:
1. 阅读示例
2. 画三角形
3. 加载纹理
4. 实现小游戏
```

**第3步: 深入(可选)**
```
如果需要极致性能:
- 学习Vulkan
- 学习DX12
```

### 云风的选择

```
ejoy2d → OpenGL ES 2.0 (手机)
Ant Engine → BGFX (抽象层)
Soluna → Sokol (轻量抽象层)
```

**逻辑**: 用抽象层,专注游戏逻辑,不纠结底层API。

---

## 💡 常见误解

### ❌ 误解1: Vulkan一定比OpenGL快

**真相**: 对简单项目,OpenGL可能更快(驱动优化好)。

只有复杂场景(>5000 draw calls)Vulkan才显著更快。

### ❌ 误解2: 学OpenGL没用,都用Vulkan了

**真相**:
- 大量现有项目还在用OpenGL
- 学OpenGL理解概念更容易
- WebGL基于OpenGL ES

### ❌ 误解3: DirectX是游戏专用

**真相**: DirectX也用于:
- 视频播放
- CAD软件
- 机器学习(DirectML)

---

## 🔗 学习资源

### OpenGL

- **LearnOpenGL**: https://learnopengl.com/ (最佳教程)
- **OpenGL Wiki**: https://www.khronos.org/opengl/wiki/

### Vulkan

- **Vulkan Tutorial**: https://vulkan-tutorial.com/
- **Vulkan Guide**: https://vkguide.dev/

### DirectX

- **RasterTek**: http://www.rastertek.com/tutdx11.html
- **Microsoft Docs**: https://docs.microsoft.com/en-us/windows/win32/direct3d11/

---

## 📝 总结

### 各API定位

| API | 定位 | 适合人群 |
|-----|------|---------|
| **OpenGL** | 通用学习 | 初学者 |
| **Vulkan** | 极致性能 | 专家 |
| **DirectX** | Windows专用 | Windows开发者 |
| **Sokol/BGFX** | 跨平台抽象 | 实用主义者 |

### 学习优先级

1. **先学OpenGL** - 理解图形编程概念
2. **用Sokol实践** - 跨平台快速开发
3. **深入Vulkan**(可选) - 高级性能优化

---

**下一步**: 阅读 [常见技术库介绍](./05-常见技术库介绍.md) 了解OpenCV等库的用途
