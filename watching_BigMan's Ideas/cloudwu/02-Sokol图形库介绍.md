# Sokol 图形库介绍

## 📚 什么是 Sokol?

**Sokol** 是一套用 C 语言编写的**单头文件(single-header)**跨平台库,专注于提供简洁的图形 API 抽象层。

### 核心特点

| 特性 | 说明 |
|------|------|
| **单头文件** | 每个模块只有一个 `.h` 文件 |
| **跨平台** | Windows/Linux/macOS/iOS/Android/Web |
| **多后端** | OpenGL/Metal/D3D11/WebGL |
| **零依赖** | 不依赖其他库(除系统 API) |
| **极简设计** | API 简洁,易于理解 |
| **高性能** | 接近原生 API 性能 |

### 名字由来

**Sokol** (Сокол) 在俄语中意为"猎鹰"🦅,象征速度和敏捷。

---

## 🎯 为什么 Soluna 选择 Sokol?

### 云风的选择理由(来自 Discussion)

> "sokol 只有几个 .h 文件,集成起来很方便,迭代也快。对于 2D 需求,sokol 的着色器管线更简洁。"

### Sokol vs BGFX 对比

| 特性 | Sokol | BGFX |
|------|-------|------|
| **文件数量** | 少(~10个.h) | 多(完整库) |
| **集成难度** | 极简单 | 中等 |
| **学习曲线** | 平缓 | 陡峭 |
| **2D 友好度** | 高 | 中等 |
| **多线程渲染** | 单线程 | 支持encoder |
| **社区规模** | 中等 | 较大 |

**Soluna 的权衡**:
- ✅ 集成速度快,迭代效率高
- ⚠️ 缺少多线程渲染 API(但可以通过封装解决)

---

## 📦 Sokol 模块组成

Sokol 由多个独立模块组成,每个模块负责特定功能:

### 核心图形模块

| 模块 | 文件 | 功能 |
|------|------|------|
| **sokol_gfx** | `sokol_gfx.h` | 核心图形 API(缓冲区/着色器/管线) |
| **sokol_app** | `sokol_app.h` | 窗口/事件/输入管理 |
| **sokol_glue** | `sokol_glue.h` | gfx 和 app 的粘合层 |

### 实用工具模块

| 模块 | 文件 | 功能 |
|------|------|------|
| **sokol_time** | `sokol_time.h` | 高精度计时器 |
| **sokol_audio** | `sokol_audio.h` | 跨平台音频 |
| **sokol_fetch** | `sokol_fetch.h` | 异步资源加载 |
| **sokol_gl** | `sokol_gl.h` | OpenGL 兼容层(immediate mode) |
| **sokol_debugtext** | `sokol_debugtext.h` | 调试文本渲染 |
| **sokol_shape** | `sokol_shape.h` | 基础几何图形生成 |

### 扩展模块

| 模块 | 功能 |
|------|------|
| **sokol_imgui** | Dear ImGui 集成 |
| **sokol_spine** | Spine 骨骼动画支持 |
| **sokol_fontstash** | 字体渲染 |

---

## 🚀 Sokol 快速上手

### 1. Hello Triangle(最小示例)

```c
#define SOKOL_IMPL
#define SOKOL_GLCORE33  // 或 SOKOL_D3D11, SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"

static sg_pass_action pass_action;

void init(void) {
    // 初始化图形 API
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });

    // 设置清屏颜色(黑色)
    pass_action = (sg_pass_action){
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f} }
    };
}

void frame(void) {
    // 开始渲染
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

    // TODO: 在这里绘制三角形

    // 结束渲染
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .window_title = "Hello Sokol",
    };
}
```

### 2. 完整三角形渲染

```c
// 顶点结构
typedef struct {
    float pos[2];     // 位置
    float color[3];   // 颜色
} vertex_t;

// 顶点数据
static const vertex_t vertices[] = {
    { { 0.0f,  0.5f}, {1.0f, 0.0f, 0.0f} },  // 顶部(红色)
    { { 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} },  // 右下(绿色)
    { {-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f} }   // 左下(蓝色)
};

// 顶点着色器(GLSL)
const char* vs_source =
    "#version 330\n"
    "layout(location=0) in vec2 pos;\n"
    "layout(location=1) in vec3 color;\n"
    "out vec3 frag_color;\n"
    "void main() {\n"
    "  gl_Position = vec4(pos, 0.0, 1.0);\n"
    "  frag_color = color;\n"
    "}\n";

// 片段着色器(GLSL)
const char* fs_source =
    "#version 330\n"
    "in vec3 frag_color;\n"
    "out vec4 out_color;\n"
    "void main() {\n"
    "  out_color = vec4(frag_color, 1.0);\n"
    "}\n";

static sg_pipeline pip;
static sg_bindings bind;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });

    // 创建顶点缓冲区
    bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices)
    });

    // 创建着色器
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.source = vs_source,
        .fs.source = fs_source,
    });

    // 创建渲染管线
    pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT2,  // pos
                [1].format = SG_VERTEXFORMAT_FLOAT3   // color
            }
        }
    });

    pass_action = (sg_pass_action){
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.1f, 0.1f, 0.1f, 1.0f} }
    };
}

void frame(void) {
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

    // 绑定管线和顶点缓冲区
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);

    // 绘制三角形
    sg_draw(0, 3, 1);

    sg_end_pass();
    sg_commit();
}
```

---

## 🎨 核心概念详解

### 1. 资源对象

Sokol 使用句柄(handle)管理资源:

```c
// 缓冲区(Buffer)
sg_buffer vbuf = sg_make_buffer(&desc);

// 图像(Texture)
sg_image img = sg_make_image(&desc);

// 着色器(Shader)
sg_shader shd = sg_make_shader(&desc);

// 渲染管线(Pipeline)
sg_pipeline pip = sg_make_pipeline(&desc);

// 释放资源
sg_destroy_buffer(vbuf);
sg_destroy_image(img);
sg_destroy_shader(shd);
sg_destroy_pipeline(pip);
```

### 2. 渲染流程

```
每帧渲染:
  1. sg_begin_pass()      开始渲染通道
  2. sg_apply_pipeline()  应用管线(着色器+状态)
  3. sg_apply_bindings()  绑定资源(顶点缓冲区+纹理)
  4. sg_apply_uniforms()  更新 uniform 数据(可选)
  5. sg_draw()            绘制调用
  6. sg_end_pass()        结束渲染通道
  7. sg_commit()          提交到 GPU
```

### 3. 缓冲区类型

```c
// 顶点缓冲区(Vertex Buffer)
sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
    .type = SG_BUFFERTYPE_VERTEXBUFFER,
    .data = SG_RANGE(vertices),
    .usage = SG_USAGE_IMMUTABLE  // 不可变(静态几何)
});

// 索引缓冲区(Index Buffer)
sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
    .type = SG_BUFFERTYPE_INDEXBUFFER,
    .data = SG_RANGE(indices)
});

// Uniform 缓冲区(Constant Buffer)
sg_buffer ubuf = sg_make_buffer(&(sg_buffer_desc){
    .type = SG_BUFFERTYPE_STORAGEBUFFER,
    .usage = SG_USAGE_STREAM  // 每帧更新
});
```

### 4. 纹理(Image)

```c
// 创建纹理
sg_image img = sg_make_image(&(sg_image_desc){
    .width = 256,
    .height = 256,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .data.subimage[0][0] = {
        .ptr = pixels,
        .size = 256 * 256 * 4
    }
});

// 在着色器中使用
bind.fs.images[0] = img;
bind.fs.samplers[0] = smp;
sg_apply_bindings(&bind);
```

### 5. 着色器 Uniform

```c
// C 结构(必须对齐)
typedef struct {
    float mvp[16];  // 模型-视图-投影矩阵
} vs_params_t;

// 着色器中声明
const char* vs_src =
    "#version 330\n"
    "uniform mat4 mvp;\n"
    "layout(location=0) in vec3 pos;\n"
    "void main() {\n"
    "  gl_Position = mvp * vec4(pos, 1.0);\n"
    "}\n";

// 运行时更新
vs_params_t vs_params = {
    .mvp = { /* 矩阵数据 */ }
};
sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(vs_params));
```

---

## 🔧 Soluna 中的 Sokol 应用

### 窗口创建(sokol_app)

```c
// Soluna 使用 sokol_app 管理窗口和事件
sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc){
        .init_cb = soluna_init,
        .frame_cb = soluna_frame,
        .event_cb = soluna_event,    // 输入事件
        .cleanup_cb = soluna_cleanup,
        .width = 1280,
        .height = 720,
        .window_title = "Soluna",
        .high_dpi = true,            // 支持高DPI
        .sample_count = 4,           // MSAA 抗锯齿
    };
}
```

### 2D 渲染管线

```c
// Soluna 的着色器设计(简化版)
struct vs_input {
    float2 pos;      // 顶点位置
    short2 uv;       // 纹理坐标
    short  sr_idx;   // 变换矩阵索引
};

// 顶点着色器
vec4 vs_main(vs_input in) {
    // 从 uniform buffer 获取变换矩阵
    mat2 sr_matrix = u_sr_matrices[in.sr_idx];

    // 应用变换
    vec2 transformed = sr_matrix * in.pos;

    return vec4(transformed, 0.0, 1.0);
}
```

### 纹理图集绑定

```c
// Soluna 运行时打包纹理图集后
sg_image atlas_img = sg_make_image(&(sg_image_desc){
    .width = 2048,
    .height = 2048,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .data = /* 打包后的图像数据 */
});

// 绑定到着色器
bind.fs.images[0] = atlas_img;
sg_apply_bindings(&bind);
```

---

## 🌐 跨平台支持

### 后端选择

编译时通过宏选择图形后端:

| 平台 | 宏定义 | 说明 |
|------|--------|------|
| Windows | `SOKOL_D3D11` | DirectX 11(推荐) |
| Windows | `SOKOL_GLCORE33` | OpenGL 3.3 |
| macOS | `SOKOL_METAL` | Metal(推荐) |
| macOS | `SOKOL_GLCORE33` | OpenGL 3.3(已弃用) |
| Linux | `SOKOL_GLCORE33` | OpenGL 3.3 |
| Web | `SOKOL_GLES3` | WebGL 2.0 |

### 编译示例

```bash
# Windows (D3D11)
cl /DSOKOL_D3D11 main.c

# macOS (Metal)
clang -DSOKOL_METAL main.c -framework Metal -framework MetalKit

# Linux (OpenGL)
gcc -DSOKOL_GLCORE33 main.c -lGL -lX11 -ldl -lpthread

# WebAssembly
emcc -DSOKOL_GLES3 main.c -o game.html
```

---

## 📊 性能特点

### 优势

1. **零开销抽象**
   - 直接映射到原生 API
   - 没有额外的虚函数调用
   - 编译时优化友好

2. **批量渲染优化**
   - 支持实例化渲染(Instancing)
   - 减少 draw call 数量

3. **高效内存管理**
   - 资源池(Resource Pool)设计
   - 避免频繁的堆分配

### 限制

1. **单线程渲染**
   - 不支持多线程提交命令
   - Soluna 通过 Batch 系统解决

2. **固定管线模型**
   - 不如 Vulkan/DX12 灵活
   - 但对 2D 游戏足够

---

## 🎓 学习资源

### 官方资源

- **GitHub**: https://github.com/floooh/sokol
- **示例集合**: https://floooh.github.io/sokol-html5/
- **文档**: https://github.com/floooh/sokol/tree/master/doc

### 示例项目

| 项目 | 说明 |
|------|------|
| **sokol-samples** | 官方示例 |
| **sokol-tools** | 着色器编译工具 |
| **Chips Emulator** | 基于 Sokol 的游戏机模拟器 |
| **Soluna** | 本项目,2D 游戏引擎 |

---

## 🔍 Sokol vs 其他图形库

| 特性 | Sokol | SDL2 | GLFW | bgfx |
|------|-------|------|------|------|
| **图形抽象** | 高级 | 低级 | 无 | 高级 |
| **窗口管理** | ✅ | ✅ | ✅ | ❌ |
| **音频** | ✅ | ✅ | ❌ | ❌ |
| **输入** | ✅ | ✅ | ✅ | ❌ |
| **集成难度** | 极简单 | 简单 | 简单 | 中等 |
| **文件数量** | 少 | 中等 | 少 | 多 |
| **多后端** | ✅ | 部分 | ❌ | ✅ |
| **WebGL** | ✅ | ✅ | ❌ | ✅ |

---

## 💡 Soluna 使用 Sokol 的最佳实践

### 1. 着色器管理

```c
// 使用 sokol-shdc 工具离线编译着色器
// 生成 .h 文件,包含所有后端的字节码

#include "shaders/sprite.glsl.h"

sg_shader shd = sg_make_shader(sprite_shader_desc(sg_query_backend()));
```

### 2. 资源加载

```c
// 使用 sokol_fetch 异步加载图片
sfetch_send(&(sfetch_request_t){
    .path = "textures/player.png",
    .callback = image_loaded_callback,
    .buffer = /* 缓冲区 */
});
```

### 3. 调试工具

```c
// 使用 sokol_debugtext 显示 FPS
sdtx_print("FPS: 60");

// 使用 sokol_imgui 构建调试 UI
ImGui::Text("Draw Calls: %d", draw_calls);
```

---

## 🎯 总结

### Sokol 的核心价值

1. **极简集成** - 复制几个 .h 文件即可
2. **跨平台无痛** - 一套代码,多个平台
3. **性能接近原生** - 零开销抽象
4. **API 优雅** - 容易学习和使用

### 为什么适合 Soluna?

✅ 轻量级,符合极简哲学
✅ 快速迭代,集成方便
✅ 2D 渲染需求完美契合
✅ WebAssembly 支持良好

---

## 下一步学习

- 📄 [ltask 多线程框架](./03-ltask多线程框架.md) - 了解 Lua 多线程
- 📄 [2D 渲染管线架构](./04-2D渲染管线架构.md) - Soluna 如何使用 Sokol
- 📄 [WebAssembly 导出](./12-WebAssembly导出.md) - 将游戏编译到 Web
