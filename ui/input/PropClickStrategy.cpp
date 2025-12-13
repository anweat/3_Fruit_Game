#include "PropClickStrategy.h"

bool PropClickStrategy::handleClick(int row, int col, SelectionState& selection, PropInteractionData& propData)
{
    Q_UNUSED(selection);
    
    switch (propData.state) {
        case PropInteractionState::HOLDING:
            // 持有道具状态，第一次点击：选中目标
            if (propData.propType == ClickMode::PROP_CLAMP) {
                // 夹子需要选中第一个目标
                propData.targetRow1 = row;
                propData.targetCol1 = col;
                propData.state = PropInteractionState::FIRST_SELECTED;
            } else {
                // 锤子和魔法棒只需要一个目标
                propData.targetRow1 = row;
                propData.targetCol1 = col;
                propData.state = PropInteractionState::READY;
                // 立即释放
                if (onPropReleaseRequested) {
                    onPropReleaseRequested(propData.propType, row, col, -1, -1);
                }
            }
            return true;
            
        case PropInteractionState::FIRST_SELECTED:
            // 已选中第一个目标（仅夹子），第二次点击：选中第二个目标
            if (row == propData.targetRow1 && col == propData.targetCol1) {
                // 点击同一个位置，取消选中
                propData.state = PropInteractionState::HOLDING;
                propData.targetRow1 = propData.targetCol1 = -1;
            } else {
                // 选中第二个目标
                propData.targetRow2 = row;
                propData.targetCol2 = col;
                propData.state = PropInteractionState::READY;
                // 立即释放
                if (onPropReleaseRequested) {
                    onPropReleaseRequested(
                        propData.propType,
                        propData.targetRow1, propData.targetCol1,
                        row, col
                    );
                }
            }
            return true;
            
        default:
            break;
    }
    
    return false;
}
