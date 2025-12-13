#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QTimer>
#include <QMouseEvent>
#include <vector>
#include <array>
#include <set>
#include <memory>
#include <functional>
#include "GameEngine.h"
#include "AnimationController.h"
#include "SnapshotManager.h"
#include "IAnimationRenderer.h"

// 前向声明渲染器
class SwapAnimationRenderer;
class EliminationAnimationRenderer;
class FallAnimationRenderer;
class ShuffleAnimationRenderer;

/**
 * @brief 道具交互状态（注意：这是GameView内部使用的枚举，与InputHandler中的PropInteractionState不同）
 */
enum class PropState {
    NONE,           ///< 无道具状态
    HOLDING,        ///< 持有道具，跟随鼠标
    FIRST_SELECTED, ///< 已选中第一个目标（仅夹子使用）
    READY           ///< 准备释放（已选定所有目标）
};

/**
 * @brief 游戏OpenGL渲染视图
 * 
 * 使用OpenGL渲染8×8水果地图，支持纹理显示和动画效果
 */
class GameView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GameView(QWidget *parent = nullptr);
    ~GameView() override;
    
    void setGameEngine(GameEngine* engine);
    void updateDisplay();
    
    /**
     * @brief 设置点击模式
     * @param mode 点击模式
     */
    void setClickMode(ClickMode mode);
    
    /**
     * @brief 获取当前点击模式
     * @return 当前模式
     */
    ClickMode getClickMode() const { return clickMode_; }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onAnimationTimer();

private:
    // ========== 纹理 & 绘制基础 ==========
    void loadTextures();
    void drawQuad(float x, float y, float size);
    void drawFruit(int row, int col, const Fruit& fruit, float offsetX = 0.0f, float offsetY = 0.0f);
    void drawSelection();
    bool screenToGrid(int x, int y, int& row, int& col);
    
    // ========== 点击事件处理策略 ==========
    /// 处理普通交换模式点击
    void handleNormalClick(int row, int col);
    /// 处理道具模式点击
    void handlePropClick(int row, int col);
    
    /// 释放道具效果
    void releaseProp();
    /// 取消道具使用
    void cancelProp();
    /// 绘制道具选中框
    void drawPropSelection();
    
    // ========== 渲染 ==========
    void drawFruitGrid();           ///< 静态水果层
    void renderCurrentAnimation();  ///< 当前动画阶段渲染分发器
    
    // ========== 动画阶段控制 ==========
    /// 开始交换动画
    void beginSwapAnimation(bool success);
    /// 开始消除动画
    void beginEliminationStep(int roundIndex);
    /// 开始下落动画
    void beginFallStep(int roundIndex);
    /// 开始重排动画
    void beginShuffleAnimation();
    
    /// 阶段完成回调
    void handlePhaseComplete(AnimPhase phase);
    
    // ========== 引擎和基础 ==========
    GameEngine* gameEngine_;
    std::vector<QOpenGLTexture*> fruitTextures_;
    
    // 网格布局参数
    float gridStartX_;
    float gridStartY_;
    float cellSize_;
    
    // 动画定时器 & 帧计数
    QTimer* animationTimer_;
    int animationFrame_;
    
    // ========== 动画系统（解耦组件）==========
    AnimationController* animController_;       ///< 动画状态机控制器
    SnapshotManager* snapshotManager_;          ///< 快照管理器
    SwapAnimationRenderer* swapRenderer_;       ///< 交换动画渲染器
    EliminationAnimationRenderer* eliminationRenderer_; ///< 消除动画渲染器
    FallAnimationRenderer* fallRenderer_;       ///< 下落动画渲染器
    ShuffleAnimationRenderer* shuffleRenderer_; ///< 重排动画渲染器
    
    // ========== 选中和道具状态 ==========
    int selectedRow_;
    int selectedCol_;
    bool hasSelection_;
    
    ClickMode clickMode_ = ClickMode::NORMAL;
    
    PropState propState_ = PropState::NONE;
    ClickMode heldPropType_ = ClickMode::NORMAL;
    int propTargetRow1_ = -1;
    int propTargetCol1_ = -1;
    int propTargetRow2_ = -1;
    int propTargetCol2_ = -1;
};

#endif // GAMEVIEW_H

