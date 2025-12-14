#include "SpecialAchievementDetector.h"

void SpecialAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    // 首次生成特殊元素
    if (!snapshot.specialGenerated.isEmpty()) {
        callback("ach_special_first", 1, 1);
        totalSpecialGenerated_++;
        
        // 分类统计
        if (snapshot.specialGenerated.contains("LINE")) {
            totalLineGenerated_++;
            callback("ach_line_50", totalLineGenerated_, 50);
        }
        if (snapshot.specialGenerated == "DIAMOND") {
            totalDiamondGenerated_++;
            callback("ach_diamond_30", totalDiamondGenerated_, 30);
        }
        if (snapshot.specialGenerated == "RAINBOW") {
            totalRainbowGenerated_++;
            callback("ach_rainbow_20", totalRainbowGenerated_, 20);
        }
    }
    
    // 使用特殊元素
    if (!snapshot.specialUsed.isEmpty()) {
        totalSpecialUsed_++;
        callback("ach_special_200", totalSpecialUsed_, 200);
    }
    
    // 组合检测
    if (snapshot.comboPairType == "LINE_LINE") {
        callback("ach_combo_line_line", 1, 1);
    }
    if (snapshot.comboPairType == "LINE_DIAMOND") {
        callback("ach_combo_line_diamond", 1, 1);
    }
    if (snapshot.comboPairType == "DIAMOND_DIAMOND") {
        callback("ach_combo_diamond_diamond", 1, 1);
    }
    if (snapshot.comboPairType.contains("RAINBOW") && snapshot.comboPairType != "RAINBOW_RAINBOW") {
        callback("ach_combo_any_rainbow", 1, 1);
    }
    if (snapshot.comboPairType == "RAINBOW_RAINBOW") {
        callback("ach_combo_rainbow_rainbow", 1, 1);
    }
}
