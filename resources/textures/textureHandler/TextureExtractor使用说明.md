# 材质提取工具使用说明

## 功能概述

`TextureExtractor` 是一个用于从 4x3 网格材质图中提取独立元素的工具类。它能够：

1. **自动检测边界** - 分析透明背景，自动找到每个元素的最小包围框
2. **计算中心点** - 精确计算每个元素的中心位置
3. **统一尺寸** - 生成统一大小的材质（使用最大元素的尺寸）
4. **保持透明度** - 完整保留 PNG 透明通道

## 代码结构

```
src/utils/
├── TextureExtractor.h          # 头文件
├── TextureExtractor.cpp        # 实现文件
└── TextureExtractorTool.cpp    # 使用示例/测试程序
```

## 核心功能讲解

### 1. 加载图像 - `loadImage()`

```cpp
bool loadImage(const QString& imagePath)
```

**功能**：加载 4x3 材质图
**步骤**：

- 使用 QImage 加载图像文件
- 检查图像是否有效
- 转换为 ARGB32 格式（确保支持透明通道）

---

### 2. 提取元素 - `extractElements()`

```cpp
bool extractElements(int rows = 3, int cols = 4)
```

**功能**：分析并提取所有 12 个元素
**步骤**：

1. 计算每个网格单元的尺寸（图像宽度/列数，图像高度/行数）
2. 遍历每个单元格（3 行 x 4 列 = 12 个单元）
3. 对每个单元调用 `detectBoundingBox()` 检测边界
4. 计算中心点
5. 调用 `calculateUnifiedSize()` 计算统一尺寸
6. 使用 `extractCenteredRegion()` 提取材质

---

### 3. 检测边界框 - `detectBoundingBox()`

```cpp
QRect detectBoundingBox(const QImage& cellImage, int alphaThreshold = 10)
```

**功能**：找到单元格中非透明像素的最小包围矩形
**步骤**：

1. 初始化边界坐标 (minX, minY, maxX, maxY)
2. 遍历单元格的每个像素
3. 检查像素的 Alpha 值（透明度）
4. 如果 Alpha > 阈值（默认 10），更新边界坐标
5. 返回最小包围矩形

**示例**：

```
原始单元格 (100x100):          检测到的边界框:
┌─────────────┐               ┌─────────────┐
│             │               │             │
│   ┌───┐     │               │   ▓▓▓▓▓     │ ← 边界框 (30, 20, 40x50)
│   │ ● │     │    ───→       │   ▓ ● ▓     │
│   └───┘     │               │   ▓▓▓▓▓     │
│             │               │             │
└─────────────┘               └─────────────┘
```

---

### 4. 计算统一尺寸 - `calculateUnifiedSize()`

```cpp
void calculateUnifiedSize()
```

**功能**：找到能包含所有元素的最小正方形尺寸
**步骤**：

1. 遍历所有元素的边界框
2. 找到最大宽度和最大高度
3. 取两者中的最大值作为正方形边长
4. 增加 10%边距（避免材质过于紧凑）

**示例**：

```
元素1: 40x30    │  最大宽度: 50
元素2: 50x45    │  最大高度: 45
元素3: 35x40    │  正方形边长: max(50, 45) = 50
...             │  添加边距: 50 * 1.1 = 55
统一尺寸: 55x55
```

---

### 5. 提取中心区域 - `extractCenteredRegion()`

```cpp
QImage extractCenteredRegion(const QImage& source, const QPoint& center, const QSize& size)
```

**功能**：以指定中心点提取指定尺寸的图像区域
**步骤**：

1. 创建透明背景的新图像（目标尺寸）
2. 计算源图像中的提取矩形（中心点 ± 半尺寸）
3. 与源图像边界求交集（处理边缘情况）
4. 逐像素复制到目标图像中心

**示例**：

```
源图像:                提取区域:           结果图像:
┌──────────┐          ┌──────────┐       ┌──────┐
│          │          │          │       │      │
│    ●─┐   │   ───→   │    ●─┐   │ ───→  │  ●─┐ │
│    └─┘   │          │    └─┘   │       │  └─┘ │
│          │          │          │       │      │
└──────────┘          └──────────┘       └──────┘
   中心点 ●              提取框              居中
```

---

### 6. 保存材质 - `saveTextures()`

```cpp
int saveTextures(const QString& outputDir, const QString& prefix = "fruit_")
```

**功能**：保存所有提取的材质到文件
**输出**：

- 文件格式：`<prefix><索引>.png`
- 例如：`fruit_0.png`, `fruit_1.png`, ... `fruit_11.png`

## 使用示例

### 方法 1：使用测试工具

```bash
# 编译
cd build
cmake --build .

# 运行（使用默认参数）
./TextureExtractorTool

# 运行（自定义参数）
./TextureExtractorTool <输入图片> <输出目录> <文件前缀>
```

### 方法 2：在代码中使用

```cpp
#include "utils/TextureExtractor.h"

// 创建提取器
TextureExtractor extractor;

// 加载材质图
if (!extractor.loadImage("resources/texture.png")) {
    // 处理错误
    return;
}

// 提取元素（4列 x 3行）
if (!extractor.extractElements(3, 4)) {
    // 处理错误
    return;
}

// 获取元素信息
const auto& elements = extractor.getElementInfos();
for (size_t i = 0; i < elements.size(); ++i) {
    qDebug() << "元素" << i << "中心点:" << elements[i].center;
}

// 保存材质
extractor.saveTextures("resources/textures", "fruit_");

// 或者直接获取 QImage 使用
const auto& textures = extractor.getTextures();
for (const auto& texture : textures) {
    // 使用 texture...
}
```

## 输出结果

运行后会生成 12 个材质文件：

```
resources/textures/
├── fruit_0.png    # 第1行第1列
├── fruit_1.png    # 第1行第2列
├── fruit_2.png    # 第1行第3列
├── fruit_3.png    # 第1行第4列
├── fruit_4.png    # 第2行第1列
├── fruit_5.png    # 第2行第2列
├── fruit_6.png    # 第2行第3列
├── fruit_7.png    # 第2行第4列
├── fruit_8.png    # 第3行第1列
├── fruit_9.png    # 第3行第2列
├── fruit_10.png   # 第3行第3列
└── fruit_11.png   # 第3行第4列
```

每个材质：

- ✓ 统一尺寸（正方形）
- ✓ 保留透明背景
- ✓ 元素居中
- ✓ 可直接用于游戏

## 函数文档总结

| 函数                      | 功能         | 输入                 | 输出             |
| ------------------------- | ------------ | -------------------- | ---------------- |
| `loadImage()`             | 加载材质图   | 图像路径             | bool (成功/失败) |
| `extractElements()`       | 提取所有元素 | 行数, 列数           | bool (成功/失败) |
| `detectBoundingBox()`     | 检测边界框   | 单元格图像           | 最小包围矩形     |
| `calculateCenter()`       | 计算中心点   | 矩形                 | 中心点坐标       |
| `calculateUnifiedSize()`  | 计算统一尺寸 | -                    | 设置统一尺寸     |
| `extractCenteredRegion()` | 提取中心区域 | 源图像, 中心点, 尺寸 | 提取的图像       |
| `saveTextures()`          | 保存材质     | 输出目录, 文件前缀   | 保存成功的数量   |

## 参数说明

- **alphaThreshold**: 透明度阈值（0-255），默认 10。Alpha 值大于此值的像素被视为非透明
- **rows**: 材质图的行数，默认 3
- **cols**: 材质图的列数，默认 4
- **prefix**: 输出文件名前缀，默认"fruit\_"

## 注意事项

1. 输入图像必须是 PNG 格式（支持透明通道）
2. 网格必须是规则的（等宽等高）
3. 输出目录会自动创建（如果不存在）
4. 生成的材质是正方形（方便游戏渲染）
