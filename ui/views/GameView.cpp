#include "GameView.h"
#include <QDebug>
#include <QOpenGLFunctions>
#include <QPainter>
#include <cmath>

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
{
    // 设置最小尺寸
    setMinimumSize(600, 600);
    
    // 创建动画定时器
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
        drawFruitGrid();
    }
    
    // 绘制选中框
    if (hasSelection_) {
        drawSelection();
    }
}

/**
 * @brief 加载所有水果纹理
 */
void GameView::loadTextures()
{
    // 水果类型和对应的文件名
    const char* fruitFiles[] = {
        "resources/textures/apple.png",      // APPLE
        "resources/textures/orange.png",     // ORANGE
        "resources/textures/grape.png",      // GRAPE
        "resources/textures/banana.png",     // BANANA
        "resources/textures/watermelon.png", // WATERMELON
        "resources/textures/strawberry.png"  // STRAWBERRY
    };
    
    fruitTextures_.resize(6);
    
    for (int i = 0; i < 6; i++) {
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
 * @brief 绘制水果网格
 */
void GameView::drawFruitGrid()
{
    const auto& map = gameEngine_->getMap();
    
    // 先绘制所有单元格背景（确保没有空白）
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            float x = gridStartX_ + col * cellSize_;
            float y = gridStartY_ + row * cellSize_;
            
            // 绘制单元格背景
            glDisable(GL_TEXTURE_2D);
            glColor4f(0.3f, 0.35f, 0.45f, 1.0f);
            drawQuad(x, y, cellSize_);
        }
    }
    
    // 再绘制水果纹理
    for (int row = 0; row < MAP_SIZE; row++) {
        for (int col = 0; col < MAP_SIZE; col++) {
            const Fruit& fruit = map[row][col];
            if (fruit.type != FruitType::EMPTY) {
                drawFruit(row, col, fruit);
            }
        }
    }
}

/**
 * @brief 绘制单个水果
 */
void GameView::drawFruit(int row, int col, const Fruit& fruit)
{
    // 计算位置
    float x = gridStartX_ + col * cellSize_;
    float y = gridStartY_ + row * cellSize_;
    
    // 获取纹理索引
    int textureIndex = static_cast<int>(fruit.type);
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
            } else {
                // 点击不同的水果，尝试交换
                qDebug() << "Trying to swap (" << selectedRow_ << "," << selectedCol_ 
                         << ") with (" << row << "," << col << ")";
                
                bool success = gameEngine_->swapFruits(selectedRow_, selectedCol_, row, col);
                
                if (success) {
                    qDebug() << "Swap successful! Score:" << gameEngine_->getCurrentScore();
                    hasSelection_ = false;  // 成功后清除选中
                } else {
                    qDebug() << "Swap failed, selecting new fruit";
                    // 失败后直接选中新点击的水果
                    selectedRow_ = row;
                    selectedCol_ = col;
                    hasSelection_ = true;
                }
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
 * @brief 动画定时器更新
 */
void GameView::onAnimationTimer()
{
    animationFrame_++;
    
    // 如果有选中，需要重绘以显示脉冲效果
    if (hasSelection_) {
        update();
    }
}
