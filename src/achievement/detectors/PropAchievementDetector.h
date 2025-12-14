#ifndef PROPACHIEVEMENTDETECTOR_H
#define PROPACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 道具成就检测器
 * 
 * 负责检测以下成就：
 * - ach_prop_hammer_first/clamp_first/wand_first: 首次使用道具
 * - ach_prop_chain: 锤子精准打击
 * - ach_prop_50/200: 累计道具使用次数
 */
class PropAchievementDetector : public IAchievementDetector
{
public:
    PropAchievementDetector() : totalPropUsed_(0) {}

    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "PropAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_prop_hammer_first", "ach_prop_clamp_first",
            "ach_prop_wand_first", "ach_prop_chain",
            "ach_prop_50", "ach_prop_200"
        };
    }

    void setTotalPropUsed(int total) { totalPropUsed_ = total; }

private:
    int totalPropUsed_;  // 累计道具使用次数
};

#endif // PROPACHIEVEMENTDETECTOR_H
