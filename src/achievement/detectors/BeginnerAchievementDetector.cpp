#include "BeginnerAchievementDetector.h"

void BeginnerAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    // 🌟 初来乍到 - 完成首次三消
    if (snapshot.lastMatchSize >= 3) {
        callback("ach_first_match", 1, 1);
    }
    
    // 🌟 百分新手 - 首次得分超过100
    if (snapshot.currentScore >= 100) {
        callback("ach_score_100", snapshot.currentScore, 100);
    }
    
    // 🌟 特殊发现 - 首次生成特殊元素
    if (!snapshot.specialGenerated.isEmpty()) {
        callback("ach_first_special", 1, 1);
    }
}
