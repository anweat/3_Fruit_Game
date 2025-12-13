# GameView 动画系统解耦架构设计文档

## 1. 架构概述

### 设计模式
- **策略模式（Strategy Pattern）**: 动画渲染器使用统一接口
- **状态机模式（State Machine Pattern）**: AnimationController管理动画流转
- **依赖倒置原则（DIP）**: GameView依赖抽象接口，不依赖具体实现

### 核心组件
```
IAnimationRenderer (接口)
    ├── SwapAnimationRenderer (交换动画)
    ├── EliminationAnimationRenderer (消除动画)
    ├── FallAnimationRenderer (下落动画)
    └── ShuffleAnimationRenderer (重排动画)

AnimationController (状态机控制器)
SnapshotManager (快照管理器)
GameView (视图层，组合上述组件)
```

## 2. 类设计

### 2.1 IAnimationRenderer (抽象基类)
```cpp
class IAnimationRenderer {
public:
    virtual ~IAnimationRenderer() = default;
    
    // 纯虚函数：渲染接口
    virtual void render(
        const GameAnimationSequence& animSeq,  // 动画序列
        int roundIndex,                        // 轮次索引
        float progress,                        // 进度 0.0~1.0
        const std::vector<std::vector<Fruit>>& snapshot,   // 快照
        const std::vector<std::vector<Fruit>>& engineMap,  // 引擎地图
        float gridStartX, float gridStartY, float cellSize,
        const std::vector<QOpenGLTexture*>& textures,
        QOpenGLFunctions* gl
    ) = 0;
    
protected:
    // 工具函数供子类使用
    void drawFruit(...);
    void drawQuad(...);
};
```

**职责**:
- 定义统一的渲染接口
- 提供通用的绘制工具函数
- 遵循开闭原则：对扩展开放，对修改封闭

### 2.2 SwapAnimationRenderer (交换动画渲染器)
```cpp
class SwapAnimationRenderer : public IAnimationRenderer {
public:
    void render(...) override;
    
private:
    // 无需成员变量，所有信息从 animSeq 获取
};
```

**职责**:
- 绘制两个水果的交换动画
- 支持成功交换（线性移动）
- 支持失败回弹（往返移动）

**算法**:
```
进度 t = progress
如果失败: t = (t<0.5) ? 2t : 2(1-t)  // 回弹效果
偏移 = 方向 × cellSize × t
绘制水果1在(位置1 + 偏移)
绘制水果2在(位置2 - 偏移)
```

### 2.3 EliminationAnimationRenderer (消除动画渲染器)
```cpp
class EliminationAnimationRenderer : public IAnimationRenderer {
public:
    void render(...) override;
    
private:
    void renderElimination(...);    // 绘制消除效果
    void renderBombEffects(...);    // 绘制炸弹特效
};
```

**职责**:
- 绘制水果缩小消失效果
- 绘制炸弹特效（LINE_H, LINE_V, DIAMOND, RAINBOW）

**算法**:
```
消除动画:
    缩放 = 1.0 - progress
    透明度 = 1.0 - progress
    
炸弹特效:
    LINE_H: 横向白色长条，逐渐变窄淡出
    LINE_V: 竖向白色长条，逐渐变窄淡出
    DIAMOND: 白色方形，逐渐放大淡出
    RAINBOW: 全屏白色闪光，逐渐淡出
```

### 2.4 FallAnimationRenderer (下落动画渲染器)
```cpp
class FallAnimationRenderer : public IAnimationRenderer {
public:
    void render(...) override;
    
private:
    void renderOldFruits(...);   // 绘制下落的旧水果
    void renderNewFruits(...);   // 绘制新生成的水果
};
```

**职责**:
- 绘制旧水果下落动画
- 绘制新水果从顶部入场动画

**算法**:
```
旧水果下落:
    当前Y = 起始Y + (目标Y - 起始Y) × progress
    
新水果生成:
    起始Y = 网格上方 -1.5 cellSize
    当前Y = 起始Y + (目标Y - 起始Y) × progress
```

### 2.5 ShuffleAnimationRenderer (重排动画渲染器)
```cpp
class ShuffleAnimationRenderer : public IAnimationRenderer {
public:
    void render(...) override;
};
```

**职责**:
- 绘制死局重排动画
- 旧元素淡出缩小 → 新元素淡入放大

**算法**:
```
阶段1 (0.0~0.5): 旧地图淡出
    透明度 = 1.0 - (progress / 0.5)
    缩放 = 1.0 - (progress / 0.5) × 0.3
    
阶段2 (0.5~1.0): 新地图淡入
    透明度 = (progress - 0.5) / 0.5
    缩放 = 0.7 + (progress - 0.5) / 0.5 × 0.3
```

### 2.6 AnimationController (状态机控制器)
```cpp
class AnimationController {
public:
    void beginSwap(bool success);
    void beginElimination(int roundIndex);
    void beginFall(int roundIndex);
    void beginShuffle();
    bool updateProgress();  // 返回当前阶段是否完成
    
    AnimPhase getCurrentPhase();
    float getProgress();
    int getCurrentRoundIndex();
    
private:
    AnimPhase currentPhase_;
    float progress_;
    int currentRoundIndex_;
};
```

**职责**:
- 管理动画阶段流转
- 统一管理动画进度更新
- 提供阶段完成通知

**状态流转图**:
```
IDLE
  ↓ (用户交换)
SWAPPING
  ↓ (成功)
ELIMINATING (第0轮)
  ↓
FALLING (第0轮)
  ↓
ELIMINATING (第1轮) → ... → FALLING (第N轮)
  ↓
SHUFFLING (如果死局) 或 IDLE
```

### 2.7 SnapshotManager (快照管理器)
```cpp
class SnapshotManager {
public:
    void saveSnapshot(const Map& map);
    void applySwap(row1, col1, row2, col2);
    void applyElimination(animSeq, roundIndex);
    void applyFall(animSeq, roundIndex, engineMap);
    
    void updateHiddenCells(animSeq, roundIndex, phase);
    bool isCellHidden(row, col);
    
private:
    std::vector<std::vector<Fruit>> snapshot_;
    std::set<std::pair<int, int>> hiddenCells_;
};
```

**职责**:
- 保存和管理地图快照
- 根据消息更新快照状态
- 管理隐藏格子集合

**关键时序**:
```
1. GameView::handleClick()
   ↓
2. snapshotManager_->saveSnapshot(引擎地图)
   ↓
3. gameEngine_->swapFruits() // 引擎修改地图
   ↓
4. animController_->beginSwap()
   ↓
5. 动画播放使用快照显示原始状态
```

## 3. 渲染管线

### 3.1 核心流程
```
GameView::paintGL()
    ↓
1. 绘制静态网格（排除 hiddenCells_）
    使用: animPhase_ != IDLE ? snapshot_ : engineMap_
    跳过: snapshotManager_->isCellHidden(row, col)
    ↓
2. 根据 animController_->getCurrentPhase() 分发渲染
    if (SWAPPING):
        swapRenderer_->render(...)
    if (ELIMINATING):
        eliminationRenderer_->render(...)
    if (FALLING):
        fallRenderer_->render(...)
    if (SHUFFLING):
        shuffleRenderer_->render(...)
```

### 3.2 定时器回调
```
GameView::onAnimationTimer()
    ↓
1. bool completed = animController_->updateProgress()
    ↓
2. if (completed):
    根据当前阶段决定下一步:
        SWAPPING → applySwap() → ELIMINATING
        ELIMINATING → applyElimination() → FALLING
        FALLING → applyFall() → 下一轮ELIMINATING 或 IDLE/SHUFFLING
        SHUFFLING → IDLE
    ↓
3. update() // 触发重绘
```

## 4. GameView 简化对比

### 4.1 重构前 (1421行)
```cpp
class GameView {
private:
    // 动画状态
    AnimPhase animPhase_;
    float animProgress_;
    int currentRoundIndex_;
    bool swapSuccess_;
    int swapRow1_, swapCol1_, swapRow2_, swapCol2_;
    Fruit swapFruit1_, swapFruit2_;
    
    // 快照
    std::vector<std::vector<Fruit>> mapSnapshot_;
    std::set<std::pair<int, int>> hiddenCells_;
    
    // 大量的动画函数 (800+ 行)
    void drawSwapAnimation();
    void drawEliminationAnimation();
    void drawBombEffects();
    void drawFallAnimation();
    void drawShuffleAnimation();
    void beginSwapAnimation();
    void updateSwapAnimation();
    void beginEliminationStep();
    void updateEliminationAnimation();
    // ... 等等
};
```

### 4.2 重构后 (约700行)
```cpp
class GameView {
private:
    // 动画系统（解耦后）
    AnimationController* animController_;
    SnapshotManager* snapshotManager_;
    IAnimationRenderer* swapRenderer_;
    IAnimationRenderer* eliminationRenderer_;
    IAnimationRenderer* fallRenderer_;
    IAnimationRenderer* shuffleRenderer_;
    
    // 仅保留分发逻辑
    void paintGL() override {
        drawFruitGrid();  // 使用 snapshotManager_
        
        auto phase = animController_->getCurrentPhase();
        auto progress = animController_->getProgress();
        auto roundIndex = animController_->getCurrentRoundIndex();
        
        switch (phase) {
            case AnimPhase::SWAPPING:
                swapRenderer_->render(...);
                break;
            case AnimPhase::ELIMINATING:
                eliminationRenderer_->render(...);
                break;
            // ...
        }
    }
    
    void onAnimationTimer() {
        if (animController_->updateProgress()) {
            handlePhaseComplete();
        }
        update();
    }
};
```

## 5. 优势总结

### 5.1 职责分离
- **AnimationController**: 只管状态流转
- **SnapshotManager**: 只管快照数据
- **各Renderer**: 只管各自的绘制逻辑
- **GameView**: 只管组合和协调

### 5.2 可维护性
- 修改某个动画效果：只需修改对应的Renderer
- 添加新动画：实现IAnimationRenderer接口即可
- 修改状态流转：只需修改AnimationController

### 5.3 可测试性
- 每个Renderer可以独立单元测试
- AnimationController的状态机可以独立测试
- SnapshotManager的数据管理可以独立测试

### 5.4 可扩展性
- 新增动画类型：继承IAnimationRenderer
- 新增动画阶段：在AnimPhase枚举添加
- 遵循开闭原则

### 5.5 代码规模
| 模块 | 行数 |
|------|------|
| IAnimationRenderer.h/cpp | ~150行 |
| SwapAnimationRenderer | ~100行 |
| EliminationAnimationRenderer | ~250行 |
| FallAnimationRenderer | ~150行 |
| ShuffleAnimationRenderer | ~150行 |
| AnimationController | ~150行 |
| SnapshotManager | ~200行 |
| GameView (简化后) | ~700行 |
| **总计** | **~1850行** |

原GameView: 1421行 → 现在分散到8个文件，每个文件100-250行，更易维护。

## 6. 使用示例

### 6.1 初始化
```cpp
GameView::GameView() {
    animController_ = new AnimationController();
    snapshotManager_ = new SnapshotManager();
    swapRenderer_ = new SwapAnimationRenderer();
    eliminationRenderer_ = new EliminationAnimationRenderer();
    fallRenderer_ = new FallAnimationRenderer();
    shuffleRenderer_ = new ShuffleAnimationRenderer();
    
    // 设置完成回调
    animController_->setPhaseCompleteCallback([this](AnimPhase phase) {
        handlePhaseComplete(phase);
    });
}
```

### 6.2 处理交换
```cpp
void GameView::handleNormalClick(int row, int col) {
    // 1. 保存快照
    snapshotManager_->saveSnapshot(gameEngine_->getMap());
    
    // 2. 调用引擎（修改地图）
    bool success = gameEngine_->swapFruits(selectedRow_, selectedCol_, row, col);
    
    // 3. 开始动画
    animController_->beginSwap(success);
    snapshotManager_->updateHiddenCells(...);
}
```

### 6.3 阶段完成处理
```cpp
void GameView::handlePhaseComplete(AnimPhase phase) {
    switch (phase) {
        case AnimPhase::SWAPPING:
            if (animController_->isSwapSuccess()) {
                snapshotManager_->applySwap(...);
                animController_->beginElimination(0);
            } else {
                animController_->reset();
                snapshotManager_->clearSnapshot();
            }
            break;
            
        case AnimPhase::ELIMINATING:
            snapshotManager_->applyElimination(...);
            animController_->beginFall(currentRound);
            break;
            
        case AnimPhase::FALLING:
            snapshotManager_->applyFall(...);
            int nextRound = animController_->getCurrentRoundIndex() + 1;
            if (nextRound < animSeq.rounds.size()) {
                animController_->beginElimination(nextRound);
            } else if (animSeq.shuffled) {
                animController_->beginShuffle();
            } else {
                animController_->reset();
                snapshotManager_->clearSnapshot();
            }
            break;
    }
}
```

## 7. 下一步工作

1. 完成 FallAnimationRenderer 和 ShuffleAnimationRenderer
2. 更新 CMakeLists.txt 添加新文件
3. 重构 GameView.h 和 GameView.cpp
4. 编译测试
5. 编写单元测试

## 8. 注意事项

### 8.1 线程安全
- 当前设计为单线程，所有操作在UI线程
- 如需多线程，考虑为SnapshotManager添加互斥锁

### 8.2 内存管理
- GameView负责创建和释放所有Renderer
- 使用智能指针（std::unique_ptr）更安全

### 8.3 性能考虑
- 虚函数调用开销极小（~纳秒级）
- 避免在render()中分配内存
- 复用纹理和GL状态
