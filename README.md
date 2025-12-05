# 水果消消乐 (Fruit Crush)

基于Qt 6和OpenGL的水果消除游戏项目。

## 项目结构

```
FruitCrush/
├── src/                    # 源代码
│   ├── core/              # 核心游戏逻辑
│   ├── special/           # 特殊元素
│   ├── props/             # 道具系统
│   ├── score/             # 得分系统
│   ├── mode/              # 游戏模式
│   ├── achievement/       # 成就系统
│   ├── data/              # 数据管理
│   └── utils/             # 工具类
├── ui/                     # 用户界面
│   ├── views/             # 视图页面
│   └── widgets/           # 自定义控件
├── animation/              # 动画系统
│   └── effects/           # 特效
├── tests/                  # 测试文件
└── resources/              # 资源文件

```

## 构建要求

- CMake 3.16+
- Qt 6.x
- MinGW GCC 6.3+
- OpenGL 3.3+
- SQLite (Qt内置)

## 构建步骤

### 使用VSCode

1. 打开项目文件夹
2. 按 `Ctrl+Shift+P`，选择 `Tasks: Run Task` -> `CMake Configure`
3. 按 `Ctrl+Shift+B` 编译项目
4. 按 `F5` 启动调试

### 使用命令行

```bash
# 配置项目
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=E:/QT/6.9.1/mingw_64 -B build

# 编译
cmake --build build -- -j8

# 运行
./build/bin/FruitCrush.exe

# 运行测试
cd build
ctest --output-on-failure
```

## 开发文档

详见 [项目计划书_水果消消乐.md](./项目计划书_水果消消乐.md)

## 功能特性

- ✅ 8×8游戏地图
- ✅ 基础三消玩法
- ✅ 4种特殊元素与组合效果
- ✅ 3种道具系统
- ✅ 62个成就系统
- ✅ 休闲模式 & 比赛模式
- ✅ 丰富的动画效果
- ✅ 排行榜系统

## 许可证

本项目为教学项目。
