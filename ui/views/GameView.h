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

/**
 * @brief 视图动画阶段（严格按时序推进）
 */
enum class AnimPhase {
    IDLE,           ///< 空闲，等待玩家操作
    SWAPPING,       ///< 交换动画中（成功或失败）
    ELIMINATING,    ///< 消除动画中
    FALLING,        ///< 下落+新生成入场动画中
    SHUFFLING       ///< 死局重排动画中
};

/**
 * @brief 道具交互状态
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
    
    // ========== 渲染各层 ==========
    void drawFruitGrid();           ///< 静态水果层（隐藏 hiddenCells_）
    void drawSwapAnimation();       ///< 交换动画覆盖层
    void drawEliminationAnimation();///< 消除动画覆盖层
    void drawFallAnimation();       ///< 下落动画覆盖层
    void drawBombEffects();         ///< 炸弹特效动画层
    
    // ========== 动画阶段控制函数 ==========
    /// 开始交换动画（从 lastAnimation_.swap 初始化）
    void beginSwapAnimation(bool success);
    /// 更新交换动画进度，返回是否完成
    bool updateSwapAnimation();
    
    /// 开始第 stepIndex 轮消除动画（使用 rounds[stepIndex].elimination）
    void beginEliminationStep(int roundIndex);
    /// 更新消除动画进度，返回是否完成
    bool updateEliminationAnimation();
    
    /// 开始第 stepIndex 轮下落动画（使用 rounds[stepIndex].fall）
    void beginFallStep(int roundIndex);
    /// 更新下落动画进度，返回是否完成
    bool updateFallAnimation();
    
    /// 开始重排动画
    void beginShuffleAnimation();
    /// 更新重排动画进度，返回是否完成
    bool updateShuffleAnimation();
    /// 绘制重排动画
    void drawShuffleAnimation();
    
    /// 计算当前轮消除+下落的列级隐藏范围
    void computeColumnHiddenRanges();
    
    /// 更新 hiddenCells_ 集合
    void updateHiddenCells();
    
    /// 保存当前地图快照
    void saveMapSnapshot();
    
    /// 根据消除消息更新快照（清空被消除的格子）
    void applyEliminationToSnapshot(int roundIndex);
    
    /// 根据下落消息更新快照（移动元素+生成新元素）
    void applyFallToSnapshot(int roundIndex);
    
    /// 检查 (row, col) 是否在当前隐藏范围内
    bool isCellHidden(int row, int col) const;
    
    GameEngine* gameEngine_;                              ///< 游戏引擎
    std::vector<std::vector<Fruit>> mapSnapshot_;          ///< 地图快照（动画期间使用）
    std::vector<QOpenGLTexture*> fruitTextures_;          ///< 水果纹理数组
    
    // 网格布局参数
    float gridStartX_;
    float gridStartY_;
    float cellSize_;
    
    // 选中状态
    int selectedRow_;
    int selectedCol_;
    bool hasSelection_;
    
    // 点击模式
    ClickMode clickMode_ = ClickMode::NORMAL;
    
    // 道具交互状态
    PropState propState_ = PropState::NONE;         ///< 当前道具状态
    ClickMode heldPropType_ = ClickMode::NORMAL;    ///< 持有的道具类型
    int propTargetRow1_ = -1;                        ///< 道具目标位置1(行)
    int propTargetCol1_ = -1;                        ///< 道具目标位置1(列)
    int propTargetRow2_ = -1;                        ///< 道具目标位置2(行)，仅夹子使用
    int propTargetCol2_ = -1;                        ///< 道具目标位置2(列)，仅夹子使用
    
    // 动画定时器 & 帧计数
    QTimer* animationTimer_;
    int animationFrame_;
    
    // ========== 动画状态机 ==========
    AnimPhase animPhase_ = AnimPhase::IDLE;              ///< 当前动画阶段
    float animProgress_ = 0.0f;                          ///< 当前阶段进度 0~1
    bool swapSuccess_ = false;                           ///< 当前交换是否成功
    
    // 交换动画参数
    int swapRow1_ = -1;
    int swapCol1_ = -1;
    int swapRow2_ = -1;
    int swapCol2_ = -1;
    Fruit swapFruit1_;                                   ///< 交换前第一个格子的水果
    Fruit swapFruit2_;                                   ///< 交换前第二个格子的水果
    
    // 轮次动画索引（rounds[currentRoundIndex_] 包含消除+下落）
    int currentRoundIndex_ = -1;
    
    // ========== 隐藏格子集合 ==========
    // 在动画期间，这些格子的静态层不绘制，由动画层负责
    std::set<std::pair<int, int>> hiddenCells_;
};

#endif // GAMEVIEW_H

