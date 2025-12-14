#ifndef BEGINNERACHIEVEMENTDETECTOR_H
#define BEGINNERACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 初学者成就检测器
 * 
 * 负责检测以下成就：
 * - ach_first_match: 初来乍到 - 完成首次三消
 * - ach_score_100: 百分新手 - 首次得分超过100
 * - ach_first_special: 特殊发现 - 首次生成特殊元素
 */
class BeginnerAchievementDetector : public IAchievementDetector
{
public:
    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "BeginnerAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_first_match",
            "ach_score_100",
            "ach_first_special"
        };
    }
};

#endif // BEGINNERACHIEVEMENTDETECTOR_H
