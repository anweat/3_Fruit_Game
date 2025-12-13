#ifndef PROPCLICKSTRATEGY_H
#define PROPCLICKSTRATEGY_H

#include "IClickStrategy.h"
#include "InputHandler.h"
#include <functional>

/**
 * @brief 道具模式点击策略
 * 
 * 处理逻辑：
 * - 锤子/魔法棒：单次点击即释放
 * - 夹子：需要点击两次（选择起点和终点）
 */
class PropClickStrategy : public IClickStrategy
{
public:
    bool handleClick(int row, int col, SelectionState& selection, PropInteractionData& propData) override;
    
    // 回调函数：请求释放道具
    std::function<void(ClickMode, int, int, int, int)> onPropReleaseRequested;
};

#endif // PROPCLICKSTRATEGY_H
