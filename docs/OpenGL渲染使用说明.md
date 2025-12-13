# OpenGL渲染系统使用说明

## 功能概述

已成功实现基于Qt OpenGL的游戏渲染系统！现在可以看到华丽的水果纹理和流畅的交互效果。

## 核心特性

### 1. **OpenGL硬件加速渲染**
- 使用QOpenGLWidget实现高性能渲染
- 支持纹理映射和透明混合
- 2D正交投影，适合消消乐游戏
- 60 FPS流畅帧率

### 2. **水果纹理系统**
已加载6种水果高清纹理：
- 🍎 苹果 (apple.png)
- 🍊 橙子 (orange.png)
- 🍇 葡萄 (grape.png)
- 🍌 香蕉 (banana.png)
- 🍉 西瓜 (watermelon.png)
- 🍓 草莓 (strawberry.png)

### 3. **交互功能**
- **鼠标点击选择**：点击任意水果进行选中
- **脉冲动画**：选中的水果显示黄色闪烁边框
- **点击交换**：选中后点击相邻水果尝试交换
- **自动消除**：交换成功后自动触发连锁消除

### 4. **视觉效果**
- **网格背景**：深蓝色主题，专业游戏界面
- **单元格背景**：每个水果都有独立的深灰色背景
- **特殊元素标记**：
  - 金色边框 = 直线炸弹（LINE_H/LINE_V）
  - 青色边框 = 菱形炸弹（DIAMOND）
  - 紫色边框 = 万能炸弹（RAINBOW）
- **选中效果**：黄色半透明叠加 + 脉冲边框

## 使用方法

### 启动游戏

```bash
cd d:\codeproject\DataSructure\06-General\Game_Qoder_2
.\build\bin\FruitCrush.exe
```

### 操作流程

1. **进入游戏**
   - 在主菜单点击"休闲模式"按钮
   - 自动加载OpenGL游戏视图
   - 显示8×8水果网格

2. **选择水果**
   - 用鼠标点击任意水果
   - 选中的水果会显示黄色闪烁边框
   - 再次点击同一个水果可取消选中

3. **交换水果**
   - 先点击一个水果（选中）
   - 再点击相邻的水果（尝试交换）
   - 如果能产生匹配，交换成功并自动消除
   - 如果不能匹配，交换失败，选中状态清除

4. **观察效果**
   - 交换成功后会自动触发消除
   - 水果会下落填充空位
   - 新水果会从顶部生成
   - 可能触发连锁消除
   - 观察分数和连击数变化

5. **返回主菜单**
   - 点击底部"返回主菜单"按钮
   - 返回主界面

## 技术实现

### GameView 类 (ui/views/GameView.h/cpp)

#### 核心方法说明

**1. 初始化相关**
```cpp
void initializeGL() override;
```
- 初始化OpenGL上下文
- 设置背景颜色和混合模式
- 加载所有水果纹理

**2. 渲染相关**
```cpp
void paintGL() override;
```
- 清空屏幕缓冲
- 设置2D正交投影
- 绘制网格背景
- 绘制所有水果
- 绘制选中框效果

**3. 纹理加载**
```cpp
void loadTextures();
```
- 从resources/textures目录加载PNG图片
- 转换为OpenGL兼容格式
- 设置纹理过滤和环绕模式
- 存储到fruitTextures_数组

**4. 单个水果绘制**
```cpp
void drawFruit(int row, int col, const Fruit& fruit);
```
- 计算屏幕坐标
- 绘制单元格背景
- 绑定对应纹理
- 使用GL_QUADS绘制四边形
- 添加特殊元素边框高亮

**5. 鼠标交互**
```cpp
void mousePressEvent(QMouseEvent *event) override;
```
- 将屏幕坐标转换为网格坐标
- 处理选中/取消选中逻辑
- 调用gameEngine_->swapFruits()执行交换
- 更新显示

**6. 动画更新**
```cpp
void onAnimationTimer();
```
- 每16ms触发一次（60 FPS）
- 更新动画帧计数
- 触发选中框脉冲效果重绘

### 布局参数

```cpp
float gridStartX_;   // 网格起始X坐标（自动居中）
float gridStartY_;   // 网格起始Y坐标（自动居中）
float cellSize_;     // 单元格大小（根据窗口大小自适应）
```

### 坐标转换

```cpp
bool screenToGrid(int x, int y, int& row, int& col);
```
- 输入：屏幕像素坐标
- 输出：网格行列索引（0-7）
- 返回：是否在有效范围内

## 渲染流程

1. **窗口初始化**
   ```
   initializeGL() 
   → 加载纹理 
   → 设置OpenGL状态
   ```

2. **窗口缩放**
   ```
   resizeGL() 
   → 计算网格布局 
   → 更新cellSize_
   ```

3. **每帧渲染**
   ```
   paintGL()
   → 清空屏幕
   → 绘制背景网格
   → 遍历8×8地图
   → 绘制每个水果纹理
   → 绘制选中框
   ```

4. **动画循环**
   ```
   animationTimer_(16ms)
   → animationFrame_++
   → 更新脉冲效果
   → 触发重绘
   ```

## 性能优化

- ✅ 使用QOpenGLTexture缓存纹理，避免重复加载
- ✅ 仅在有选中时才进行动画重绘
- ✅ 使用硬件加速的OpenGL渲染
- ✅ 2D正交投影，避免复杂的3D计算
- ✅ 纹理过滤使用Linear模式，确保缩放平滑

## 颜色方案

| 元素 | 颜色 | 说明 |
|------|------|------|
| 背景 | RGB(0.1, 0.15, 0.25) | 深蓝色 |
| 网格背景 | RGB(0.2, 0.25, 0.35) | 中蓝色 |
| 单元格背景 | RGB(0.3, 0.35, 0.45) | 浅蓝灰色 |
| 选中填充 | RGBA(1.0, 1.0, 0.0, 0.3-0.5) | 半透明黄色（脉冲） |
| 选中边框 | RGBA(1.0, 1.0, 0.0, 0.8-1.0) | 黄色（脉冲） |
| 直线炸弹边框 | RGBA(1.0, 0.8, 0.0, 0.8) | 金色 |
| 菱形炸弹边框 | RGBA(0.0, 0.8, 1.0, 0.8) | 青色 |
| 万能炸弹边框 | RGBA(1.0, 0.0, 1.0, 0.8) | 紫色 |

## 调试信息

运行时会在控制台输出以下调试信息：

```
GameView created
OpenGL initialized
Loaded texture: resources/textures/apple.png
Loaded texture: resources/textures/orange.png
...
Resized: 800 x 600 Cell size: 60
Selected: 2, 3
Trying to swap (2, 3) with (2, 4)
Swap successful! Score: 30
```

## 后续扩展方向

- [ ] 添加交换动画（平滑移动）
- [ ] 添加消除动画（渐隐/缩放）
- [ ] 添加下落动画（重力效果）
- [ ] 添加粒子特效（爆炸/星星）
- [ ] 添加分数飘字动画
- [ ] 添加背景音乐和音效
- [ ] 优化触摸/拖拽交互
- [ ] 添加Shader特效

## 常见问题

### Q: 纹理无法加载？
A: 确保resources/textures目录下有对应的PNG文件，路径相对于可执行文件。

### Q: 窗口显示黑屏？
A: 检查OpenGL驱动是否支持，查看控制台是否有错误信息。

### Q: 选中没有反应？
A: 确保gameEngine_已正确设置，检查坐标转换是否正确。

### Q: 交换总是失败？
A: 只有相邻的水果才能交换，且交换后必须能产生至少3个连续匹配。

## 文件结构

```
ui/views/
├── GameView.h          # OpenGL视图头文件
└── GameView.cpp        # OpenGL视图实现

resources/textures/
├── apple.png          # 苹果纹理
├── orange.png         # 橙子纹理
├── grape.png          # 葡萄纹理
├── banana.png         # 香蕉纹理
├── watermelon.png     # 西瓜纹理
└── strawberry.png     # 草莓纹理
```

## 总结

现在已经实现了一个功能完整的OpenGL渲染系统，包括：
- ✅ 高性能纹理渲染
- ✅ 鼠标点击交互
- ✅ 选中动画效果
- ✅ 特殊元素标记
- ✅ 自适应布局

可以开始进行实际游戏测试，体验流畅的图形效果和交互体验！
