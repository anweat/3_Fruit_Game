# GameView 重构计划

## 当前状态

### ✅ 已完成的组件

1. **动画渲染器体系** (策略模式)
   - `IAnimationRenderer.h/cpp` - 抽象基类，定义统一接口
   - `SwapAnimationRenderer.h/cpp` - 交换动画
   - `EliminationAnimationRenderer.h/cpp` - 消除动画
   - `FallAnimationRenderer.h/cpp` - 下落动画
   - `ShuffleAnimationRenderer.h/cpp` - 重排动画

2. **状态管理组件**
   - `AnimationController.h/cpp` - 动画状态机
   - `SnapshotManager.h/cpp` - 快照管理器

3. **构建配置**
   - ✅ CMakeLists.txt 已更新
   - ✅ include路径已配置

## 重构后的GameView结构

### 成员变量简化

**重构前 (1421行)**:
```cpp
class GameView {
private:
    // 动画状态（分散在多个变量中）
    AnimPhase animPhase_;
    float animProgress_;
    int animationFrame_;
    int currentRoundIndex_;
    bool swapSuccess_;
    int swapRow1_, swapCol1_, swapRow2_, swapCol2_;
    Fruit swapFruit1_, swapFruit2_;
    
    // 快照数据
    std::vector<std::vector<Fruit>> mapSnapshot_;
    std::set<std::pair<int, int>> hiddenCells_;
    
    // ... 大量动画相关函数（约800行）
};
```

**重构后 (约700行)**:
```cpp
class GameView : public QOpenGLWidget, protected QOpenGLFunctions {
private:
    // ========== 引擎和基础 ==========
    GameEngine* gameEngine_;
    std::vector<QOpenGLTexture*> fruitTextures_;
    float gridStartX_, gridStartY_, cellSize_;
    QTimer* animationTimer_;
    int animationFrame_;
    
    // ========== 动画系统（解耦组件）==========
    AnimationController* animController_;
    SnapshotManager* snapshotManager_;
    IAnimationRenderer* swapRenderer_;
    IAnimationRenderer* eliminationRenderer_;
    IAnimationRenderer* fallRenderer_;
    IAnimationRenderer* shuffleRenderer_;
    
    // ========== 选中和道具状态 ==========
    int selectedRow_, selectedCol_;
    bool hasSelection_;
    ClickMode clickMode_;
    PropState propState_;
    ClickMode heldPropType_;
    int propTargetRow1_, propTargetCol1_;
    int propTargetRow2_, propTargetCol2_;
};
```

### 核心方法重构

#### 1. 构造/析构函数

```cpp
GameView::GameView(QWidget *parent)
    : QOpenGLWidget(parent)
    // ... 基础初始化 ...
{
    // 创建动画系统组件
    animController_ = new AnimationController();
    snapshotManager_ = new SnapshotManager();
    swapRenderer_ = new SwapAnimationRenderer();
    eliminationRenderer_ = new EliminationAnimationRenderer();
    fallRenderer_ = new FallAnimationRenderer();
    shuffleRenderer_ = new ShuffleAnimationRenderer();
    
    // 设置阶段完成回调
    animController_->setPhaseCompleteCallback([this](AnimPhase phase) {
        handlePhaseComplete(phase);
    });
    
    // 定时器
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &GameView::onAnimationTimer);
    animationTimer_->start(16);
}

GameView::~GameView()
{
    makeCurrent();
    
    // 清理纹理
    for (auto* texture : fruitTextures_) {
        delete texture;
    }
    
    // 清理动画组件
    delete swapRenderer_;
    delete eliminationRenderer_;
    delete fallRenderer_;
    delete shuffleRenderer_;
    delete animController_;
    delete snapshotManager_;
    
    doneCurrent();
}
```

#### 2. paintGL() - 渲染分发器

```cpp
void GameView::paintGL()
{
    // 清空屏幕
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置2D正交投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 绘制网格背景
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.2f, 0.25f, 0.35f, 1.0f);
    drawQuad(gridStartX_ - 10, gridStartY_ - 10, cellSize_ * MAP_SIZE + 20);
    
    // 绘制水果
    if (gameEngine_) {
        // 基础网格（使用快照或引擎地图，排除隐藏格子）
        drawFruitGrid();
        
        // 根据动画阶段分发渲染
        AnimPhase phase = animController_->getCurrentPhase();
        if (phase != AnimPhase::IDLE) {
            renderCurrentAnimation();
        }
    }
    
    // 绘制选中框和道具框
    if (hasSelection_ && animController_->getCurrentPhase() == AnimPhase::IDLE) {
        drawSelection();
    }
    if (animController_->getCurrentPhase() == AnimPhase::IDLE && propState_ != PropState::NONE) {
        drawPropSelection();
    }
}
```

#### 3. renderCurrentAnimation() - 动画渲染分发

```cpp
void GameView::renderCurrentAnimation()
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    AnimPhase phase = animController_->getCurrentPhase();
    float progress = animController_->getProgress();
    int roundIndex = animController_->getCurrentRoundIndex();
    
    const auto& snapshot = snapshotManager_->getSnapshot();
    const auto& engineMap = gameEngine_->getMap();
    
    IAnimationRenderer* renderer = nullptr;
    
    switch (phase) {
        case AnimPhase::SWAPPING:
            renderer = swapRenderer_;
            break;
        case AnimPhase::ELIMINATING:
            renderer = eliminationRenderer_;
            break;
        case AnimPhase::FALLING:
            renderer = fallRenderer_;
            break;
        case AnimPhase::SHUFFLING:
            renderer = shuffleRenderer_;
            break;
        default:
            return;
    }
    
    if (renderer) {
        renderer->render(
            animSeq, roundIndex, progress,
            snapshot, engineMap,
            gridStartX_, gridStartY_, cellSize_,
            fruitTextures_, this
        );
    }
}
```

#### 4. drawFruitGrid() - 静态网格绘制

```cpp
void GameView::drawFruitGrid()
{
    // 动画期间使用快照，空闲时使用实时地图
    const auto& map = (animController_->getCurrentPhase() != AnimPhase::IDLE && 
                       !snapshotManager_->isSnapshotEmpty()) 
                      ? snapshotManager_->getSnapshot()
                      : gameEngine_->getMap();
    
    // 绘制所有单元格背景
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            float x = gridStartX_ + col * cellSize_;
            float y = gridStartY_ + row * cellSize_;
            glDisable(GL_TEXTURE_2D);
            glColor4f(0.3f, 0.35f, 0.45f, 1.0f);
            drawQuad(x, y, cellSize_);
        }
    }
    
    // 绘制水果纹理（跳过隐藏格子）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            const Fruit& fruit = map[row][col];
            if (fruit.type == FruitType::EMPTY) {
                continue;
            }
            
            // 隐藏集合中的格子由动画层负责绘制
            if (snapshotManager_->isCellHidden(row, col)) {
                continue;
            }
            
            drawFruit(row, col, fruit, 0.0f, 0.0f);
        }
    }
}
```

#### 5. onAnimationTimer() - 动画更新

```cpp
void GameView::onAnimationTimer()
{
    animationFrame_++;
    
    AnimPhase phase = animController_->getCurrentPhase();
    
    if (phase != AnimPhase::IDLE) {
        // 更新动画进度
        bool completed = animController_->updateProgress();
        
        if (completed) {
            // 阶段完成，处理状态流转
            handlePhaseComplete(phase);
        }
        
        update();  // 触发重绘
    } else {
        // 空闲时仅为选中框做脉冲重绘
        if (hasSelection_) {
            update();
        }
    }
}
```

#### 6. handlePhaseComplete() - 阶段完成处理

```cpp
void GameView::handlePhaseComplete(AnimPhase phase)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    
    switch (phase) {
        case AnimPhase::SWAPPING: {
            // 交换完成，应用到快照
            if (animController_->isSwapSuccess()) {
                const auto& swap = animSeq.swap;
                snapshotManager_->applySwap(swap.row1, swap.col1, swap.row2, swap.col2);
                
                // 开始第0轮消除
                if (!animSeq.rounds.empty()) {
                    animController_->beginElimination(0);
                    snapshotManager_->updateHiddenCells(
                        animSeq, 0, AnimPhase::ELIMINATING
                    );
                } else {
                    // 无消除轮次，回到空闲
                    animController_->reset();
                    snapshotManager_->clearSnapshot();
                    snapshotManager_->clearHiddenCells();
                }
            } else {
                // 交换失败，回到空闲
                animController_->reset();
                snapshotManager_->clearSnapshot();
                snapshotManager_->clearHiddenCells();
            }
            break;
        }
        
        case AnimPhase::ELIMINATING: {
            // 消除完成，应用到快照
            int roundIndex = animController_->getCurrentRoundIndex();
            snapshotManager_->applyElimination(animSeq, roundIndex);
            
            // 开始本轮下落
            animController_->beginFall(roundIndex);
            snapshotManager_->updateHiddenCells(
                animSeq, roundIndex, AnimPhase::FALLING
            );
            break;
        }
        
        case AnimPhase::FALLING: {
            // 下落完成，应用到快照
            int roundIndex = animController_->getCurrentRoundIndex();
            snapshotManager_->applyFall(animSeq, roundIndex, gameEngine_->getMap());
            
            int nextRound = roundIndex + 1;
            if (nextRound < static_cast<int>(animSeq.rounds.size())) {
                // 开始下一轮消除
                animController_->beginElimination(nextRound);
                snapshotManager_->updateHiddenCells(
                    animSeq, nextRound, AnimPhase::ELIMINATING
                );
            } else {
                // 所有轮次完成，检查是否有重排
                if (animSeq.shuffled) {
                    animController_->beginShuffle();
                    snapshotManager_->hideAllCells();
                } else {
                    // 回到空闲
                    animController_->reset();
                    snapshotManager_->clearSnapshot();
                    snapshotManager_->clearHiddenCells();
                }
            }
            break;
        }
        
        case AnimPhase::SHUFFLING: {
            // 重排完成，回到空闲
            animController_->reset();
            snapshotManager_->clearSnapshot();
            snapshotManager_->clearHiddenCells();
            break;
        }
        
        default:
            break;
    }
}
```

#### 7. handleNormalClick() - 普通点击处理

```cpp
void GameView::handleNormalClick(int row, int col)
{
    if (!hasSelection_) {
        // 第一次点击，选中水果
        selectedRow_ = row;
        selectedCol_ = col;
        hasSelection_ = true;
    } else {
        // 第二次点击
        if (row == selectedRow_ && col == selectedCol_) {
            // 点击同一个，取消选中
            hasSelection_ = false;
        } else if (std::abs(row - selectedRow_) + std::abs(col - selectedCol_) == 1) {
            // 相邻元素触发交换
            
            // 关键时序：先保存快照，再调用引擎
            snapshotManager_->saveSnapshot(gameEngine_->getMap());
            
            bool success = gameEngine_->swapFruits(selectedRow_, selectedCol_, row, col);
            
            // 开始交换动画
            animController_->beginSwap(success);
            
            // 更新隐藏格子（隐藏交换的两个格子）
            std::set<std::pair<int, int>> hiddenCells;
            hiddenCells.insert({selectedRow_, selectedCol_});
            hiddenCells.insert({row, col});
            // snapshotManager_会在内部管理这些
            
            hasSelection_ = false;
        } else {
            // 不相邻：切换选中目标
            selectedRow_ = row;
            selectedCol_ = col;
            hasSelection_ = true;
        }
    }
}
```

#### 8. releaseProp() - 道具释放

```cpp
void GameView::releaseProp()
{
    if (!gameEngine_) return;
    
    bool success = false;
    
    if (heldPropType_ == ClickMode::PROP_CLAMP) {
        // 夹子：强制交换
        if (propTargetRow1_ >= 0 && propTargetRow2_ >= 0) {
            // 先保存快照
            snapshotManager_->saveSnapshot(gameEngine_->getMap());
            
            // 调用夹子专用接口
            success = gameEngine_->useClampProp(
                propTargetRow1_, propTargetCol1_,
                propTargetRow2_, propTargetCol2_
            );
            
            if (success) {
                // 开始交换动画
                animController_->beginSwap(true);
            }
        }
    } else {
        // 锤子或魔法棒：单个目标
        // 关键时序：先保存快照，再调用引擎
        snapshotManager_->saveSnapshot(gameEngine_->getMap());
        
        success = gameEngine_->useProp(heldPropType_, propTargetRow1_, propTargetCol1_);
        
        if (success) {
            const auto& seq = gameEngine_->getLastAnimation();
            if (!seq.rounds.empty()) {
                // 开始第0轮消除动画
                animController_->beginElimination(0);
                snapshotManager_->updateHiddenCells(
                    seq, 0, AnimPhase::ELIMINATING
                );
            }
        }
    }
    
    // 释放后重置状态
    cancelProp();
}
```

### 删除的方法（移到渲染器）

这些方法不再需要，由各渲染器实现：
- ❌ `drawSwapAnimation()` → SwapAnimationRenderer
- ❌ `drawEliminationAnimation()` → EliminationAnimationRenderer
- ❌ `drawBombEffects()` → EliminationAnimationRenderer
- ❌ `drawFallAnimation()` → FallAnimationRenderer
- ❌ `drawShuffleAnimation()` → ShuffleAnimationRenderer
- ❌ `beginSwapAnimation()` → AnimationController
- ❌ `updateSwapAnimation()` → AnimationController
- ❌ `beginEliminationStep()` → AnimationController
- ❌ `updateEliminationAnimation()` → AnimationController
- ❌ `beginFallStep()` → AnimationController
- ❌ `updateFallAnimation()` → AnimationController
- ❌ `beginShuffleAnimation()` → AnimationController
- ❌ `updateShuffleAnimation()` → AnimationController
- ❌ `saveMapSnapshot()` → SnapshotManager
- ❌ `applyEliminationToSnapshot()` → SnapshotManager
- ❌ `applyFallToSnapshot()` → SnapshotManager
- ❌ `updateHiddenCells()` → SnapshotManager
- ❌ `isCellHidden()` → SnapshotManager

## 代码量对比

| 文件 | 重构前 | 重构后 | 减少 |
|------|--------|--------|------|
| GameView.h | 197行 | ~150行 | -47行 |
| GameView.cpp | 1421行 | ~700行 | -721行 |
| **总计** | **1618行** | **~850行** | **-768行** |

**新增文件**:
- IAnimationRenderer: 150行
- AnimationController: 150行
- SnapshotManager: 200行
- SwapAnimationRenderer: 100行
- EliminationAnimationRenderer: 250行
- FallAnimationRenderer: 150行
- ShuffleAnimationRenderer: 150行
- **新增总计**: **1150行**

**总代码量**: 850 + 1150 = 2000行（分布在8个文件）

虽然总代码量略有增加，但：
1. 每个文件职责单一，100-250行
2. 易于维护和测试
3. 符合SOLID原则
4. 易于扩展新动画

## 下一步

1. ✅ 完成所有渲染器实现
2. ✅ 更新CMakeLists.txt
3. ⏳ 重构GameView.h
4. ⏳ 重构GameView.cpp
5. ⏳ 编译测试
6. ⏳ 运行测试
7. ⏳ 性能对比

## 优势总结

### 1. 架构清晰
```
GameView (协调者)
    ├── AnimationController (状态机)
    ├── SnapshotManager (数据管理)
    └── Renderers (绘制策略)
        ├── SwapAnimationRenderer
        ├── EliminationAnimationRenderer
        ├── FallAnimationRenderer
        └── ShuffleAnimationRenderer
```

### 2. 职责分离
- GameView: 只负责协调和分发
- AnimationController: 只负责状态流转
- SnapshotManager: 只负责数据管理
- Renderers: 只负责各自的绘制

### 3. 易于测试
- 每个Renderer可独立单元测试
- AnimationController可独立测试状态流转
- SnapshotManager可独立测试数据管理

### 4. 易于扩展
- 新增动画：继承IAnimationRenderer
- 新增状态：在AnimPhase添加
- 符合开闭原则

### 5. 代码可读性
- 每个文件100-250行
- 功能清晰，易于理解
- 注释完善
