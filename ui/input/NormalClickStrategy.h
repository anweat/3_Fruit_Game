#ifndef NORMALCLICKSTRATEGY_H
#define NORMALCLICKSTRATEGY_H

#include "IClickStrategy.h"
#include "InputHandler.h"
#include <functional>

/**
 * @brief 普通交换模式点击策略
 * 
 * 处理逻辑：
 * 1. 第一次点击：选中水果
 * 2. 点击同一位置：取消选中
 * 3. 点击相邻位置：触发交换
 * 4. 点击其他位置：切换选中
 */
class NormalClickStrategy : public IClickStrategy
{
public:
    bool handleClick(int row, int col, SelectionState& selection, PropInteractionData& propData) override;
    
    // 回调函数：请求执行交换
    std::function<void(int, int, int, int)> onSwapRequested;
};

#endif // NORMALCLICKSTRATEGY_H
