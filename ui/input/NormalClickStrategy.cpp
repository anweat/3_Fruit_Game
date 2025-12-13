#include "NormalClickStrategy.h"
#include <cmath>

bool NormalClickStrategy::handleClick(int row, int col, SelectionState& selection, PropInteractionData& propData)
{
    Q_UNUSED(propData);
    
    if (!selection.hasSelection) {
        // 第一次点击，选中水果
        selection.select(row, col);
        return true;
    } else {
        // 第二次点击
        if (row == selection.selectedRow && col == selection.selectedCol) {
            // 点击同一个，取消选中
            selection.clear();
            return true;
        } else if (std::abs(row - selection.selectedRow) + std::abs(col - selection.selectedCol) == 1) {
            // 相邻元素，触发交换
            if (onSwapRequested) {
                onSwapRequested(selection.selectedRow, selection.selectedCol, row, col);
            }
            selection.clear();
            return true;
        } else {
            // 不相邻，切换选中目标
            selection.select(row, col);
            return true;
        }
    }
}
