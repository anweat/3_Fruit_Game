#include "InputHandler.h"
#include "NormalClickStrategy.h"
#include "PropClickStrategy.h"

InputHandler::InputHandler(QObject* parent)
    : QObject(parent)
    , gameEngine_(nullptr)
    , currentMode_(ClickMode::NORMAL)
    , canAcceptInput_(true)
{
    updateClickStrategy();
}

InputHandler::~InputHandler() = default;

void InputHandler::setGameEngine(GameEngine* engine)
{
    gameEngine_ = engine;
}

void InputHandler::setClickMode(ClickMode mode)
{
    // 如果有活跃的道具状态，先清除
    if (propInteractionData_.isActive()) {
        propInteractionData_.clear();
        emit propStateChanged();
    }
    
    currentMode_ = mode;
    
    // 更新策略
    updateClickStrategy();
    
    // 如果是道具模式，设置道具状态
    if (mode == ClickMode::PROP_HAMMER || 
        mode == ClickMode::PROP_CLAMP || 
        mode == ClickMode::PROP_MAGIC_WAND) {
        propInteractionData_.state = PropInteractionState::HOLDING;
        propInteractionData_.propType = mode;
        emit requestMouseTracking(true);  // 开启鼠标跟踪
        emit propStateChanged();
    } else {
        emit requestMouseTracking(false);
    }
    
    // 清除选中状态
    selectionState_.clear();
    emit selectionChanged();
}

void InputHandler::handleMousePress(const QPoint& screenPos, const QRect& gridRect, float cellSize)
{
    if (!canAcceptInput_) {
        return;  // 动画期间不接受输入
    }
    
    int mapSize = gameEngine_ ? gameEngine_->getCurrentMapSize() : 8;
    int row, col;
    if (screenToGrid(screenPos, gridRect, cellSize, mapSize, row, col)) {
        // 点击在网格内，使用策略处理
        if (clickStrategy_) {
            bool changed = clickStrategy_->handleClick(row, col, selectionState_, propInteractionData_);
            if (changed) {
                emit selectionChanged();
                emit propStateChanged();
            }
        }
    } else {
        // 点击网格外，取消选中或道具
        if (propInteractionData_.isActive()) {
            propInteractionData_.clear();
            emit propStateChanged();
        } else if (selectionState_.hasSelection) {
            selectionState_.clear();
            emit selectionChanged();
        }
    }
}

void InputHandler::handleMouseRelease(const QPoint& screenPos)
{
    Q_UNUSED(screenPos);
    // 可以在这里处理拖拽结束等逻辑
}

void InputHandler::handleMouseMove(const QPoint& screenPos, const QRect& gridRect, float cellSize)
{
    Q_UNUSED(screenPos);
    Q_UNUSED(gridRect);
    Q_UNUSED(cellSize);
    
    // 道具持有状态时，触发重绘以显示跟随效果
    if (propInteractionData_.state == PropInteractionState::HOLDING) {
        emit propStateChanged();
    }
}

bool InputHandler::screenToGrid(const QPoint& screenPos, const QRect& gridRect, float cellSize, int mapSize, int& row, int& col)
{
    float gridX = screenPos.x() - gridRect.x();
    float gridY = screenPos.y() - gridRect.y();
    
    col = static_cast<int>(gridX / cellSize);
    row = static_cast<int>(gridY / cellSize);
    
    return (row >= 0 && row < mapSize && col >= 0 && col < mapSize);
}

void InputHandler::updateClickStrategy()
{
    if (currentMode_ == ClickMode::NORMAL) {
        // 普通交换模式
        auto strategy = std::make_unique<NormalClickStrategy>();
        
        // 设置回调
        strategy->onSwapRequested = [this](int r1, int c1, int r2, int c2) {
            emit swapRequested(r1, c1, r2, c2);
        };
        
        clickStrategy_ = std::move(strategy);
    } else {
        // 道具模式
        auto strategy = std::make_unique<PropClickStrategy>();
        
        // 设置回调
        strategy->onPropReleaseRequested = [this](ClickMode type, int r1, int c1, int r2, int c2) {
            emit propReleaseRequested(type, r1, c1, r2, c2);
        };
        
        clickStrategy_ = std::move(strategy);
    }
}
