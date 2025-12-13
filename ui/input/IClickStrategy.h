#ifndef ICLICKSTRATEGY_H
#define ICLICKSTRATEGY_H

// 前向声明
struct SelectionState;
struct PropInteractionData;

/**
 * @brief 点击策略接口
 * 
 * 使用策略模式处理不同的点击模式：
 * - 普通交换模式
 * - 道具模式
 * - 提示模式（未来扩展）
 */
class IClickStrategy
{
public:
    virtual ~IClickStrategy() = default;
    
    /**
     * @brief 处理点击事件
     * @param row 点击的行
     * @param col 点击的列
     * @param selection 选中状态（引用，可修改）
     * @param propData 道具状态（引用，可修改）
     * @return 是否产生了状态变化（需要通知视图更新）
     */
    virtual bool handleClick(int row, int col, SelectionState& selection, PropInteractionData& propData) = 0;
};

#endif // ICLICKSTRATEGY_H
