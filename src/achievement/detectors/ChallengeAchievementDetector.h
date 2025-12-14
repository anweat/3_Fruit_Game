#ifndef CHALLENGEACHIEVEMENTDETECTOR_H
#define CHALLENGEACHIEVEMENTDETECTOR_H

#include "IAchievementDetector.h"

/**
 * @brief 挑战成就检测器
 * 
 * 负责检测以下成就：
 * - ach_challenge_60s: 速战速决 - 60秒内得分超过10000
 * - ach_challenge_flash: 闪电手 - 30秒内完成20次消除
 * - ach_challenge_noprop: 无道具挑战 - 不用道具得分20000
 * - ach_challenge_chain5: 连锁反应 - 连锁反应5次
 * - ach_challenge_30fruits: 地图清理 - 消除30个特殊元素
 * - ach_challenge_marathon: 持久战 - 游戏持续10分钟
 */
class ChallengeAchievementDetector : public IAchievementDetector
{
public:
    void detect(
        const GameDataSnapshot& snapshot,
        std::function<void(const QString&, int, int)> callback
    ) override;

    QString getName() const override { return "ChallengeAchievementDetector"; }

    QSet<QString> getResponsibleAchievements() const override {
        return {
            "ach_challenge_60s", "ach_challenge_flash",
            "ach_challenge_noprop", "ach_challenge_chain5",
            "ach_challenge_30fruits", "ach_challenge_marathon"
        };
    }
};

#endif // CHALLENGEACHIEVEMENTDETECTOR_H
