// ============================================
// rect.js - Raylib 动画示例(通过 crepl 管道)
// ============================================
// 用法: node rect.js | ./crepl libraylib.so
//
// 工作原理:
//   1. Node.js 生成 crepl 命令并输出到 stdout
//   2. 通过管道(|)传递给 crepl 的 stdin
//   3. crepl 逐行读取并执行函数调用
//
// 等价于手动在 crepl 中逐行输入:
//   > InitWindow 800 600 "FFI"
//   > SetTargetFPS 60
//   > BeginDrawing
//   > ClearBackground 0xFF000000
//   > DrawRectangle 1 1 100 100 0xFF0000FF
//   > EndDrawing
//   > BeginDrawing
//   > ...
// ============================================

// ========== 初始化窗口 ==========
// InitWindow(width, height, title)
console.log("InitWindow 800 600 \"FFI\"");

// ========== 设置帧率 ==========
// SetTargetFPS(fps) - 限制为 60 FPS
console.log("SetTargetFPS 60");

// ========== 矩形位置变量 ==========
let x = 0  // 矩形左上角 X 坐标
let y = 0  // 矩形左上角 Y 坐标

// ========== 主循环(无限) ==========
while (true) {
    // 每帧移动矩形(对角线移动)
    x += 1
    y += 1

    // ========== 开始绘制 ==========
    console.log("BeginDrawing");

    // ========== 清空背景 ==========
    // ClearBackground(color) - 0xFF000000 = 黑色(RGBA)
    //   格式: 0xRRGGBBAA
    //   0xFF = 255(不透明), 0x00 = 0(全透明)
    console.log("ClearBackground 0xFF000000");

    // ========== 绘制矩形 ==========
    // DrawRectangle(x, y, width, height, color)
    //   x, y: 左上角坐标(动态变化)
    //   width, height: 100x100 像素
    //   color: 0xFF0000FF = 红色(RGBA)
    //     R=0xFF(255, 红色)
    //     G=0x00(0,   无绿)
    //     B=0x00(0,   无蓝)
    //     A=0xFF(255, 不透明)
    console.log(`DrawRectangle ${x} ${y} 100 100 0xFF0000FF`);

    // ========== 结束绘制 ==========
    console.log("EndDrawing");
}

// ============================================
// 执行效果:
// ============================================
// 1. 创建 800x600 窗口
// 2. 每帧:
//    - 清空为黑色
//    - 绘制红色矩形(从左上角向右下角移动)
//    - 速度: 每帧移动 1 像素
//
// 矩形轨迹:
//   帧 1: (1,   1)
//   帧 2: (2,   2)
//   帧 3: (3,   3)
//   ...
//   帧 800: (800, 800) → 超出窗口右下角
// ============================================
