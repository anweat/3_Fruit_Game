#include "GameView.h"
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
    , selectedRow_(-1)
    , selectedCol_(-1)
    , hasSelection_(false)
    , animationFrame_(0)
    , animPhase_(AnimPhase::IDLE)
    , animProgress_(0.0f)
    , swapSuccess_(false)
    , swapRow1_(-1)
    , swapCol1_(-1)
    , swapRow2_(-1)
    , swapCol2_(-1)
    , currentRoundIndex_(-1)
{
    setMinimumSize(600, 600);
    
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &GameView::onAnimationTimer);
    animationTimer_->start(16); // ~60 FPS
    
    qDebug() << "GameView created";
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
        // 基础网格和静态水果（隐藏 hiddenCells_ 中的格子）
        drawFruitGrid();
        
        // 覆盖层：交换动画（用保存的交换前水果绘制）
        if (animPhase_ == AnimPhase::SWAPPING) {
            drawSwapAnimation();
        }
        // 覆盖层：消除动画
        if (animPhase_ == AnimPhase::ELIMINATING) {
            drawEliminationAnimation();
            drawBombEffects();  // 炸弹特效动画
        }
        // 覆盖层：下落动画
        if (animPhase_ == AnimPhase::FALLING) {
            drawFallAnimation();
        }
        // 覆盖层：重排动画
        if (animPhase_ == AnimPhase::SHUFFLING) {
            drawShuffleAnimation();
        }
    }
    
    // 绘制选中框（仅在空闲状态）
    if (hasSelection_ && animPhase_ == AnimPhase::IDLE) {
        drawSelection();
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
 * @brief 绘制水果网格（静态层，使用快照数据，隐藏 hiddenCells_ 中的格子）
 */
void GameView::drawFruitGrid()
{
    // 动画期间使用快照，空闲时使用实时地图
    const auto& map = (animPhase_ != AnimPhase::IDLE && !mapSnapshot_.empty()) 
                      ? mapSnapshot_ 
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
            if (hiddenCells_.count({row, col}) > 0) {
                continue;
            }
            
            drawFruit(row, col, fruit, 0.0f, 0.0f);
        }
    }
}

/**
 * @brief 绘制交换动画（用保存的交换前水果绘制）
 */
void GameView::drawSwapAnimation()
{
    if (swapRow1_ < 0 || swapCol1_ < 0 || swapRow2_ < 0 || swapCol2_ < 0) {
        return;
    }
    
    float t = animProgress_;
    if (!swapSuccess_) {
        // 回弹：前半程出去，后半程回来
        t = (t <= 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
    }
    
    // 计算位移方向
    int dirRow = swapRow2_ - swapRow1_;
    int dirCol = swapCol2_ - swapCol1_;
    float dx = dirCol * cellSize_ * t;
    float dy = dirRow * cellSize_ * t;
    
    // 绘制第一个水果（从位置1向位置2移动）
    if (swapFruit1_.type != FruitType::EMPTY) {
        drawFruit(swapRow1_, swapCol1_, swapFruit1_, dx, dy);
    }
    
    // 绘制第二个水果（从位置2向位置1移动）
    if (swapFruit2_.type != FruitType::EMPTY) {
        drawFruit(swapRow2_, swapCol2_, swapFruit2_, -dx, -dy);
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
 * @brief 绘制一轮消除动画（使用快照数据）
 */
void GameView::drawEliminationAnimation()
{
    if (!gameEngine_ || currentRoundIndex_ < 0) {
        return;
    }

    const GameAnimationSequence& animSeq = gameEngine_->getLastAnimation();
    if (currentRoundIndex_ >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }

    const EliminationStep& step = animSeq.rounds[currentRoundIndex_].elimination;
    if (step.positions.empty()) {
        return;
    }

    float t = animProgress_;
    float scale = 1.0f - t;
    float alpha = 1.0f - t;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 使用快照数据（消除动画需要显示快照中的水果，因为它们还没被清除）
    const auto& map = mapSnapshot_.empty() ? gameEngine_->getMap() : mapSnapshot_;

    for (const auto& pos : step.positions) {
        int row = pos.first;
        int col = pos.second;
        if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
            continue;
        }

        const Fruit& fruit = map[row][col];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }

        float cellX = gridStartX_ + col * cellSize_;
        float cellY = gridStartY_ + row * cellSize_;
        float centerX = cellX + cellSize_ * 0.5f;
        float centerY = cellY + cellSize_ * 0.5f;

        float size = cellSize_ * scale;
        float x = centerX - size * 0.5f;
        float y = centerY - size * 0.5f;

        // 绘制缩小的水果纹理
        int textureIndex = static_cast<int>(fruit.type);
        // CANDY 类型使用索引 6
        if (fruit.type == FruitType::CANDY) {
            textureIndex = 6;
        }
        if (textureIndex >= 0 && textureIndex < static_cast<int>(fruitTextures_.size()) && fruitTextures_[textureIndex]) {
            glEnable(GL_TEXTURE_2D);
            fruitTextures_[textureIndex]->bind();
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            
            float padding = size * 0.1f;
            float fruitSize = size - padding * 2;
            float fruitX = x + padding;
            float fruitY = y + padding;
            
            glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
            glEnd();
            
            fruitTextures_[textureIndex]->release();
        }
    }
}

/**
 * @brief 绘制炸弹特效动画
 * 
 * 特效类型：
 * - LINE_H: 横排白色长条，逐渐变窄变淡
 * - LINE_V: 竖排白色长条，逐渐变窄变淡
 * - DIAMOND: 白色正方形，逐渐放大变暗
 * - RAINBOW: 全屏白色闪光
 */
void GameView::drawBombEffects()
{
    if (!gameEngine_ || currentRoundIndex_ < 0) {
        return;
    }
    
    const GameAnimationSequence& animSeq = gameEngine_->getLastAnimation();
    if (currentRoundIndex_ >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }
    
    const EliminationStep& step = animSeq.rounds[currentRoundIndex_].elimination;
    if (step.bombEffects.empty()) {
        return;
    }
    
    float t = animProgress_;  // 0 → 1
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    
    for (const auto& effect : step.bombEffects) {
        switch (effect.type) {
            case BombEffectType::LINE_H: {
                // 横排特效：白色长条覆盖整行，逐渐变窄变淡
                float rowY = gridStartY_ + effect.row * cellSize_;
                float startX = gridStartX_;
                float fullWidth = cellSize_ * MAP_SIZE;
                float centerY = rowY + cellSize_ * 0.5f;
                
                // 高度从 cellSize_ 缩小到 0
                float height = cellSize_ * (1.0f - t);
                // 透明度从 0.8 淡化到 0
                float alpha = 0.8f * (1.0f - t);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(startX, centerY - height * 0.5f);
                    glVertex2f(startX + fullWidth, centerY - height * 0.5f);
                    glVertex2f(startX + fullWidth, centerY + height * 0.5f);
                    glVertex2f(startX, centerY + height * 0.5f);
                glEnd();
                break;
            }
            
            case BombEffectType::LINE_V: {
                // 竖排特效：白色长条覆盖整列，逐渐变窄变淡
                float colX = gridStartX_ + effect.col * cellSize_;
                float startY = gridStartY_;
                float fullHeight = cellSize_ * MAP_SIZE;
                float centerX = colX + cellSize_ * 0.5f;
                
                // 宽度从 cellSize_ 缩小到 0
                float width = cellSize_ * (1.0f - t);
                // 透明度从 0.8 淡化到 0
                float alpha = 0.8f * (1.0f - t);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(centerX - width * 0.5f, startY);
                    glVertex2f(centerX + width * 0.5f, startY);
                    glVertex2f(centerX + width * 0.5f, startY + fullHeight);
                    glVertex2f(centerX - width * 0.5f, startY + fullHeight);
                glEnd();
                break;
            }
            
            case BombEffectType::DIAMOND: {
                // 菱形特效：白色正方形从中心放大并变暗
                float centerX = gridStartX_ + effect.col * cellSize_ + cellSize_ * 0.5f;
                float centerY = gridStartY_ + effect.row * cellSize_ + cellSize_ * 0.5f;
                
                // 大小从 1 格子放大到 5×5 范围
                float maxSize = cellSize_ * (effect.range * 2 + 1);  // 5x5 = 5格子
                float size = cellSize_ + (maxSize - cellSize_) * t;
                // 透明度从 0.6 淡化到 0
                float alpha = 0.6f * (1.0f - t);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(centerX - size * 0.5f, centerY - size * 0.5f);
                    glVertex2f(centerX + size * 0.5f, centerY - size * 0.5f);
                    glVertex2f(centerX + size * 0.5f, centerY + size * 0.5f);
                    glVertex2f(centerX - size * 0.5f, centerY + size * 0.5f);
                glEnd();
                break;
            }
            
            case BombEffectType::RAINBOW: {
                // 彩虹特效：全屏白色闪光
                float startX = gridStartX_;
                float startY = gridStartY_;
                float fullSize = cellSize_ * MAP_SIZE;
                // 透明度从 0.5 淡化到 0
                float alpha = 0.5f * (1.0f - t);
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glBegin(GL_QUADS);
                    glVertex2f(startX, startY);
                    glVertex2f(startX + fullSize, startY);
                    glVertex2f(startX + fullSize, startY + fullSize);
                    glVertex2f(startX, startY + fullSize);
                glEnd();
                break;
            }
            
            default:
                break;
        }
    }
}

/**
 * @brief 绘制一轮下落动画（包括老元素下落 + 新元素入场）
 * 
 * 下落动画使用引擎的最终数据，因为我们需要知道最终位置的水果类型
 */
void GameView::drawFallAnimation()
{
    if (!gameEngine_ || currentRoundIndex_ < 0) {
        return;
    }

    const GameAnimationSequence& animSeq = gameEngine_->getLastAnimation();
    if (currentRoundIndex_ >= static_cast<int>(animSeq.rounds.size())) {
        return;
    }

    const FallStep& step = animSeq.rounds[currentRoundIndex_].fall;
    float t = animProgress_;
    
    // 下落动画使用引擎的最终地图（因为需要知道最终水果类型）
    const auto& map = gameEngine_->getMap();

    // 1. 绘制本轮下落的老元素（from→to 插值）
    for (const auto& move : step.moves) {
        int toRow = move.toRow;
        int toCol = move.toCol;
        if (toRow < 0 || toRow >= MAP_SIZE || toCol < 0 || toCol >= MAP_SIZE) {
            continue;
        }

        const Fruit& fruit = map[toRow][toCol];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }

        float fromY = gridStartY_ + move.fromRow * cellSize_;
        float toY   = gridStartY_ + move.toRow   * cellSize_;
        float curY  = fromY + (toY - fromY) * t;
        float offsetY = curY - toY;

        drawFruit(toRow, toCol, fruit, 0.0f, offsetY);
    }

    // 2. 绘制本轮新生成的水果（从网格上方掉入）
    for (const auto& nf : step.newFruits) {
        int row = nf.row;
        int col = nf.col;
        if (row < 0 || row >= MAP_SIZE || col < 0 || col >= MAP_SIZE) {
            continue;
        }

        const Fruit& fruit = map[row][col];
        if (fruit.type == FruitType::EMPTY) {
            continue;
        }

        // 新水果从网格上方落下
        float startY = gridStartY_ - cellSize_ * 1.5f;
        float endY   = gridStartY_ + row * cellSize_;
        float curY   = startY + (endY - startY) * t;
        float offsetY = curY - endY;

        drawFruit(row, col, fruit, 0.0f, offsetY);
    }
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
    if (animPhase_ != AnimPhase::IDLE) {
        return;
    }
    
    int row, col;
    if (screenToGrid(static_cast<int>(event->position().x()), 
                     static_cast<int>(event->position().y()), row, col)) {
        if (!hasSelection_) {
            // 第一次点击，选中水果
            selectedRow_ = row;
            selectedCol_ = col;
            hasSelection_ = true;
            qDebug() << "Selected:" << row << "," << col;
        } else {
            // 第二次点击
            if (row == selectedRow_ && col == selectedCol_) {
                // 点击同一个，取消选中
                hasSelection_ = false;
                qDebug() << "Deselected";
            } else if (std::abs(row - selectedRow_) + std::abs(col - selectedCol_) == 1) {
                // 相邻元素触发交换
                qDebug() << "Trying to swap (" << selectedRow_ << "," << selectedCol_ 
                         << ") with (" << row << "," << col << ")";
                
                // 在交换前保存地图快照和两个格子的水果
                saveMapSnapshot();
                swapFruit1_ = mapSnapshot_[selectedRow_][selectedCol_];
                swapFruit2_ = mapSnapshot_[row][col];
                
                bool success = gameEngine_->swapFruits(selectedRow_, selectedCol_, row, col);
                
                // 开始交换动画
                beginSwapAnimation(success);
                hasSelection_ = false;
                
                if (success) {
                    qDebug() << "Swap successful! Score:" << gameEngine_->getCurrentScore();
                } else {
                    qDebug() << "Swap failed, deselect";
                }
            } else {
                // 不相邻：切换选中目标
                selectedRow_ = row;
                selectedCol_ = col;
                hasSelection_ = true;
                qDebug() << "Change selection to:" << row << "," << col;
            }
        }
    } else {
        // 点击网格外，取消选中
        hasSelection_ = false;
        qDebug() << "Clicked outside grid, deselected";
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
 * @brief 动画定时器更新（状态机核心）
 * 
 * 状态流转：
 *   SWAPPING → (成功) → ELIMINATING → FALLING → (下一轮消除) → ELIMINATING → ... → IDLE
 *   SWAPPING → (失败) → IDLE
 * 
 * 每个动画阶段完成后，更新快照以反映变化
 */
void GameView::onAnimationTimer()
{
    animationFrame_++;
    
    switch (animPhase_) {
    case AnimPhase::SWAPPING:
        if (updateSwapAnimation()) {
            // 交换动画完成，更新快照
            if (swapSuccess_) {
                // 应用交换到快照
                std::swap(mapSnapshot_[swapRow1_][swapCol1_], mapSnapshot_[swapRow2_][swapCol2_]);
            }
            
            if (swapSuccess_ && gameEngine_) {
                const auto& seq = gameEngine_->getLastAnimation();
                if (!seq.rounds.empty()) {
                    // 开始第 0 轮消除
                    beginEliminationStep(0);
                } else {
                    // 没有消除轮次，回到空闲
                    animPhase_ = AnimPhase::IDLE;
                    hiddenCells_.clear();
                    mapSnapshot_.clear();
                }
            } else {
                // 交换失败，回到空闲
                animPhase_ = AnimPhase::IDLE;
                hiddenCells_.clear();
                mapSnapshot_.clear();
            }
        }
        update();
        break;
        
    case AnimPhase::ELIMINATING:
        if (updateEliminationAnimation()) {
            // 本轮消除完成，应用消除到快照
            applyEliminationToSnapshot(currentRoundIndex_);
            // 进入本轮下落
            beginFallStep(currentRoundIndex_);
        }
        update();
        break;
        
    case AnimPhase::FALLING:
        if (updateFallAnimation()) {
            // 本轮下落完成，应用下落到快照
            applyFallToSnapshot(currentRoundIndex_);
            
            if (gameEngine_) {
                const auto& seq = gameEngine_->getLastAnimation();
                int nextRound = currentRoundIndex_ + 1;
                if (nextRound < static_cast<int>(seq.rounds.size())) {
                    // 进入下一轮消除
                    beginEliminationStep(nextRound);
                } else {
                    // 所有轮次完成，检查是否有重排
                    if (seq.shuffled) {
                        beginShuffleAnimation();
                    } else {
                        // 回到空闲
                        currentRoundIndex_ = -1;
                        animPhase_ = AnimPhase::IDLE;
                        hiddenCells_.clear();
                        mapSnapshot_.clear();
                    }
                }
            }
        }
        update();
        break;
        
    case AnimPhase::SHUFFLING:
        if (updateShuffleAnimation()) {
            // 重排动画完成，回到空闲
            currentRoundIndex_ = -1;
            animPhase_ = AnimPhase::IDLE;
            hiddenCells_.clear();
            mapSnapshot_.clear();
        }
        update();
        break;
        
    case AnimPhase::IDLE:
    default:
        // 空闲时仅为选中框做脉冲重绘
        if (hasSelection_) {
            update();
        }
        break;
    }
}

// ========== 动画阶段控制函数实现 ==========

void GameView::beginSwapAnimation(bool success)
{
    if (!gameEngine_) return;
    
    const auto& map = gameEngine_->getMap();
    const auto& animSeq = gameEngine_->getLastAnimation();
    swapRow1_ = animSeq.swap.row1;
    swapCol1_ = animSeq.swap.col1;
    swapRow2_ = animSeq.swap.row2;
    swapCol2_ = animSeq.swap.col2;
    swapSuccess_ = success;
    animProgress_ = 0.0f;
    animPhase_ = AnimPhase::SWAPPING;
    
    // 交换动画：隐藏这两个格子的静态层，由动画层绘制
    hiddenCells_.clear();
    hiddenCells_.insert({swapRow1_, swapCol1_});
    hiddenCells_.insert({swapRow2_, swapCol2_});
}

bool GameView::updateSwapAnimation()
{
    const float duration = 200.0f; // 200ms
    const float delta = 16.0f / duration;
    animProgress_ += delta;
    
    if (animProgress_ >= 1.0f) {
        animProgress_ = 1.0f;
        return true; // 完成
    }
    return false;
}

void GameView::beginEliminationStep(int roundIndex)
{
    currentRoundIndex_ = roundIndex;
    animProgress_ = 0.0f;
    animPhase_ = AnimPhase::ELIMINATING;
    
    // 计算隐藏格子
    updateHiddenCells();
}

bool GameView::updateEliminationAnimation()
{
    const float duration = 220.0f; // 220ms
    const float delta = 16.0f / duration;
    animProgress_ += delta;
    
    if (animProgress_ >= 1.0f) {
        animProgress_ = 1.0f;
        return true;
    }
    return false;
}

void GameView::beginFallStep(int roundIndex)
{
    currentRoundIndex_ = roundIndex;
    animProgress_ = 0.0f;
    animPhase_ = AnimPhase::FALLING;
    
    // 计算隐藏格子
    updateHiddenCells();
}

bool GameView::updateFallAnimation()
{
    const float duration = 180.0f; // 180ms
    const float delta = 16.0f / duration;
    animProgress_ += delta;
    
    if (animProgress_ >= 1.0f) {
        animProgress_ = 1.0f;
        return true;
    }
    return false;
}

void GameView::computeColumnHiddenRanges()
{
    updateHiddenCells();
}

void GameView::updateHiddenCells()
{
    hiddenCells_.clear();
    
    if (!gameEngine_) return;
    
    const auto& seq = gameEngine_->getLastAnimation();
    
    if (currentRoundIndex_ < 0 || 
        currentRoundIndex_ >= static_cast<int>(seq.rounds.size())) {
        return;
    }
    
    const auto& round = seq.rounds[currentRoundIndex_];
    
    // 消除阶段：隐藏被消除的格子
    if (animPhase_ == AnimPhase::ELIMINATING) {
        for (const auto& pos : round.elimination.positions) {
            hiddenCells_.insert(pos);
        }
    }
    
    // 下落阶段：隐藏参与下落的目标位置和新生成位置
    if (animPhase_ == AnimPhase::FALLING) {
        // 下落的元素：隐藏目标位置
        for (const auto& move : round.fall.moves) {
            hiddenCells_.insert({move.toRow, move.toCol});
        }
        
        // 新生成的元素
        for (const auto& nf : round.fall.newFruits) {
            hiddenCells_.insert({nf.row, nf.col});
        }
    }
}

bool GameView::isCellHidden(int row, int col) const
{
    return hiddenCells_.count({row, col}) > 0;
}

// ========== 快照管理函数 ==========

void GameView::saveMapSnapshot()
{
    if (!gameEngine_) return;
    mapSnapshot_ = gameEngine_->getMap();
}

void GameView::applyEliminationToSnapshot(int roundIndex)
{
    if (!gameEngine_ || mapSnapshot_.empty()) return;
    
    const auto& seq = gameEngine_->getLastAnimation();
    if (roundIndex < 0 || roundIndex >= static_cast<int>(seq.rounds.size())) {
        return;
    }
    
    const auto& round = seq.rounds[roundIndex];
    
    // 清空被消除的格子
    for (const auto& pos : round.elimination.positions) {
        int r = pos.first;
        int c = pos.second;
        if (r >= 0 && r < MAP_SIZE && c >= 0 && c < MAP_SIZE) {
            mapSnapshot_[r][c].type = FruitType::EMPTY;
            mapSnapshot_[r][c].special = SpecialType::NONE;
        }
    }
}

void GameView::applyFallToSnapshot(int roundIndex)
{
    if (!gameEngine_ || mapSnapshot_.empty()) return;
    
    const auto& seq = gameEngine_->getLastAnimation();
    if (roundIndex < 0 || roundIndex >= static_cast<int>(seq.rounds.size())) {
        return;
    }
    
    const auto& round = seq.rounds[roundIndex];
    
    // 处理下落移动：按列收集移动，然后执行
    // 这里简化处理：直接同步到引擎的最终状态
    // 因为下落动画完成后，快照应该与引擎地图一致
    
    // 应用下落移动
    for (const auto& move : round.fall.moves) {
        int fromR = move.fromRow;
        int fromC = move.fromCol;
        int toR = move.toRow;
        int toC = move.toCol;
        
        if (toR >= 0 && toR < MAP_SIZE && toC >= 0 && toC < MAP_SIZE) {
            // 从引擎获取最终状态
            mapSnapshot_[toR][toC] = gameEngine_->getMap()[toR][toC];
        }
    }
    
    // 应用新生成的水果
    for (const auto& nf : round.fall.newFruits) {
        int r = nf.row;
        int c = nf.col;
        if (r >= 0 && r < MAP_SIZE && c >= 0 && c < MAP_SIZE) {
            mapSnapshot_[r][c] = gameEngine_->getMap()[r][c];
        }
    }
}

// ========== 重排动画函数 ==========

void GameView::beginShuffleAnimation()
{
    animProgress_ = 0.0f;
    animPhase_ = AnimPhase::SHUFFLING;
    
    // 重排动画期间隐藏所有格子
    hiddenCells_.clear();
    for (int row = 0; row < MAP_SIZE; ++row) {
        for (int col = 0; col < MAP_SIZE; ++col) {
            hiddenCells_.insert({row, col});
        }
    }
}

bool GameView::updateShuffleAnimation()
{
    const float duration = 600.0f; // 600ms 重排动画
    const float delta = 16.0f / duration;
    animProgress_ += delta;
    
    if (animProgress_ >= 1.0f) {
        animProgress_ = 1.0f;
        return true;
    }
    return false;
}

void GameView::drawShuffleAnimation()
{
    if (!gameEngine_) return;
    
    const auto& seq = gameEngine_->getLastAnimation();
    if (!seq.shuffled || seq.newMapAfterShuffle.empty()) {
        return;
    }
    
    float t = animProgress_;
    
    // 第一阶段(0-0.5): 旧元素淡出并缩小
    // 第二阶段(0.5-1.0): 新元素淡入并放大
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (t < 0.5f) {
        // 第一阶段：绘制快照中的旧元素（淡出）
        float phase = t / 0.5f;  // 0 → 1
        float alpha = 1.0f - phase;
        float scale = 1.0f - phase * 0.3f;
        
        const auto& map = mapSnapshot_.empty() ? gameEngine_->getMap() : mapSnapshot_;
        
        for (int row = 0; row < MAP_SIZE; ++row) {
            for (int col = 0; col < MAP_SIZE; ++col) {
                const Fruit& fruit = map[row][col];
                if (fruit.type == FruitType::EMPTY) continue;
                
                float cellX = gridStartX_ + col * cellSize_;
                float cellY = gridStartY_ + row * cellSize_;
                float centerX = cellX + cellSize_ * 0.5f;
                float centerY = cellY + cellSize_ * 0.5f;
                
                float size = cellSize_ * scale;
                float x = centerX - size * 0.5f;
                float y = centerY - size * 0.5f;
                
                int textureIndex = static_cast<int>(fruit.type);
                if (textureIndex >= 0 && textureIndex < static_cast<int>(fruitTextures_.size()) && fruitTextures_[textureIndex]) {
                    glEnable(GL_TEXTURE_2D);
                    fruitTextures_[textureIndex]->bind();
                    glColor4f(1.0f, 1.0f, 1.0f, alpha);
                    
                    float padding = size * 0.1f;
                    float fruitSize = size - padding * 2;
                    float fruitX = x + padding;
                    float fruitY = y + padding;
                    
                    glBegin(GL_QUADS);
                        glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
                        glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
                        glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
                        glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
                    glEnd();
                    
                    fruitTextures_[textureIndex]->release();
                }
            }
        }
    } else {
        // 第二阶段：绘制新地图中的元素（淡入）
        float phase = (t - 0.5f) / 0.5f;  // 0 → 1
        float alpha = phase;
        float scale = 0.7f + phase * 0.3f;
        
        const auto& newMap = seq.newMapAfterShuffle;
        
        for (int row = 0; row < MAP_SIZE; ++row) {
            for (int col = 0; col < MAP_SIZE; ++col) {
                const Fruit& fruit = newMap[row][col];
                if (fruit.type == FruitType::EMPTY) continue;
                
                float cellX = gridStartX_ + col * cellSize_;
                float cellY = gridStartY_ + row * cellSize_;
                float centerX = cellX + cellSize_ * 0.5f;
                float centerY = cellY + cellSize_ * 0.5f;
                
                float size = cellSize_ * scale;
                float x = centerX - size * 0.5f;
                float y = centerY - size * 0.5f;
                
                int textureIndex = static_cast<int>(fruit.type);
                if (textureIndex >= 0 && textureIndex < static_cast<int>(fruitTextures_.size()) && fruitTextures_[textureIndex]) {
                    glEnable(GL_TEXTURE_2D);
                    fruitTextures_[textureIndex]->bind();
                    glColor4f(1.0f, 1.0f, 1.0f, alpha);
                    
                    float padding = size * 0.1f;
                    float fruitSize = size - padding * 2;
                    float fruitX = x + padding;
                    float fruitY = y + padding;
                    
                    glBegin(GL_QUADS);
                        glTexCoord2f(0.0f, 0.0f); glVertex2f(fruitX, fruitY);
                        glTexCoord2f(1.0f, 0.0f); glVertex2f(fruitX + fruitSize, fruitY);
                        glTexCoord2f(1.0f, 1.0f); glVertex2f(fruitX + fruitSize, fruitY + fruitSize);
                        glTexCoord2f(0.0f, 1.0f); glVertex2f(fruitX, fruitY + fruitSize);
                    glEnd();
                    
                    fruitTextures_[textureIndex]->release();
                }
            }
        }
    }
}
