# GameView 重构待办事项

## 已完成 ✅

1. **GameView.h 重构**
   - 添加动画组件成员变量（AnimationController, SnapshotManager, 4个渲染器）
   - 删除旧的成员变量（animPhase_, animProgress_, swapRow1-2_, currentRoundIndex_, hidden Cells_, mapSnapshot_等）
   - 简化函数声明

2. **GameView.cpp 部分重构**
   - 构造函数：创建动画组件
   - 析构函数：删除动画组件
   - paintGL：使用renderCurrentAnimation
   - drawFruitGrid：使用SnapshotManager
   - renderCurrentAnimation：新的渲染分发器

## 待完成（需要手动删除/修改的旧代码）

### 1. 删除旧的动画绘制函数（约350行）

在GameView.cpp中删除以下函数：
- `void GameView::drawSwapAnimation()` (行349-376)
- `void GameView::drawEliminationAnimation()` (行509-591)
- `void GameView::drawBombEffects()` (行593-709)
- `void GameView::drawFallAnimation()` (行711-1016)
- `void GameView::drawShuffleAnimation()` (行1240-1330)

### 2. 删除旧的动画更新函数（约120行）

删除以下函数：
- `bool GameView::updateSwapAnimation()` (行1038-1049)
- `bool GameView::updateEliminationAnimation()` (行1061-1072)
- `bool GameView::updateFallAnimation()` (行1084-1095)
- `bool GameView::updateShuffleAnimation()` (行1227-1238)
- `void GameView::updateHiddenCells()` (行1102-1136)
- `bool GameView::isCellHidden(int, int)` (行1138-1141)
- `void GameView::computeColumnHiddenRanges()` (行1097-1100)

### 3. 删除旧的快照管理函数（约70行）

删除以下函数：
- `void GameView::saveMapSnapshot()` (行1145-1149)
- `void GameView::applyEliminationToSnapshot(int)` (行1151-1171)
- `void GameView::applyFallToSnapshot(int)` (行1173-1209)

### 4. 重写begin动画函数（4个函数）

#### beginSwapAnimation (行1018-1036)
```cpp
void GameView::beginSwapAnimation(bool success)
{
    if (!gameEngine_) return;
    
    // 保存快照
    snapshotManager_->saveSnapshot(gameEngine_->getMap());
    
    // 开始交换动画
    animController_->beginSwap(success);
    
    // 更新隐藏格子（交换的两个格子）
    const auto& animSeq = gameEngine_->getLastAnimation();
    snapshotManager_->updateHiddenCells(animSeq, 0, AnimPhase::SWAPPING);
}
```

#### beginEliminationStep (行1051-1059)
```cpp
void GameView::beginEliminationStep(int roundIndex)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    
    // 开始消除动画
    animController_->beginElimination(roundIndex);
    
    // 更新隐藏格子
    snapshotManager_->updateHiddenCells(animSeq, roundIndex, AnimPhase::ELIMINATING);
}
```

#### beginFallStep (行1074-1082)
```cpp
void GameView::beginFallStep(int roundIndex)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    
    // 应用消除到快照
    snapshotManager_->applyElimination(animSeq, roundIndex);
    
    // 开始下落动画
    animController_->beginFall(roundIndex);
    
    // 更新隐藏格子
    snapshotManager_->updateHiddenCells(animSeq, roundIndex, AnimPhase::FALLING);
}
```

#### beginShuffleAnimation (行1213-1225)
```cpp
void GameView::beginShuffleAnimation()
{
    if (!gameEngine_) return;
    
    // 开始重排动画
    animController_->beginShuffle();
    
    // 隐藏所有格子
    snapshotManager_->hideAllCells();
}
```

### 5. 添加handlePhaseComplete回调函数

```cpp
void GameView::handlePhaseComplete(AnimPhase phase)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    int currentRound = animController_->getCurrentRoundIndex();
    
    switch (phase) {
        case AnimPhase::SWAPPING:
            // 交换完成
            if (animController_->isSwapSuccess()) {
                // 应用交换到快照
                snapshotManager_->applySwap(
                    animSeq.swap.row1, animSeq.swap.col1,
                    animSeq.swap.row2, animSeq.swap.col2
                );
                
                // 开始第一轮消除（如果有）
                if (!animSeq.rounds.empty()) {
                    beginEliminationStep(0);
                } else {
                    // 回到空闲
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
            
        case AnimPhase::ELIMINATING:
            // 消除完成，进入下落
            beginFallStep(currentRound);
            break;
            
        case AnimPhase::FALLING:
            // 下落完成
            // 应用下落到快照
            snapshotManager_->applyFall(animSeq, currentRound, gameEngine_->getMap());
            
            // 检查是否有下一轮
            if (currentRound + 1 < static_cast<int>(animSeq.rounds.size())) {
                beginEliminationStep(currentRound + 1);
            } else if (animSeq.shuffled) {
                beginShuffleAnimation();
            } else {
                // 全部完成，回到空闲
                animController_->reset();
                snapshotManager_->clearSnapshot();
                snapshotManager_->clearHiddenCells();
            }
            break;
            
        case AnimPhase::SHUFFLING:
            // 重排完成，回到空闲
            animController_->reset();
            snapshotManager_->clearSnapshot();
            snapshotManager_->clearHiddenCells();
            break;
            
        default:
            break;
    }
}
```

### 6. 重写onAnimationTimer

```cpp
void GameView::onAnimationTimer()
{
    animationFrame_++;
    
    // 更新AnimationController
    animController_->update();
    
    // 空闲时仅为选中框做脉冲重绘
    if (animController_->getCurrentPhase() == AnimPhase::IDLE) {
        if (hasSelection_) {
            update();
        }
    } else {
        // 动画期间每帧重绘
        update();
    }
}
```

### 7. 修复鼠标事件（2处）

#### mousePressEvent
将 `if (animPhase_ != AnimPhase::IDLE)` 改为：
```cpp
if (animController_->getCurrentPhase() != AnimPhase::IDLE)
```

#### handleNormalClick  
删除对旧成员变量的引用，改为：
```cpp
// 保存快照
snapshotManager_->saveSnapshot(gameEngine_->getMap());

// 执行交换
bool success = gameEngine_->trySwap(selectedRow_, selectedCol_, row, col);

// 开始交换动画
beginSwapAnimation(success);
```

## 预计修改量

- **删除代码**：约600行
- **新增代码**：约150行
- **净减少**：约450行

## 重构后的优势

1. **代码行数**：从1421行降至约700行
2. **职责分离**：动画渲染、状态管理、快照管理各自独立
3. **易于扩展**：添加新动画只需创建新的Renderer
4. **易于测试**：各组件可独立测试
5. **易于维护**：每个类职责单一，修改影响范围小
