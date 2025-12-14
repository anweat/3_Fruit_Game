#ifndef COMBOACCHIEVEMENTDETECTOR_H
#define COMBOACCHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 连击成就检测器
 * 
 * 负责检测以下成就：
 * - ach_combo_3/5/8/12/15: 单局连击成就
 * - ach_combo_100/500: 累计连击次数成就
 */
class ComboAchievementDetector : public IAchievementDetector
{
public:
    ComboAchievementDetector() : totalCombo3Plus_(0) {}

    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "ComboAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_combo_3", "ach_combo_5", "ach_combo_8", 
            "ach_combo_12", "ach_combo_15",
            "ach_combo_100", "ach_combo_500"
        };
    }

    // 累计统计数据
    void setTotalCombo3Plus(int total) { totalCombo3Plus_ = total; }
    int getTotalCombo3Plus() const { return totalCombo3Plus_; }

private:
    int totalCombo3Plus_;  // 累计3+连击次数
};

#endif // COMBOACCHIEVEMENTDETECTOR_H
