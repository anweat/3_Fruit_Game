#include "PropAchievementDetector.h"

void PropAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    if (snapshot.propUsedType == "Hammer") {
        callback("ach_prop_hammer_first", 1, 1);
        totalPropUsed_++;
        
        // 精准打击
        if (snapshot.propChainEliminate >= 10) {
            callback("ach_prop_chain", snapshot.propChainEliminate, 10);
        }
    }
    if (snapshot.propUsedType == "Clamp") {
        callback("ach_prop_clamp_first", 1, 1);
        totalPropUsed_++;
    }
    if (snapshot.propUsedType == "MagicWand") {
        callback("ach_prop_wand_first", 1, 1);
        totalPropUsed_++;
    }
    
    // 累计道具使用
    callback("ach_prop_50", totalPropUsed_, 50);
    callback("ach_prop_200", totalPropUsed_, 200);
}
