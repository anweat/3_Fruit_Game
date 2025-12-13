#include "GameView.h"
#include "SwapAnimationRenderer.h"
#include "EliminationAnimationRenderer.h"
#include "FallAnimationRenderer.h"
#include "ShuffleAnimationRenderer.h"
#include <QDebug>
#include <QOpenGLFunctions>
#include <QPainter>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数
 */
GameView::GameView(QWidget *parent)
    : QOpenGLWidget(parent)
    , gameEngine_(nullptr)
    , gridStartX_(0.1f)
    , gridStartY_(0.1f)
    , cellSize_(0.1f)
    , animationFrame_(0)
    , animController_(nullptr)
    , snapshotManager_(nullptr)
    , swapRenderer_(nullptr)
    , eliminationRenderer_(nullptr)
    , fallRenderer_(nullptr)
    , shuffleRenderer_(nullptr)
    , selectedRow_(-1)
    , selectedCol_(-1)
    , hasSelection_(false)
{
    setMinimumSize(600, 600);
    
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
    
    // 动画定时器
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &GameView::onAnimationTimer);
    animationTimer_->start(16); // ~60 FPS
    
    qDebug() << "GameView created with animation system";
}

/**
 * @brief 析构函数
 */
GameView::~GameView()
{
    makeCurrent();
    
    // 清理纹理
    for (auto* texture : fruitTextures_) {
        if (texture) {
            delete texture;
        }
    }
    fruitTextures_.clear();
    
    // 清理动画组件
    delete swapRenderer_;
    delete eliminationRenderer_;
    delete fallRenderer_;
    delete shuffleRenderer_;
    delete animController_;
    delete snapshotManager_;
    
    doneCurrent();
    
    qDebug() << "GameView destroyed";
}

/**
 * @brief 设置游戏引擎
 */
void GameView::setGameEngine(GameEngine* engine)
{
    gameEngine_ = engine;
    update();
}

/**
 * @brief 设置点击模式（拿取道具）
 */
void GameView::setClickMode(ClickMode mode)
{
    // 如果已经有道具，先取消
    if (propState_ != PropState::NONE) {
        cancelProp();
    }
    
    clickMode_ = mode;
    
    // 如果是道具模式，进入HOLDING状态
    if (mode == ClickMode::PROP_HAMMER || 
        mode == ClickMode::PROP_CLAMP || 
        mode == ClickMode::PROP_MAGIC_WAND) {
        propState_ = PropState::HOLDING;
        heldPropType_ = mode;
        setMouseTracking(true);  // 开启鼠标跟踪
    } else {
        propState_ = PropState::NONE;
        heldPropType_ = ClickMode::NORMAL;
        setMouseTracking(false);
    }
    
    // 清除选中状态
    hasSelection_ = false;
    selectedRow_ = -1;
    selectedCol_ = -1;
    update();
}

/**
 * @brief 更新显示
 */
void GameView::updateDisplay()
{
    update(); // 触发重绘
}

/**
 * @brief 初始化OpenGL
 */
void GameView::initializeGL()
{
    initializeOpenGLFunctions();
    
    // 设置背景颜色（深蓝色）
    glClearColor(0.1f, 0.15f, 0.25f, 1.0f);
    
    // 启用2D混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 启用纹理
    glEnable(GL_TEXTURE_2D);
    
    // 加载水果纹理
    loadTextures();
    
    // 初始化所有动画渲染器的OpenGL函数
    swapRenderer_->initialize();
    eliminationRenderer_->initialize();
    fallRenderer_->initialize();
    shuffleRenderer_->initialize();
    
    qDebug() << "OpenGL initialized";
}

/**
 * @brief 窗口尺寸改变
 */
void GameView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    
    // 计算网格布局
    int minSize = qMin(w, h);
    cellSize_ = (minSize * 0.8f) / MAP_SIZE;  // 80%的空间用于网格
    
    float gridWidth = cellSize_ * MAP_SIZE;
    gridStartX_ = (w - gridWidth) / 2.0f;
    gridStartY_ = (h - gridWidth) / 2.0f;
    
    qDebug() << "Resized:" << w << "x" << h << "Cell size:" << cellSize_;
}

/**
 * @brief 绘制场景
 */
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
    
    // 绘制选中框（仅在空闲状态）
    if (hasSelection_ && animController_->getCurrentPhase() == AnimPhase::IDLE) {
        drawSelection();
    }
    
    // 绘制道具选中框（在空闲状态且持有道具时）
    if (animController_->getCurrentPhase() == AnimPhase::IDLE && propState_ != PropState::NONE) {
        drawPropSelection();
    }
}

/**
 * @brief 加载所有水果纹理
 */
void GameView::loadTextures()
{
    // 水果类型和对应的文件名
    // 索引 0-5: 普通水果，索引 6: CANDY（彩虹糖）
    const char* fruitFiles[] = {
        "resources/textures/apple.png",      // APPLE = 0
        "resources/textures/orange.png",     // ORANGE = 1
        "resources/textures/grape.png",      // GRAPE = 2
        "resources/textures/banana.png",     // BANANA = 3
        "resources/textures/watermelon.png", // WATERMELON = 4
        "resources/textures/strawberry.png", // STRAWBERRY = 5
        "resources/textures/Candy.png"       // CANDY = 6 (彩虹糖/Rainbow)
    };
    
    fruitTextures_.resize(7);  // 7 种纹理
    
    for (int i = 0; i < 7; i++) {
        QImage image(fruitFiles[i]);
        if (image.isNull()) {
            qWarning() << "Failed to load texture:" << fruitFiles[i];
            fruitTextures_[i] = nullptr;
            continue;
        }
        
        // 转换为OpenGL格式（不需要翻转）
        QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
        
        fruitTextures_[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
        fruitTextures_[i]->setData(glImage);
        fruitTextures_[i]->setMinificationFilter(QOpenGLTexture::Linear);
        fruitTextures_[i]->setMagnificationFilter(QOpenGLTexture::Linear);
        fruitTextures_[i]->setWrapMode(QOpenGLTexture::ClampToEdge);
        
        qDebug() << "Loaded texture:" << fruitFiles[i];
    }
}

/**
 * @brief 绘制水果网格（静态层，使用快照数据，排除隐藏格子）
 */
void GameView::drawFruitGrid()
{
    // 动画期间使用快照，空闲时使用实时地图
    const auto& map = (animController_->getCurrentPhase() != AnimPhase::IDLE && !snapshotManager_->isSnapshotEmpty()) 
                      ? snapshotManager_->getSnapshot() 
                      : gameEngine_->getMap();
    
    // 先绘制所有单元格背景
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            float x = gridStartX_ + col * cellSize_;
            float y = gridStartY_ + row * cellSize_;
            glDisable(GL_TEXTURE_2D);
            glColor4f(0.3f, 0.35f, 0.45f, 1.0f);
            drawQuad(x, y, cellSize_);
        }
    }
    
    // 绘制水果纹理
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            const Fruit& fruit = map[row][col];
            // 跳过空位
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

/**
 * @brief 渲染当前动画阶段（分发器）
 */
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
            animSeq, 
            roundIndex, 
            progress, 
            snapshot, 
            engineMap, 
            gridStartX_, 
            gridStartY_, 
            cellSize_, 
            fruitTextures_
        );
    }
}

/**
 * @brief 绘制单个水果（支持位移偏移，用于动画）
 */
void GameView::drawFruit(int row, int col, const Fruit& fruit, float offsetX, float offsetY)
{
    // 计算位置
    float x = gridStartX_ + col * cellSize_ + offsetX;
    float y = gridStartY_ + row * cellSize_ + offsetY;
    
    // 获取纹理索引
    int textureIndex = static_cast<int>(fruit.type);
    
    // 跳过 EMPTY 类型
    if (fruit.type == FruitType::EMPTY) {
        return;
    }
    
    // CANDY 类型使用索引 6
    if (fruit.type == FruitType::CANDY) {
        textureIndex = 6;
    }
    
    if (textureIndex < 0 || textureIndex >= static_cast<int>(fruitTextures_.size())) {
        return;
    }
    
    QOpenGLTexture* texture = fruitTextures_[textureIndex];
    if (!texture) {
        return;
    }
    
    // 绘制水果纹理（不再重复绘制背景）
    glEnable(GL_TEXTURE_2D);
    texture->bind();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    // 留出边距
    float padding = cellSize_ * 0.1f;
    float fruitSize = cellSize_ - padding * 2;
    float fruitX = x + padding;
    float fruitY = y + padding;
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
    glEnd();
    
    texture->release();
    
    // 如果有特殊属性，绘制标记
    if (fruit.special != SpecialType::NONE) {
        glDisable(GL_TEXTURE_2D);
        
        // 根据特殊类型选择颜色
        switch (fruit.special) {
            case SpecialType::LINE_H:
            case SpecialType::LINE_V:
                glColor4f(1.0f, 0.8f, 0.0f, 0.8f); // 金色
                break;
            case SpecialType::DIAMOND:
                glColor4f(0.0f, 0.8f, 1.0f, 0.8f); // 青色
                break;
            case SpecialType::RAINBOW:
                glColor4f(1.0f, 0.0f, 1.0f, 0.8f); // 紫色
                break;
            default:
                break;
        }
        
        // 绘制外框高亮
        float borderSize = 4.0f;
        glLineWidth(borderSize);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + cellSize_, y);
            glVertex2f(x + cellSize_, y + cellSize_);
            glVertex2f(x, y + cellSize_);
        glEnd();
    }
}

/**
 * @brief 绘制选中框
 */
void GameView::drawSelection()
{
    if (selectedRow_ < 0 || selectedCol_ < 0) {
        return;
    }
    
    float x = gridStartX_ + selectedCol_ * cellSize_;
    float y = gridStartY_ + selectedRow_ * cellSize_;
    
    glDisable(GL_TEXTURE_2D);
    
    // 绘制脉冲效果
    float pulse = 0.5f + 0.5f * std::sin(animationFrame_ * 0.1f);
    glColor4f(1.0f, 1.0f, 0.0f, 0.3f + 0.2f * pulse);
    
    // 绘制填充
    drawQuad(x, y, cellSize_);
    
    // 绘制边框
    glColor4f(1.0f, 1.0f, 0.0f, 0.8f + 0.2f * pulse);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + cellSize_, y);
        glVertex2f(x + cellSize_, y + cellSize_);
        glVertex2f(x, y + cellSize_);
    glEnd();
}

/**
 * @brief 绘制四边形
 */
void GameView::drawQuad(float x, float y, float size)
{
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + size, y);
        glVertex2f(x + size, y + size);
        glVertex2f(x, y + size);
    glEnd();
}

/**
 * @brief 屏幕坐标转换为网格坐标
 */
bool GameView::screenToGrid(int x, int y, int& row, int& col)
{
    float gridX = x - gridStartX_;
    float gridY = y - gridStartY_;
    
    col = static_cast<int>(gridX / cellSize_);
    row = static_cast<int>(gridY / cellSize_);
    
    return (row >= 0 && row < MAP_SIZE && col >= 0 && col < MAP_SIZE);
}

/**
 * @brief 鼠标按下事件
 */
void GameView::mousePressEvent(QMouseEvent *event)
{
    if (!gameEngine_) {
        return;
    }
    
    // 动画进行中时不接受新点击
    if (animController_->getCurrentPhase() != AnimPhase::IDLE) {
        return;
    }
    
    int row, col;
    if (screenToGrid(static_cast<int>(event->position().x()), 
                     static_cast<int>(event->position().y()), row, col)) {
        // 根据当前模式分发处理
        if (clickMode_ == ClickMode::NORMAL) {
            handleNormalClick(row, col);
        } else {
            handlePropClick(row, col);
        }
    } else {
        // 点击网格外，取消选中或道具
        if (propState_ != PropState::NONE) {
            cancelProp();
        } else {
            hasSelection_ = false;
        }
    }
    
    update();
}

/**
 * @brief 鼠标释放事件
 */
void GameView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * @brief 处理普通交换模式的点击
 */
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
            
            // 在交换前保存地图快照
            snapshotManager_->saveSnapshot(gameEngine_->getMap());
            
            bool success = gameEngine_->swapFruits(selectedRow_, selectedCol_, row, col);
            
            // 开始交换动画
            beginSwapAnimation(success);
            hasSelection_ = false;
        } else {
            // 不相邻：切换选中目标
            selectedRow_ = row;
            selectedCol_ = col;
            hasSelection_ = true;
        }
    }
}

/**
 * @brief 处理道具模式的点击
 */
void GameView::handlePropClick(int row, int col)
{
    // 状态机处理
    switch (propState_) {
        case PropState::HOLDING:
            // 持有道具状态，第一次点击：选中目标
            if (heldPropType_ == ClickMode::PROP_CLAMP) {
                // 夹子需要选中第一个目标
                propTargetRow1_ = row;
                propTargetCol1_ = col;
                propState_ = PropState::FIRST_SELECTED;
            } else {
                // 锤子和魔法棒只需要一个目标
                propTargetRow1_ = row;
                propTargetCol1_ = col;
                propState_ = PropState::READY;
                // 立即释放
                releaseProp();
            }
            break;
            
        case PropState::FIRST_SELECTED:
            // 已选中第一个目标（仅夹子），第二次点击：选中第二个目标
            if (row == propTargetRow1_ && col == propTargetCol1_) {
                // 点击同一个位置，取消选中
                propState_ = PropState::HOLDING;
                propTargetRow1_ = -1;
                propTargetCol1_ = -1;
            } else {
                // 选中第二个目标
                propTargetRow2_ = row;
                propTargetCol2_ = col;
                propState_ = PropState::READY;
                // 立即释放
                releaseProp();
            }
            break;
            
        default:
            break;
    }
    
    update();
}

/**
 * @brief 动画定时器，驱动AnimationController更新
 */
void GameView::onAnimationTimer()
{
    animationFrame_++;
    
    // 更新AnimationController，检查是否有阶段完成
    bool phaseCompleted = animController_->updateProgress();
    
    // 空闲时仅为选中框做脉冲重绘
    if (animController_->getCurrentPhase() == AnimPhase::IDLE) {
        if (hasSelection_) {
            update();
        }
    } else {
        // 动画期间每帧重绘
        update();
    }
    
    // 如果阶段完成，在渲染后再强制一次update确保新状态被显示
    if (phaseCompleted) {
        update();
    }
}

// ========== 动画阶段控制函数实现 ==========

/**
 * @brief 开始交换动画
 */
void GameView::beginSwapAnimation(bool success)
{
    if (!gameEngine_) return;
    
    // 开始交换动画（状态机）
    animController_->beginSwap(success);
    
    // 更新隐藏格子（隐藏交换的两个格子）
    const auto& animSeq = gameEngine_->getLastAnimation();
    snapshotManager_->updateHiddenCells(animSeq, 0, AnimPhase::SWAPPING);
}

/**
 * @brief 开始消除动画
 */
void GameView::beginEliminationStep(int roundIndex)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    
    // 开始消除动画（状态机）
    animController_->beginElimination(roundIndex);
    
    // 更新隐藏格子（隐藏被消除的格子）
    snapshotManager_->updateHiddenCells(animSeq, roundIndex, AnimPhase::ELIMINATING);
}

/**
 * @brief 开始下落动画
 */
void GameView::beginFallStep(int roundIndex)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    
    // 应用消除到快照（消除完成后才开始下落）
    snapshotManager_->applyElimination(animSeq, roundIndex);
    
    // 开始下落动画（状态机）
    animController_->beginFall(roundIndex);
    
    // 更新隐藏格子（隐藏下落目标位置和新生成位置）
    snapshotManager_->updateHiddenCells(animSeq, roundIndex, AnimPhase::FALLING);
}

/**
 * @brief 开始重排动画
 */
void GameView::beginShuffleAnimation()
{
    if (!gameEngine_) return;
    
    // 开始重排动画（状态机）
    animController_->beginShuffle();
    
    // 隐藏所有格子
    snapshotManager_->hideAllCells();
}

/**
 * @brief 阶段完成回调函数
 */
void GameView::handlePhaseComplete(AnimPhase phase)
{
    if (!gameEngine_) return;
    
    const auto& animSeq = gameEngine_->getLastAnimation();
    int currentRound = animController_->getCurrentRoundIndex();
    
    switch (phase) {
        case AnimPhase::SWAPPING:
            // 交换动画完成
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
                    // 没有消除，回到空闲
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
            // 消除动画完成，进入下落
            beginFallStep(currentRound);
            break;
            
        case AnimPhase::FALLING:
            // 下落动画完成
            // 应用下落到快照
            snapshotManager_->applyFall(animSeq, currentRound, gameEngine_->getMap());
            
            // 检查是否有下一轮消除
            if (currentRound + 1 < static_cast<int>(animSeq.rounds.size())) {
                beginEliminationStep(currentRound + 1);
            } else if (animSeq.shuffled) {
                // 所有轮次完成，开始重排
                beginShuffleAnimation();
            } else {
                // 全部完成，回到空闲
                animController_->reset();
                snapshotManager_->clearSnapshot();
                snapshotManager_->clearHiddenCells();
            }
            break;
            
        case AnimPhase::SHUFFLING:
            // 重排动画完成，回到空闲
            animController_->reset();
            snapshotManager_->clearSnapshot();
            snapshotManager_->clearHiddenCells();
            break;
            
        default:
            break;
    }
}

/**
 * @brief 释放道具效果
 */
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

/**
 * @brief 取消道具使用
 */
void GameView::cancelProp()
{
    propState_ = PropState::NONE;
    heldPropType_ = ClickMode::NORMAL;
    clickMode_ = ClickMode::NORMAL;
    propTargetRow1_ = -1;
    propTargetCol1_ = -1;
    propTargetRow2_ = -1;
    propTargetCol2_ = -1;
    setMouseTracking(false);
    update();
}

/**
 * @brief 鼠标移动事件（用于道具跟随）
 */
void GameView::mouseMoveEvent(QMouseEvent *event)
{
    if (propState_ == PropState::HOLDING) {
        // 道具持有状态，重绘以显示跟随效果
        update();
    }
}

/**
 * @brief 绘制道具选中框
 */
void GameView::drawPropSelection()
{
    glDisable(GL_TEXTURE_2D);
    
    // 绘制第一个选中框
    if (propState_ == PropState::FIRST_SELECTED || propState_ == PropState::READY) {
        if (propTargetRow1_ >= 0 && propTargetCol1_ >= 0) {
            float x = gridStartX_ + propTargetCol1_ * cellSize_;
            float y = gridStartY_ + propTargetRow1_ * cellSize_;
            
            // 根据道具类型选择颜色
            if (heldPropType_ == ClickMode::PROP_HAMMER) {
                glColor4f(0.55f, 0.27f, 0.07f, 0.6f);  // 棕色
            } else if (heldPropType_ == ClickMode::PROP_CLAMP) {
                glColor4f(0.25f, 0.41f, 0.88f, 0.6f);  // 蓝色
            } else if (heldPropType_ == ClickMode::PROP_MAGIC_WAND) {
                glColor4f(0.58f, 0.44f, 0.86f, 0.6f);  // 紫色
            }
            
            // 绘制填充
            drawQuad(x, y, cellSize_);
            
            // 绘制边框
            glColor4f(1.0f, 1.0f, 0.0f, 0.9f);
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + cellSize_, y);
                glVertex2f(x + cellSize_, y + cellSize_);
                glVertex2f(x, y + cellSize_);
            glEnd();
        }
    }
    
    // 绘制第二个选中框（仅夹子）
    if (propState_ == PropState::READY && heldPropType_ == ClickMode::PROP_CLAMP) {
        if (propTargetRow2_ >= 0 && propTargetCol2_ >= 0) {
            float x = gridStartX_ + propTargetCol2_ * cellSize_;
            float y = gridStartY_ + propTargetRow2_ * cellSize_;
            
            glColor4f(0.25f, 0.41f, 0.88f, 0.6f);  // 蓝色
            drawQuad(x, y, cellSize_);
            
            glColor4f(1.0f, 1.0f, 0.0f, 0.9f);
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + cellSize_, y);
                glVertex2f(x + cellSize_, y + cellSize_);
                glVertex2f(x, y + cellSize_);
            glEnd();
        }
    }
}
