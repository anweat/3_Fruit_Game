#include "ComboAchievementDetector.h"

void ComboAchievementDetector::detect(
    const GameDataSnapshot& snapshot,
    std::function<void(const QString&, int, int)> callback
)
{
    int combo = snapshot.maxCombo;
    
    // 单局连击成就
    if (combo >= 3) callback("ach_combo_3", combo, 3);
    if (combo >= 5) callback("ach_combo_5", combo, 5);
    if (combo >= 8) callback("ach_combo_8", combo, 8);
    if (combo >= 12) callback("ach_combo_12", combo, 12);
    if (combo >= 15) callback("ach_combo_15", combo, 15);
    
    // 累计连击次数
    if (snapshot.isComboTrigger && snapshot.currentCombo >= 3) {
        totalCombo3Plus_++;
        callback("ach_combo_100", totalCombo3Plus_, 100);
        callback("ach_combo_500", totalCombo3Plus_, 500);
    }
}
