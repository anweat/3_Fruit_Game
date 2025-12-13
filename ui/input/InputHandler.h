#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <memory>
#include <functional>
#include "GameEngine.h"

/**
 * @brief 选中状态数据结构
 */
struct SelectionState
{
    bool hasSelection = false;
    int selectedRow = -1;
    int selectedCol = -1;
    
    void clear() {
        hasSelection = false;
        selectedRow = -1;
        selectedCol = -1;
    }
    
    void select(int row, int col) {
        hasSelection = true;
        selectedRow = row;
        selectedCol = col;
    }
};

/**
 * @brief 道具交互状态枚举
 */
enum class PropInteractionState {
    NONE,           // 无道具
    HOLDING,        // 持有道具，跟随鼠标
    FIRST_SELECTED, // 已选中第一个目标（仅夹子）
    READY           // 准备释放
};

/**
 * @brief 道具交互状态数据结构
 */
struct PropInteractionData
{
    PropInteractionState state = PropInteractionState::NONE;
    ClickMode propType = ClickMode::NORMAL;
    int targetRow1 = -1;
    int targetCol1 = -1;
    int targetRow2 = -1;  // 仅夹子使用
    int targetCol2 = -1;
    
    void clear() {
        state = PropInteractionState::NONE;
        propType = ClickMode::NORMAL;
        targetRow1 = targetCol1 = -1;
        targetRow2 = targetCol2 = -1;
    }
    
    bool isActive() const {
        return state != PropInteractionState::NONE;
    }
};

// 前向声明
class IClickStrategy;

/**
 * @brief 输入处理器 - 管理所有鼠标交互逻辑
 * 
 * 职责：
 * - 接收并处理鼠标事件
 * - 维护选中状态和道具状态
 * - 使用策略模式处理不同的点击模式
 * - 通过信号通知GameView执行操作
 */
class InputHandler : public QObject
{
    Q_OBJECT
    
public:
    explicit InputHandler(QObject* parent = nullptr);
    ~InputHandler() override;
    
    // 设置游戏引擎
    void setGameEngine(GameEngine* engine);
    
    // 处理鼠标事件（由GameView调用）
    void handleMousePress(const QPoint& screenPos, const QRect& gridRect, float cellSize);
    void handleMouseRelease(const QPoint& screenPos);
    void handleMouseMove(const QPoint& screenPos, const QRect& gridRect, float cellSize);
    
    // 模式切换
    void setClickMode(ClickMode mode);
    ClickMode getClickMode() const { return currentMode_; }
    
    // 获取当前状态（用于渲染）
    const SelectionState& getSelectionState() const { return selectionState_; }
    const PropInteractionData& getPropInteractionData() const { return propInteractionData_; }
    
    // 检查是否可以接受输入（非动画状态）
    void setCanAcceptInput(bool canAccept) { canAcceptInput_ = canAccept; }
    
signals:
    // 信号：通知GameView执行操作
    void swapRequested(int row1, int col1, int row2, int col2);
    void propReleaseRequested(ClickMode propType, int row1, int col1, int row2, int col2);
    void selectionChanged();
    void propStateChanged();
    void requestMouseTracking(bool enable);  // 请求启用/禁用鼠标跟踪
    
private:
    // 坐标转换
    bool screenToGrid(const QPoint& screenPos, const QRect& gridRect, float cellSize, int& row, int& col);
    
    // 切换点击策略
    void updateClickStrategy();
    
    GameEngine* gameEngine_;
    ClickMode currentMode_;
    bool canAcceptInput_;
    
    // 状态数据
    SelectionState selectionState_;
    PropInteractionData propInteractionData_;
    
    // 策略对象
    std::unique_ptr<IClickStrategy> clickStrategy_;
};

#endif // INPUTHANDLER_H
