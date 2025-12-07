# ltask 多线程框架详解

## 📚 什么是 ltask?

**ltask** 是云风(cloudwu)开发的 **Lua 多线程任务调度框架**,专为游戏服务器和高并发场景设计。

### 核心问题

**Lua 本身是单线程的**:
```
┌──────────────┐
│  Lua VM 1    │  ← 独立虚拟机,不能共享数据
├──────────────┤
│  Lua VM 2    │  ← 独立虚拟机,不能共享数据
├──────────────┤
│  Lua VM 3    │  ← 独立虚拟机,不能共享数据
└──────────────┘

问题:如何让多个 Lua VM 协作?
```

**ltask 的解决方案**:
- ✅ 创建多个独立的 Lua 虚拟机
- ✅ 通过消息传递(Message Passing)通信
- ✅ 避免共享内存,消除锁竞争
- ✅ Actor 模型架构

---

## 🎯 为什么 Soluna 选择 ltask?

### 云风的理由(来自 Discussion)

> "ltask 提供了方便的 Lua 多线程支持"

###游戏中的多线程需求

```
游戏架构:

┌─────────────────────────────────┐
│  逻辑线程(Lua)                   │
│  • 游戏逻辑更新                  │
│  • AI 计算                       │
│  • 碰撞检测                      │
└─────────────────────────────────┘
         ↓ 消息
┌─────────────────────────────────┐
│  渲染线程(Lua + Sokol)           │
│  • 收集渲染批次                  │
│  • 提交到 GPU                    │
└─────────────────────────────────┘
```

**优势**:
- 逻辑和渲染并行,充分利用多核 CPU
- 避免逻辑卡顿影响渲染帧率

---

## 🏗️ Actor 模型架构

### 什么是 Actor 模型?

```
Actor = 独立的执行单元

┌──────────────┐
│   Actor A    │
│  ┌────────┐  │
│  │ Mailbox│  │  ← 消息队列
│  └────────┘  │
│  ┌────────┐  │
│  │  State │  │  ← 私有状态
│  └────────┘  │
│  ┌────────┐  │
│  │ Lua VM │  │  ← 独立虚拟机
│  └────────┘  │
└──────────────┘
      ↓ 发送消息
┌──────────────┐
│   Actor B    │
│  ┌────────┐  │
│  │ Mailbox│←─┘
│  └────────┘  │
└──────────────┘
```

**核心原则**:
1. 每个 Actor 有独立的状态(不共享)
2. Actor 只通过消息通信
3. Actor 异步处理消息
4. 消息传递是线程安全的

---

## 🚀 ltask 快速上手

### 1. 创建 Service(Actor)

```lua
-- hello_service.lua
local ltask = require "ltask"

-- 定义服务
local SERVICE = {}

-- 初始化函数
function SERVICE.init()
    print("Hello Service 启动!")
end

-- 处理消息
function SERVICE.hello(name)
    return "Hello, " .. name .. "!"
end

return ltask.dispatch(SERVICE)
```

### 2. 启动和调用 Service

```lua
-- main.lua
local ltask = require "ltask"

-- 启动 hello_service
local hello_addr = ltask.spawn("hello_service")

-- 调用远程服务(阻塞调用)
local result = ltask.call(hello_addr, "hello", "World")
print(result)  -- 输出: Hello, World!

-- 发送消息(非阻塞)
ltask.send(hello_addr, "hello", "Async")
```

### 3. 完整示例:计数器服务

```lua
-- counter_service.lua
local ltask = require "ltask"

local SERVICE = {}
local count = 0  -- 私有状态

function SERVICE.init()
    print("计数器服务启动,初始值:", count)
end

function SERVICE.increment()
    count = count + 1
    return count
end

function SERVICE.get()
    return count
end

function SERVICE.reset()
    count = 0
    return "计数器已重置"
end

return ltask.dispatch(SERVICE)
```

```lua
-- main.lua
local ltask = require "ltask"

local counter = ltask.spawn("counter_service")

-- 调用方法
print(ltask.call(counter, "increment"))  -- 1
print(ltask.call(counter, "increment"))  -- 2
print(ltask.call(counter, "increment"))  -- 3
print(ltask.call(counter, "get"))        -- 3
print(ltask.call(counter, "reset"))      -- 计数器已重置
print(ltask.call(counter, "get"))        -- 0
```

---

## 🔧 核心 API 详解

### 1. 启动服务

```lua
-- 启动独占服务(Exclusive Service)
local addr = ltask.spawn("service_name", arg1, arg2)

-- 启动共享服务(Shared Service,可被多个线程调用)
local addr = ltask.spawn_shared("service_name")

-- 获取当前服务地址
local self_addr = ltask.self()
```

### 2. 消息发送

```lua
-- 阻塞调用(等待返回值)
local result = ltask.call(addr, "method", arg1, arg2)

-- 非阻塞发送(不等待返回)
ltask.send(addr, "method", arg1, arg2)

-- 广播消息(发送给多个服务)
ltask.multi_call({addr1, addr2, addr3}, "method", arg)
```

### 3. 消息接收

```lua
-- 等待并处理一条消息
local msg = ltask.wait()

-- 处理所有消息(直到队列为空)
while ltask.has_message() do
    local msg = ltask.wait()
    -- 处理消息
end
```

### 4. 定时器

```lua
-- 延迟执行(单位:毫秒)
ltask.sleep(1000)  -- 睡眠 1 秒

-- 设置超时回调
ltask.timeout(5000, function()
    print("5 秒后执行")
end)
```

---

## 🎮 Soluna 中的 ltask 应用

### 逻辑线程 + 渲染线程架构

```lua
-- render_service.lua (渲染服务)
local ltask = require "ltask"
local sokol = require "sokol"

local SERVICE = {}
local batch_queue = {}  -- 待渲染的批次队列

function SERVICE.init()
    sokol.init_graphics()
end

-- 接收渲染批次
function SERVICE.submit_batch(batch_data)
    table.insert(batch_queue, batch_data)
end

-- 每帧渲染
function SERVICE.render_frame()
    sokol.begin_frame()

    -- 渲染所有批次
    for _, batch in ipairs(batch_queue) do
        sokol.draw_batch(batch)
    end

    batch_queue = {}  -- 清空队列
    sokol.end_frame()
end

return ltask.dispatch(SERVICE)
```

```lua
-- game_logic_service.lua (游戏逻辑服务)
local ltask = require "ltask"

local SERVICE = {}
local render_addr  -- 渲染服务地址

function SERVICE.init(render_service)
    render_addr = render_service
end

function SERVICE.update(dt)
    -- 更新游戏逻辑
    local player_pos = update_player(dt)
    local enemies = update_enemies(dt)

    -- 生成渲染数据
    local batch = create_render_batch(player_pos, enemies)

    -- 发送到渲染线程(非阻塞)
    ltask.send(render_addr, "submit_batch", batch)
end

return ltask.dispatch(SERVICE)
```

```lua
-- main.lua
local ltask = require "ltask"

-- 启动渲染服务
local render_addr = ltask.spawn("render_service")

-- 启动游戏逻辑服务
local logic_addr = ltask.spawn("game_logic_service", render_addr)

-- 主循环
while true do
    -- 更新逻辑(每帧)
    ltask.call(logic_addr, "update", 0.016)

    -- 渲染(每帧)
    ltask.call(render_addr, "render_frame")
end
```

---

## 🔍 消息传递机制

### 消息序列化

ltask 自动序列化 Lua 数据:

```lua
-- 支持的数据类型
ltask.send(addr, "method",
    42,                    -- number
    "hello",               -- string
    true,                  -- boolean
    {1, 2, 3},            -- table(数组)
    {x=10, y=20}          -- table(字典)
)

-- 不支持的类型
ltask.send(addr, "method",
    function() end,        -- ❌ 函数
    coroutine.create(),    -- ❌ 协程
    io.open("file")        -- ❌ userdata
)
```

### 零拷贝优化

对于大数据,使用 `lightuserdata`:

```lua
-- 发送大缓冲区(零拷贝)
local buffer = create_large_buffer()  -- C 分配的内存
ltask.send(addr, "process", ltask.pack_lightuserdata(buffer))
```

---

## ⚡ 性能优化

### 1. 批量消息处理

```lua
-- ❌ 差:每条消息一次调用
for i = 1, 1000 do
    ltask.send(addr, "process", i)
end

-- ✅ 好:打包批量发送
local batch = {}
for i = 1, 1000 do
    table.insert(batch, i)
end
ltask.send(addr, "process_batch", batch)
```

### 2. 避免频繁跨线程调用

```lua
-- ❌ 差:每帧调用
function update()
    local data = ltask.call(other_service, "get_data")  -- 阻塞!
    process(data)
end

-- ✅ 好:缓存或异步
local cached_data
function update()
    if not cached_data then
        ltask.send(other_service, "get_data")
    end
    process(cached_data)
end
```

### 3. 使用共享服务

```lua
-- 只读配置服务适合共享
local config_addr = ltask.spawn_shared("config_service")

-- 多个线程可以并发调用
ltask.call(config_addr, "get", "player_speed")
```

---

## 🆚 ltask vs 其他并发方案

| 方案 | 模型 | 共享内存 | 复杂度 | 性能 |
|------|------|---------|--------|------|
| **ltask** | Actor | ❌ | 低 | 高 |
| **lanes** | 线程 | ✅ | 中等 | 高 |
| **coroutine** | 协程 | ✅ | 低 | 中等 |
| **multithread(C)** | 线程 | ✅ | 高 | 最高 |

### lanes vs ltask

```lua
-- lanes:共享内存,需要锁
local lanes = require "lanes"
local linda = lanes.linda()

-- 写数据(需要加锁)
linda:set("key", value)

-- 读数据(需要加锁)
local value = linda:get("key")
```

```lua
-- ltask:消息传递,无锁
ltask.send(addr, "set", "key", value)
local value = ltask.call(addr, "get", "key")
```

**ltask 优势**:
- ✅ 无锁设计,避免死锁
- ✅ 隔离性好,错误不会传播
- ✅ 易于理解和调试

---

## 🛠️ 调试技巧

### 1. 打印服务地址

```lua
local addr = ltask.spawn("service")
print("服务地址:", ltask.address_to_string(addr))
```

### 2. 监控消息队列

```lua
-- 检查消息队列长度
if ltask.queue_length() > 100 then
    print("警告:消息队列过长!")
end
```

### 3. 超时检测

```lua
-- 设置调用超时
local ok, result = pcall(function()
    return ltask.call(addr, "slow_method")
end)

if not ok then
    print("调用超时或出错:", result)
end
```

---

## 📚 实战示例:多线程游戏服务器

### 场景:MMO 游戏

```lua
-- player_service.lua (每个玩家一个 Actor)
local SERVICE = {}
local player_data = {}

function SERVICE.init(player_id)
    player_data.id = player_id
    player_data.pos = {x=0, y=0}
    player_data.hp = 100
end

function SERVICE.move(dx, dy)
    player_data.pos.x = player_data.pos.x + dx
    player_data.pos.y = player_data.pos.y + dy
    return player_data.pos
end

function SERVICE.attack(target_addr, damage)
    ltask.send(target_addr, "take_damage", damage)
end

function SERVICE.take_damage(damage)
    player_data.hp = player_data.hp - damage
    if player_data.hp <= 0 then
        print("玩家", player_data.id, "死亡")
    end
end

return ltask.dispatch(SERVICE)
```

```lua
-- world_service.lua (世界管理)
local SERVICE = {}
local players = {}  -- player_id => player_addr

function SERVICE.player_login(player_id)
    local addr = ltask.spawn("player_service", player_id)
    players[player_id] = addr
    return addr
end

function SERVICE.broadcast(message)
    for id, addr in pairs(players) do
        ltask.send(addr, "notify", message)
    end
end

return ltask.dispatch(SERVICE)
```

---

## 🎯 总结

### ltask 的核心价值

1. **无锁并发** - Actor 模型消除锁竞争
2. **简单易用** - API 简洁,学习曲线平缓
3. **高性能** - 接近原生线程性能
4. **容错性好** - 服务隔离,错误不传播

### 适合使用 ltask 的场景

✅ 游戏服务器(大量独立玩家)
✅ 多线程渲染(逻辑 + 渲染分离)
✅ 高并发任务处理
✅ 分布式系统原型

### 不适合的场景

❌ 大量共享状态的程序
❌ 对延迟极其敏感的场景
❌ 简单的单线程应用

---

## 🔗 相关资源

- **ltask GitHub**: https://github.com/cloudwu/ltask
- **云风 Blog**: https://blog.codingnow.com/
- **skynet**: ltask 的前身,服务器框架

---

## 下一步学习

- 📄 [2D 渲染管线架构](./04-2D渲染管线架构.md) - ltask 在渲染中的应用
- 📄 [Soluna 项目架构总览](./09-Soluna项目架构总览.md) - 完整架构设计
