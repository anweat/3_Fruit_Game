#ifndef SCOREACHIEVEMENTDETECTOR_H
#define SCOREACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 得分成就检测器
 * 
 * 负责检测以下成就：
 * - ach_score_1k/5k/10k/20k/30k/50k/100k: 单局得分阶梯
 * - ach_score_burst: 单次消除爆发（500分以上）
 */
class ScoreAchievementDetector : public IAchievementDetector
{
public:
    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "ScoreAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_score_1k", "ach_score_5k", "ach_score_10k",
            "ach_score_20k", "ach_score_30k", "ach_score_50k",
            "ach_score_100k", "ach_score_burst"
        };
    }
};

#endif // SCOREACHIEVEMENTDETECTOR_H
