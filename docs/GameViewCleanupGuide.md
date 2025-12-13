# GameView.cpp 清理指南

## 当前状态
✅ 已完成：
- 头文件重构
- 构造/析构函数更新
- paintGL重构
- drawFruitGrid重构
- renderCurrentAnimation创建
- onAnimationTimer重写
- begin函数重写（beginSwapAnimation, beginEliminationStep, beginFallStep）
- handlePhaseComplete创建

## 需要删除的旧代码

### 1. 删除旧的绘制函数（行349-1323）

**drawSwapAnimation** (行349-376，共28行)
```cpp
void GameView::drawSwapAnimation()
{
    if (swapRow1_ < 0 || swapCol1_ < 0 || swapRow2_ < 0 || swapCol2_ < 0) {
        return;
    }
    // ... 全部删除
}
```

**drawEliminationAnimation** (行509-591，共83行)
```cpp
void GameView::drawEliminationAnimation()
{
    // ... 全部删除
}
```

**drawBombEffects** (行593-709，共117行)
```cpp
void GameView::drawBombEffects()
{
    // ... 全部删除
}
```

**drawFallAnimation** (行711-1016，约306行)
```cpp
void GameView::drawFallAnimation()
{
    // ... 全部删除
}
```

**第二个beginShuffleAnimation** (行1190-1202，共13行) - 这是重复的！
```cpp
void GameView::beginShuffleAnimation()  // 第二个，需要删除
{
    animProgress_ = 0.0f;
    animPhase_ = AnimPhase::SHUFFLING;
    // ... 全部删除
}
```

**updateShuffleAnimation** (行1204-1215，共12行)
```cpp
bool GameView::updateShuffleAnimation()
{
    // ... 全部删除
}
```

**drawShuffleAnimation** (行1217-1323，共107行)
```cpp
void GameView::drawShuffleAnimation()
{
    // ... 全部删除
}
```

### 2. 删除旧的辅助函数（行1074-1188）

**computeColumnHiddenRanges** (行1074-1077)
```cpp
void GameView::computeColumnHiddenRanges()
{
    updateHiddenCells();
}
```

**updateHiddenCells** (行1079-1113)
```cpp
void GameView::updateHiddenCells()
{
    hiddenCells_.clear();
    // ... 全部删除
}
```

**isCellHidden** (行1115-1118)
```cpp
bool GameView::isCellHidden(int row, int col) const
{
    return hiddenCells_.count({row, col}) > 0;
}
```

**saveMapSnapshot** (行1122-1126)
```cpp
void GameView::saveMapSnapshot()
{
    if (!gameEngine_) return;
    mapSnapshot_ = gameEngine_->getMap();
}
```

**applyEliminationToSnapshot** (行1128-1148)
```cpp
void GameView::applyEliminationToSnapshot(int roundIndex)
{
    // ... 全部删除
}
```

**applyFallToSnapshot** (行1150-1188)
```cpp
void GameView::applyFallToSnapshot(int roundIndex)
{
    // ... 全部删除
}
```

### 3. 修复releaseProp函数（行1328-1373）

将：
```cpp
void GameView::releaseProp()
{
    if (!gameEngine_) {
        return;
    }
    
    bool success = false;
    
    // 根据道具类型调用不同的接口
    if (heldPropType_ == ClickMode::PROP_CLAMP) {
        // 夹子：使用专用的强制交换接口
        if (propTargetRow1_ >= 0 && propTargetRow2_ >= 0) {
            // 保存快照
            saveMapSnapshot();
            swapFruit1_ = mapSnapshot_[propTargetRow1_][propTargetCol1_];
            swapFruit2_ = mapSnapshot_[propTargetRow2_][propTargetCol2_];
            
            // 调用夹子专用接口
            success = gameEngine_->useClampProp(propTargetRow1_, propTargetCol1_,
                                                 propTargetRow2_, propTargetCol2_);
            
            if (success) {
                // 开始交换动画
                beginSwapAnimation(true);
            }
        }
    } else {
        // 锤子或魔法棒：单个目标
        // 关键修复：先保存快照，再调用引擎
        saveMapSnapshot();
        
        success = gameEngine_->useProp(heldPropType_, propTargetRow1_, propTargetCol1_);
        
        if (success) {
            // 开始动画
            const auto& seq = gameEngine_->getLastAnimation();
            if (!seq.rounds.empty()) {
                // 开始第 0 轮消除动画
                beginEliminationStep(0);
            }
        }
    }
    
    // 释放后重置状态
    cancelProp();
}
```

改为：
```cpp
void GameView::releaseProp()
{
    if (!gameEngine_) {
        return;
    }
    
    bool success = false;
    
    // 根据道具类型调用不同的接口
    if (heldPropType_ == ClickMode::PROP_CLAMP) {
        // 夹子：使用专用的强制交换接口
        if (propTargetRow1_ >= 0 && propTargetRow2_ >= 0) {
            // 保存快照
            snapshotManager_->saveSnapshot(gameEngine_->getMap());
            
            // 调用夹子专用接口
            success = gameEngine_->useClampProp(propTargetRow1_, propTargetCol1_,
                                                 propTargetRow2_, propTargetCol2_);
            
            if (success) {
                // 开始交换动画
                beginSwapAnimation(true);
            }
        }
    } else {
        // 锤子或魔法棒：单个目标
        // 关键修复：先保存快照，再调用引擎
        snapshotManager_->saveSnapshot(gameEngine_->getMap());
        
        success = gameEngine_->useProp(heldPropType_, propTargetRow1_, propTargetCol1_);
        
        if (success) {
            // 开始动画
            const auto& seq = gameEngine_->getLastAnimation();
            if (!seq.rounds.empty()) {
                // 开始第 0 轮消除动画
                beginEliminationStep(0);
            }
        }
    }
    
    // 释放后重置状态
    cancelProp();
}
```

## 删除总览

| 函数名 | 起始行 | 行数 | 说明 |
|--------|--------|------|------|
| drawSwapAnimation | 349 | ~28 | 旧交换动画绘制 |
| drawEliminationAnimation | 509 | ~83 | 旧消除动画绘制 |
| drawBombEffects | 593 | ~117 | 旧炸弹特效绘制 |
| drawFallAnimation | 711 | ~306 | 旧下落动画绘制 |
| computeColumnHiddenRanges | 1074 | ~4 | 旧隐藏格子计算 |
| updateHiddenCells | 1079 | ~35 | 旧隐藏格子更新 |
| isCellHidden | 1115 | ~4 | 旧隐藏格子检查 |
| saveMapSnapshot | 1122 | ~5 | 旧快照保存 |
| applyEliminationToSnapshot | 1128 | ~21 | 旧快照更新 |
| applyFallToSnapshot | 1150 | ~39 | 旧快照更新 |
| beginShuffleAnimation（重复） | 1190 | ~13 | 重复定义，删除这个 |
| updateShuffleAnimation | 1204 | ~12 | 旧重排动画更新 |
| drawShuffleAnimation | 1217 | ~107 | 旧重排动画绘制 |

**预计删除总行数：约774行**

## 修复后的文件结构

```
GameView.cpp (约685行)
├── 构造/析构函数
├── setGameEngine/updateDisplay/setClickMode
├── OpenGL初始化（initializeGL, resizeGL）
├── 渲染函数
│   ├── paintGL
│   ├── loadTextures
│   ├── drawFruitGrid
│   ├── renderCurrentAnimation (新！)
│   ├── drawFruit
│   ├── drawQuad
│   └── drawSelection/drawPropSelection
├── 鼠标事件
│   ├── mousePressEvent
│   ├── mouseReleaseEvent
│   ├── mouseMoveEvent
│   ├── screenToGrid
│   ├── handleNormalClick
│   └── handlePropClick
├── 动画控制（新架构！）
│   ├── onAnimationTimer
│   ├── beginSwapAnimation
│   ├── beginEliminationStep
│   ├── beginFallStep
│   ├── beginShuffleAnimation
│   └── handlePhaseComplete
└── 道具系统
    ├── releaseProp
    └── cancelProp
```

## 验证步骤

删除完成后，编译验证：
```bash
cd build
mingw32-make FruitCrush -j4
```

应该没有"no declaration matches"或"was not declared"错误。
